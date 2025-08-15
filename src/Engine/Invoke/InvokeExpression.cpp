#include "InvokeExpression.h"

//------------------------------
// Private:

void Nebulite::InvokeExpression::compileIfExpression(Entry& entry) {
    if (entry.type == Entry::Type::eval) {
        // Compile the expression using TinyExpr
        int error;
        entry.expression = te_compile(entry.str.c_str(), variables.data(), variables.size(), &error);
        if (error) {
            printCompileError(entry, error);

            // Resetting expression to nan, as explained in error print:
            // using nan directly is not supported.
            // 0/0 directly yields -nan, so we use abs(0/0)
            entry.expression = te_compile("abs(0/0)", variables.data(), variables.size(), &error);
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

    // First, we must split the expression into tokens
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
            // This part represents the '$' + formatter
            // Cannot be used, as splitOnSameDepth expects the first character to be the opening parenthesis
            std::string start = token.substr(0, token.find('('));
            std::string tokenWithoutstart = token.substr(start.length()); // Remove the leading '$'

            // Split on same depth
            std::vector<std::string> subTokens = StringHandler::splitOnSameDepth(tokenWithoutstart, '(');

            // Add back the '$' + formatter to first subToken
            if(subTokens.size() > 0) {
                subTokens[0] = start + subTokens[0];
            }

            // Add all subtokens to the actual list of tokens
            for (const auto& subToken : subTokens) {
                tokens.push_back(subToken);
            }
        } else {
            // If it doesnt start with a '$', it's a text token / potentially with variables inside
            // Just add the text token
            tokens.push_back(token);
        }
    }

    // Now we have a correct list of tokens. Either:
    // - evaluation
    // - text
    // Now we parse all tokens into a proper entry, which further splits and categorizes them:
    // - general type: {variable, eval, text}
    // - cast type
    // - formatting
    // - splitting all text-variable mixes
    // - variable info (from what document, what the key is)
    for (auto& token : tokens) {
        if (!token.empty()) {
            Entry currentEntry;
            if(token.starts_with('$')){
                parseTokenTypeEval(token, currentEntry, entries);
            }
            else{
                parseTokenTypeText(token, currentEntry, entries);
            }
        }
    }
}

void Nebulite::InvokeExpression::readFormatter(Entry* entry, const std::string& formatter) {
    // Check formatter. Integer cast should not include precision. Is ignored later on in casting but acceptable as input
    // Examples:
    // $i     : leadingZero = false , alignment = -1 , precision = -1
    // $04i   : leadingZero = true  , alignment =  4 , precision = -1
    // $03.5i : leadingZero = true  , alignment =  3 , precision =  5

    if(!formatter.size()){
        return;
    }

    // Read leading zero
    if(formatter.starts_with("0")) {
        entry->leadingZero = true;
    }
    if(formatter.size() > 1){
        int16_t dotpos = formatter.find('.');
    
        entry->alignment = std::stoi(formatter.substr(0, dotpos));
        if(dotpos != std::string::npos){
            entry->precision = std::stoi(formatter.substr(dotpos + 1));
        }
    }
}

void Nebulite::InvokeExpression::parseTokenTypeEval(std::string& token, Entry& currentEntry, std::vector<Entry>& entries) {
    // $[leading zero][alignment][.][precision]<type:f,i>
    // - bool leading zero   : on/off
    // - int alignment       : <0 means no formatting
    // - int precision       : <0 means no formatting
    // - CastType::none is then used to determine if we can simple use the double return from tinyexpr

    // 1.) find next '(' and split into formatter and token
    // Examples:
    // input        formatter       expression
    // $(1+1)       ""              "(1+1)"
    // $f(1.23)     "f"             "(1.23)"
    // $i(42)       "i"             "(42)"
    // $4.2f(2/3)   "4.2f"          "(2/3)"

    int16_t pos = token.find('(');
    std::string formatter  = token.substr(1, pos - 1); // Remove leading $
    std::string expression = token.substr(pos);

    // Check cast type in formatter:
    if(formatter.ends_with("i")){
        readFormatter(&currentEntry, formatter);
        currentEntry.cast = Entry::CastType::to_int;
    }
    else if(formatter.ends_with("f")){
        readFormatter(&currentEntry, formatter);
        currentEntry.cast = Entry::CastType::to_double;
    }
    else{
        currentEntry.cast = Entry::CastType::none;
    }

    // Current token is evaluation
    currentEntry.type = Entry::Type::eval;

    // Register internal variables
    // And build equivalent expression
    std::string newString = "";
    std::vector<std::string> subTokens = StringHandler::splitOnSameDepth(expression, '{');

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

void Nebulite::InvokeExpression::parseTokenTypeText(std::string& token, Entry& currentEntry, std::vector<Entry>& entries) {
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

void Nebulite::InvokeExpression::printCompileError(const Entry& entry, int& error) {
    std::cerr << "-----------------------------------------------------------------" << std::endl;
    std::cerr << "Error compiling expression: '" << entry.str << "' Error code: " << error << std::endl;
    std::cerr << "You might see this message multiple times due to expression parallelization." << std::endl;
    std::cerr << std::endl;
    std::cerr << "If you only see the start of your expression, make sure to encompass your expression in quotes" << std::endl;
    std::cerr << "Some functions assume that the expression is inside, e.g. argv[1]." << std::endl;
    std::cerr << "Example: " << std::endl;
    std::cerr << "if $(1+1)     echo here! # works" << std::endl;
    std::cerr << "if $(1 + 1)   echo here! # doesnt work!" << std::endl;
    std::cerr << "if '$(1 + 1)' echo here! # works" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Registered functions and variables:\n";
    for (const auto& var : variables) {
        std::cerr << "\t'" << var.name << "'\n";
    }
    std::cerr << std::endl;
    std::cerr << "Resetting expression to always yield 'nan'" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::endl;
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

    // Check if parsed expression is returnable as double
    _isReturnableAsDouble = entries.size() == 1 && entries[0].type == Entry::eval && entries[0].cast == Entry::CastType::none;
}

std::string Nebulite::InvokeExpression::eval(Nebulite::JSON* current_other) {
    // Update references to 'other'
    update_vds(&virtualDoubles_other, current_other);

    // Concatenate results of each entry
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

                //-------------------------
                // Handle casting
                if(entry.cast == Entry::CastType::to_int){
                    token = std::to_string(static_cast<int>(te_eval(entry.expression)));
                } else {
                    token = std::to_string(te_eval(entry.expression));
                }

                // Precision handling
                if (entry.precision != -1) {
                    size_t dotPos = token.find('.');
                    if (dotPos != std::string::npos) {
                        size_t currentPrecision = token.size() - dotPos - 1;
                        if (currentPrecision < static_cast<size_t>(entry.precision)) {
                            // Add zeros to match the required precision
                            token.append(entry.precision - currentPrecision, '0');
                        } else {
                            // Truncate to the required precision
                            token = token.substr(0, dotPos + entry.precision + 1);
                        }
                    } else {
                        // No decimal point, add one and pad with zeros
                        token += '.';
                        token.append(entry.precision, '0');
                    }
                }

                // Adding padding
                if(entry.alignment > 0 && token.size() < entry.alignment) {
                    for(int i = 0; i < entry.alignment - token.size(); i++){
                        token = (entry.leadingZero ? '0' : ' ') + token;
                    }
                }

                break;

            case Entry::text:
                token = entry.str;
                break;

            default:
                break;
        }
        result += token;
    }
    
    return result;
}

bool Nebulite::InvokeExpression::isReturnableAsDouble() {
    return _isReturnableAsDouble;
}

double Nebulite::InvokeExpression::evalAsDouble(Nebulite::JSON* current_other) {
    update_vds(&virtualDoubles_other, current_other);
    return te_eval(entries[0].expression);
}
