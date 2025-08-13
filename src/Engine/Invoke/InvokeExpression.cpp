#include "InvokeExpression.h"



void Nebulite::InvokeExpression::compileIfExpression(Entry& entry) {
    if (entry.type == Entry::Type::eval) {
        // Compile the expression using TinyExpr
        int error;
        entry.expression = te_compile(entry.str.c_str(), variables.data(), variables.size(), &error);
        if (error) {
            std::cerr << "Error compiling expression: '" << entry.str << "' Error code: " << error << std::endl;
            std::cerr << "Registered functions and variables:\n";
            for (const auto& var : variables) {
                std::cerr << "\t'" << var.name << "'\n";
            }
        }
    }
}

void Nebulite::InvokeExpression::registerVariable(std::string te_name, std::string key, Entry::From context){
    // Check if variable exists in variables vector:
    bool found = false;
    for(auto var : variables) {
        if(var.name == te_name) {
            found = true;
            break;
        }
    }
    if(!found) {
        // Initialize with reference to document and cache register
        std::shared_ptr<Nebulite::VirtualDouble> vd = std::make_shared<Nebulite::VirtualDouble>(key, documentCache);
        std::shared_ptr<vd_entry> vde = std::make_shared<vd_entry>(vd, context, key, te_name);

        // Register cache directly to json file, if possible
        switch(vde->from) {
            case Entry::From::self:
                #if use_external_cache
                    vde->virtualDouble->register_external_double_cache(self);
                #endif
                virtualDoubles_self.push_back(vde);
                break;
            case Entry::From::other:
                virtualDoubles_other.push_back(vde);
                break;
            case Entry::From::global:
                #if use_external_cache
                    vde->virtualDouble->register_external_double_cache(global);
                #endif
                virtualDoubles_global.push_back(vde);
                break;
            case Entry::From::resource:
                #if use_external_cache
                    vde->virtualDouble->register_external_double_cache(nullptr);
                #endif
                virtualDoubles_resource.push_back(vde);
                break;
        }

        // Push back into variable entries
        variables.push_back({
            vde->te_name.c_str(),
            vde->virtualDouble->ptr(),
            TE_VARIABLE,
            nullptr
        });
    }
}

void Nebulite::InvokeExpression::parseIntoEntries(const std::string& expr, std::vector<Entry>& entries){

    std::vector<std::string> tokensPhase1, tokens;

    // Split, keep delimiter(at start)
    // "abc$def$ghi" -> ["abc", "$def", "$ghi"]
    tokensPhase1 = StringHandler::split(expr, '$', true);

    // Now we need to split on same depth
    for(auto token : tokensPhase1) {
        // If the first token starts with '$', it means the string started with '$'
        // If not, the first token is text before the first '$'
        if(token.starts_with('$')) {
            // Remove everything until a '('
            std::string start = token.substr(0, token.find('('));
            std::string tokenWithoutstart = token.substr(start.length()); // Remove the leading '$'

            // Split on same depth
            std::vector<std::string> subTokens = StringHandler::splitOnSameDepth(tokenWithoutstart, '(');

            // Add back to first
            if(subTokens.size() > 0) {
                subTokens[0] = start + subTokens[0];
            }

            for (const auto& subToken : subTokens) {
                tokens.push_back(subToken);
            }
        } else {
            // Just add the text token
            tokens.push_back(token);
        }
    }

    // Parse all tokens
    for (auto& token : tokens) {
        if (!token.empty()) {
            Entry currentEntry;
            if(token.starts_with('$')){
                // Check cast type:
                if(token.starts_with("$i(")){
                    currentEntry.cast = Entry::CastType::to_int;
                    token = token.substr(2);
                }
                else if(token.starts_with("$f(")){
                    currentEntry.cast = Entry::CastType::to_float;
                    token = token.substr(2);
                }
                else{
                    currentEntry.cast = Entry::CastType::none;
                    token = token.substr(1);
                }

                // Current token is evaluation
                currentEntry.type = Entry::Type::eval;

                // Register internal variables
                // And build equivalent expression
                std::string newString = "";
                std::vector<std::string> subTokens = StringHandler::splitOnSameDepth(token, '{');

                for (const auto& subToken : subTokens) {
                    if(subToken.starts_with('{')){
                        // 1.) remove {}
                        std::string key, te_name;
                        key = StringHandler::replaceAll(subToken, "{", "");
                        key = StringHandler::replaceAll(key     , "}", "");
                        te_name = StringHandler::replaceAll(key, ".", "_");
                        // 2.) determine context
                        Entry::From context = getContext(key);
                        key     = stripContext(key);
                        // 3.) register variable
                        registerVariable(te_name, key, context);
                        // Append
                        newString += te_name;
                    }
                    else{
                        newString += subToken;
                    }
                }
                // Determine context
                currentEntry.type = Entry::Type::eval;
                currentEntry.str = newString; // Shouldnt contain any $
                currentEntry.from = Entry::From::None;
                currentEntry.key = ""; // No key for eval expressions

                // Add to entries
                entries.push_back(currentEntry);
            }
            else{
                // Current token is Text
                // Perhaps mixed with variables...
                std::vector<std::string> subTokens = StringHandler::splitOnSameDepth(token, '{');
                for (const auto& subToken : subTokens) {
                    // Variable outside of eval, no need to register
                    if(subToken.starts_with('{')){
                        // 1.) remove {}
                        std::string inner, te_name;
                        inner = StringHandler::replaceAll(subToken, "{", "");
                        inner = StringHandler::replaceAll(inner     , "}", "");
                        // 2.) determine context
                        currentEntry.type = Entry::Type::variable;
                        currentEntry.str = inner;
                        currentEntry.from = getContext(inner);
                        currentEntry.key = stripContext(inner);
                    }
                    else{
                        // Determine context
                        currentEntry.type = Entry::Type::text;
                        currentEntry.str = subToken;
                        currentEntry.from = Entry::From::None;
                        currentEntry.key = ""; // No key for text expressions
                    }
                    // Add to entries
                    entries.push_back(currentEntry);
                }
            }
        }
    }
}

//------------------------------
// Public:

Nebulite::InvokeExpression::InvokeExpression() {
    clear();
}

void Nebulite::InvokeExpression::parse(const std::string& expr, Nebulite::DocumentCache& documentCache, Nebulite::JSON* self, Nebulite::JSON* global){
    clear();

    // Set references
    this->self = self;
    this->global = global;
    this->documentCache = &documentCache;
    fullExpression = expr;

    // Parse
    parseIntoEntries(expr, entries);

    // Now compile all entries:
    for (auto& entry : entries) {
        compileIfExpression(entry);
    }
}

std::string Nebulite::InvokeExpression::eval(Nebulite::JSON* current_other) {
    update_vds(&virtualDoubles_other, current_other);

    #if use_external_cache
    #else
        update_vds(&virtualDoubles_self, self);
        update_vds(&virtualDoubles_other, current_other);
        update_vds(&virtualDoubles_global, global);
        update_vds(&virtualDoubles_resource, nullptr);
    #endif

    

    std::string result = "";
    std::string token;
    for (auto& entry : entries) {
        token = "";
        switch (entry.type){
            case Entry::variable:
                // Variables default to 0

                if (entry.from == Entry::self) {
                    if(self == nullptr){
                        std::cerr << "Error: Null self reference in expression: " << entry.key << std::endl;
                        return "0";
                    }
                    token = self->get<std::string>(entry.key.c_str(), "0");
                } else if (entry.from == Entry::other) {
                    if(current_other == nullptr) {
                        std::cerr << "Error: Null other reference in expression: " << entry.key << std::endl;
                        return "0";
                    }
                    token = current_other->get<std::string>(entry.key.c_str(), "0");
                } else if (entry.from == Entry::global) {
                    if (global == nullptr) {
                        std::cerr << "Error: Null global reference in expression: " << entry.key << std::endl;
                        return "0";
                    }
                    token = global->get<std::string>(entry.key.c_str(), "0");
                } else if (entry.from == Entry::resource) {
                    if (globalCache == nullptr) {
                        std::cerr << "Error: Null globalCache reference in expression: " << entry.key << std::endl;
                        return "0";
                    }
                    token = globalCache->getData<std::string>(entry.key.c_str(), "0");
                }
                break;

            case Entry::eval:
                token = std::to_string(te_eval(entry.expression));
                break;

            case Entry::text:
                token = entry.str;
                break;

            default:
                break;
        }
        switch (entry.cast){
            case Entry::CastType::none:
                result += token;
                break;

            case Entry::CastType::to_int:
                result += std::to_string(static_cast<int>(std::stof(token)));
                break;

            case Entry::CastType::to_float:
                result += std::to_string(static_cast<float>(std::stof(token)));
                break;

            default:
                break;
        }
    }
    
    return result;
}