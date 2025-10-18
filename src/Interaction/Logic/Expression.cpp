#include "Interaction/Logic/Expression.hpp"

#include "Core/GlobalSpace.hpp"

//------------------------------------------
// Private:

Nebulite::Interaction::Logic::Expression::~Expression() {
    // reset all data
    reset();

    // Remove variables
    te_variables.clear();

    // Clear all expressions
    for(auto& entry : entries) {
        if(entry.expression != nullptr) {
            te_free(entry.expression);
            entry.expression = nullptr;
        }
    }
}

void Nebulite::Interaction::Logic::Expression::reset() {
    documentCache = nullptr;
    self = nullptr;
    global = nullptr;

    // Clear existing data
    entries.clear();
    te_variables.clear();
    fullExpression.clear();
    entries.clear();

    // Clear all variable names
    te_names.clear();

    // Clear vds
    virtualDoubles_self.clear();
    virtualDoubles_other.clear();
    virtualDoubles_global.clear();
    virtualDoubles_resource.clear();

    //------------------------------------------
    // Register built-in functions

    //===================================================================================================
    // Category             Name           Pointer                             Type           Context
    //===================================================================================================

    // Logical comparison functions
    te_variables.push_back({"gt",          (void*)expr_custom::gt,             TE_FUNCTION2,  nullptr});
    te_variables.push_back({"lt",          (void*)expr_custom::lt,             TE_FUNCTION2,  nullptr});
    te_variables.push_back({"geq",         (void*)expr_custom::geq,            TE_FUNCTION2,  nullptr});
    te_variables.push_back({"leq",         (void*)expr_custom::leq,            TE_FUNCTION2,  nullptr});
    te_variables.push_back({"eq",          (void*)expr_custom::eq,             TE_FUNCTION2,  nullptr});
    te_variables.push_back({"neq",         (void*)expr_custom::neq,            TE_FUNCTION2,  nullptr});

    // Logical gate functions
    te_variables.push_back({"not",         (void*)expr_custom::logical_not,    TE_FUNCTION1,  nullptr});
    te_variables.push_back({"and",         (void*)expr_custom::logical_and,    TE_FUNCTION2,  nullptr});
    te_variables.push_back({"or",          (void*)expr_custom::logical_or,     TE_FUNCTION2,  nullptr});
    te_variables.push_back({"xor",         (void*)expr_custom::logical_xor,    TE_FUNCTION2,  nullptr});
    te_variables.push_back({"nand",        (void*)expr_custom::logical_nand,   TE_FUNCTION2,  nullptr});
    te_variables.push_back({"nor",         (void*)expr_custom::logical_nor,    TE_FUNCTION2,  nullptr});
    te_variables.push_back({"xnor",        (void*)expr_custom::logical_xnor,   TE_FUNCTION2,  nullptr});

    // Other logical functions
    te_variables.push_back({"to_bipolar",  (void*)expr_custom::to_bipolar,     TE_FUNCTION1,  nullptr});

    // Mapping functions
    te_variables.push_back({"map",         (void*)expr_custom::map,            TE_FUNCTION5,  nullptr});
    te_variables.push_back({"constrain",   (void*)expr_custom::constrain,      TE_FUNCTION3,  nullptr});

    // More mathematical functions
    te_variables.push_back({"sgn",         (void*)expr_custom::sgn,            TE_FUNCTION1,  nullptr});
}

std::string Nebulite::Interaction::Logic::Expression::stripContext(const std::string& key) {
    if (key.starts_with("self.")) {
        return key.substr(5);
    } else if (key.starts_with("other.")) {
        return key.substr(6);
    } else if (key.starts_with("global.")) {
        return key.substr(7);
    } else {
        return key;
    }
}

Nebulite::Interaction::Logic::Expression::Entry::From Nebulite::Interaction::Logic::Expression::getContext(const std::string& key) {
    if (key.starts_with("self.")) {
        return Entry::From::self;
    } else if (key.starts_with("other.")) {
        return Entry::From::other;
    } else if (key.starts_with("global.")) {
        return Entry::From::global;
    } else {
        return Entry::From::resource;
    }
}

void Nebulite::Interaction::Logic::Expression::compileIfExpression(Entry& entry) {
    if (entry.type == Entry::Type::eval) {
        // Compile the expression using TinyExpr
        int error;
        entry.expression = te_compile(entry.str.c_str(), te_variables.data(), te_variables.size(), &error);
        if (error) {
            printCompileError(entry, error);

            // Resetting expression to nan, as explained in error print:
            // using nan directly is not supported.
            // 0/0 directly yields -nan, so we use abs(0/0)
            entry.expression = te_compile("abs(0/0)", te_variables.data(), te_variables.size(), &error);
        }
    }
}

void Nebulite::Interaction::Logic::Expression::registerVariable(std::string te_name, std::string key, Entry::From context){
    // Check if variable exists in variables vector:
    bool found = false;
    for(auto te_var : te_variables) {
        if(te_var.name == te_name) {
            found = true;
            break;
        }
    }
    if(!found) {
        // Initialize with reference to document and cache register
        std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble> vd = std::make_shared<Nebulite::Interaction::Logic::VirtualDouble>(key, documentCache);

        // Register cache directly to json file, if possible
        switch(context) {
            case Entry::From::self:
                // Type self is remanent, we can register the double directly from document
                vd->setUpExternalCache(self);
                virtualDoubles_self.push_back(vd);
                break;
            case Entry::From::other:
                // Type other is non-remanent, we need to use the cache that is updated on eval
                virtualDoubles_other.push_back(vd);
                break;
            case Entry::From::global:
                // Type global is remanent, we can register the double directly from document
                vd->setUpExternalCache(global);
                virtualDoubles_global.push_back(vd);
                break;
            case Entry::From::resource:
                // Type resource is non-remanent, we need to use the cache that is updated on eval
                // The reason is that resource-documents may get deloaded,
                // making the direct double reference invalid.
                virtualDoubles_resource.push_back(vd);
                break;
            case Entry::From::None:
            default:
                // Should not happen
                std::cerr << __FUNCTION__ << ": Tried to register variable with no known context!" << std::endl;
                break;
        }

        // Store variable name for tinyexpr
        auto te_name_ptr = std::make_shared<std::string>(te_name);
        te_names.push_back(te_name_ptr);

        // Push back into variable entries
        te_variables.push_back({
            te_names.back()->c_str(),
            vd->ptr(),
            TE_VARIABLE,
            nullptr
        });
    }
}

void Nebulite::Interaction::Logic::Expression::parseIntoEntries(const std::string& expr, std::vector<Entry>& entries){
    // First, we must split the expression into tokens
    std::vector<std::string> tokensPhase1, tokens;

    // Split, keep delimiter(at start)
    // "abc$def$ghi" -> ["abc", "$def", "$ghi"]
    tokensPhase1 = Nebulite::Utility::StringHandler::split(expr, '$', true);

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
            std::vector<std::string> subTokens = Nebulite::Utility::StringHandler::splitOnSameDepth(tokenWithoutstart, '(');

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

void Nebulite::Interaction::Logic::Expression::readFormatter(Entry* entry, const std::string& formatter) {
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
        size_t dotpos = formatter.find('.');
        // Read alignment
        if(dotpos == 0){
            entry->alignment = 0;
        }
        else{
            entry->alignment = std::stoi(formatter.substr(0, dotpos));
        }
        // Read precision
        if(dotpos != std::string::npos){
            entry->precision = std::stoi(formatter.substr(dotpos + 1));
        }
    }
}

void Nebulite::Interaction::Logic::Expression::parseTokenTypeEval(std::string& token, Entry& currentEntry, std::vector<Entry>& entries) {
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
    std::vector<std::string> subTokens = Nebulite::Utility::StringHandler::splitOnSameDepth(expression, '{');

    for (const auto& subToken : subTokens) {
        if(subToken.starts_with('{')){
            // 1.) remove {}
            std::string key, te_name;
            key = Nebulite::Utility::StringHandler::replaceAll(subToken, "{", "");
            key = Nebulite::Utility::StringHandler::replaceAll(key     , "}", "");
            te_name = Nebulite::Utility::StringHandler::replaceAll(key, ".", "_");
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

void Nebulite::Interaction::Logic::Expression::parseTokenTypeText(std::string& token, Entry& currentEntry, std::vector<Entry>& entries) {
    // Current token is Text
    // Perhaps mixed with variables...
    std::vector<std::string> subTokens = Nebulite::Utility::StringHandler::splitOnSameDepth(token, '{');
    for (const auto& subToken : subTokens) {
        // Variable outside of eval, no need to register
        if(subToken.starts_with('{')){
            // 1.) remove {}
            std::string inner;
            // Replace first and last character of subToken
            // We keep all other potential {} inside the variable name
            // For later multiresolve
            inner = subToken.substr(1, subToken.length() - 2);
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

void Nebulite::Interaction::Logic::Expression::printCompileError(const Entry& entry, int& error) {
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
    for (const auto& var : te_variables) {
        std::cerr << "\t'" << var.name << "'\n";
    }
    std::cerr << std::endl;
    std::cerr << "Resetting expression to always yield 'nan'" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::endl;
}

//------------------------------------------
// Public:

Nebulite::Interaction::Logic::Expression::Expression() {
    reset();
}

void Nebulite::Interaction::Logic::Expression::parse(const std::string& expr, Nebulite::Utility::DocumentCache* documentCache, Nebulite::Utility::JSON* self, Nebulite::Utility::JSON* global){
    reset();

    // Set references
    this->self = self;
    this->global = global;
    this->documentCache = documentCache;
    fullExpression = expr;

    // Parse
    parseIntoEntries(expr, entries);

    // Now compile all entries:
    for (auto& entry : entries) {
        compileIfExpression(entry);
    }

    // Check if parsed expression is returnable as double
    _isReturnableAsDouble = (entries.size() == 1) &&
                            (entries[0].type == Entry::eval) &&
                            (entries[0].cast == Entry::CastType::none);

    // Get the unique ID for this expression
    auto globalspace = self->getGlobalSpace();
    uniqueId = globalspace->getUniqueId(fullExpression, Nebulite::Core::GlobalSpace::UniqueIdType::EXPRESSION);
}

std::string Nebulite::Interaction::Logic::Expression::eval(Nebulite::Utility::JSON* current_other, uint16_t max_recursion_depth) {
    //------------------------------------------
    // Update caches so that tinyexpr has the correct references
    updateCaches(current_other);

    //------------------------------------------
    // Evaluate expression

    // Concatenate results of each entry
    std::string result = "";
    std::string token;
    std::string key;
    Entry::From context;
    bool failed = false;
    for (auto& entry : entries) {
        token = "";
        key = entry.key;
        context = entry.from;
        switch (entry.type){
            //------------------------------------------
            case Entry::variable:
                // See if the variable contains an inner expression
                if(entry.str.find('$') != std::string::npos || entry.str.find('{') != std::string::npos) {
                    if(max_recursion_depth == 0) {
                        std::cerr << "Error: Maximum recursion depth reached when evaluating variable: " << entry.key << std::endl;
                        return "0";
                    }
                    // Create a temporary expression to evaluate the inner expression
                    Expression tempExpr;
                    tempExpr.parse(entry.str, documentCache, self, global);
                    key = tempExpr.eval(current_other, max_recursion_depth - 1);

                    // Redetermine context and strip it from key
                    context = getContext(key);
                    key = stripContext(key);
                }

                // Now, use the key to get the value from the correct document
                switch(context) {
                    case Entry::From::self:
                        if(self == nullptr){
                            std::cerr << "Error: Null self reference in expression: " << key << std::endl;
                            failed = true;
                            break;
                        }
                        token = self->get<std::string>(key.c_str(), "0");
                        break;
                    case Entry::From::other:
                        if(current_other == nullptr) {
                            std::cerr << "Error: Null other reference in expression: " << key << std::endl;
                            failed = true;
                            break;
                        }
                        token = current_other->get<std::string>(key.c_str(), "0");
                        break;
                    case Entry::From::global:
                        if (global == nullptr) {
                            std::cerr << "Error: Null global reference in expression: " << key << std::endl;
                            failed = true;
                            break;
                        }
                        token = global->get<std::string>(key.c_str(), "0");
                        break;
                    case Entry::From::resource:
                    default:
                        if (globalCache == nullptr) {
                            std::cerr << "Error: Null globalCache reference in expression: " << key  << ". If this shouldn't be a Resource reference, did you forget the prefix self/other/global?" << std::endl;
                            failed = true;
                            break;
                        }
                        token = globalCache->get<std::string>(key.c_str(), "0");
                        break;
                }
                if(failed){
                    return "0";
                }
                break;
            //------------------------------------------
            case Entry::eval:
                //------------------------------------------
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
                            token.append(entry.precision - currentPrecision + 1, '0');
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
                if(entry.alignment > 0 && token.size() < static_cast<size_t>(entry.alignment)) {
                    int32_t size = token.size();
                    for(int i = 0; i < entry.alignment - size; i++){
                        token = (entry.leadingZero ? '0' : ' ') + token;
                    }
                }
                break;
            //------------------------------------------
            case Entry::text:
                token = entry.str;
                break;
            //------------------------------------------
            default:
                break;
        }
        result += token;
    }
    
    return result;
}

bool Nebulite::Interaction::Logic::Expression::isReturnableAsDouble() {
    return _isReturnableAsDouble;
}

double Nebulite::Interaction::Logic::Expression::evalAsDouble(Nebulite::Utility::JSON* current_other) {
    // Update caches so that tinyexpr has the correct references
    updateCaches(current_other);

    // Evaluate expression
    return te_eval(entries[0].expression);
}

odpvec* Nebulite::Interaction::Logic::Expression::ensure_other_cache_entry(Nebulite::Utility::JSON* current_other) {
    auto cache = current_other->getExpressionRefsAsOther();
    std::lock_guard<std::mutex> cache_lock(cache->mtx);
    
    // Check if we can use quickcache, that does not rely on a hashmap lookup
    if(uniqueId < ORDERED_DOUBLE_POINTERS_QUICKCACHE_SIZE){
        if(cache->quickCache[uniqueId].orderedValues.empty()){
            // Not initialized yet, create one with exact size
            Nebulite::Utility::OrderedDoublePointers newCacheList(virtualDoubles_other.size());

            // Populate list with all virtual doubles from type other
            for(auto& vde : virtualDoubles_other) {
                double* reference = current_other->get_stable_double_ptr(vde->getKey());
                newCacheList.orderedValues.push_back(reference);
            }
            cache->quickCache[uniqueId] = std::move(newCacheList);
        }
        return &cache->quickCache[uniqueId].orderedValues;
    }

    // If id is too large for quickcache, use hashmap
    auto it = cache->map.find(uniqueId);
    
    // If not, create one
    if(it == cache->map.end()) {
        Nebulite::Utility::OrderedDoublePointers newCacheList(virtualDoubles_other.size());

        // Populate list with all virtual doubles from type other
        for(auto& vde : virtualDoubles_other) {
            double* reference = current_other->get_stable_double_ptr(vde->getKey());
            newCacheList.orderedValues.push_back(reference);
        }

        cache->map.emplace(uniqueId, std::move(newCacheList));
        it = cache->map.find(uniqueId);
    }
    return &it->second.orderedValues;
}

void Nebulite::Interaction::Logic::Expression::updateCaches(Nebulite::Utility::JSON* current_other) {
    //------------------------------------------
    // 1.) Update other

    // Get a list of all references, insert into virtual doubles
    if(!virtualDoubles_other.empty()){
        auto list = ensure_other_cache_entry(current_other);
        auto* list_data = list->data();
        const size_t count = virtualDoubles_other.size();
        for(size_t i = 0; i < count; ++i) {
            virtualDoubles_other[i]->setDirect(*list_data[i]);
        }
    }

    //------------------------------------------
    // 2.) Update resource

    // Update resource references
    for(auto& vde : virtualDoubles_resource) {
        vde->setUpInternalCache(nullptr);
    }
}

//------------------------------------------
// DEBUGGING

void Nebulite::Interaction::Logic::Expression::printCache(std::vector<std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble>>& vec) {
    // Sort alphabetically
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
        return a->getKey() < b->getKey();
    });
    for (const auto& vd : vec) {
        std::cout << "\t" << vd->getKey() << " = " << *(vd->ptr()) << std::endl;
    }
}

void Nebulite::Interaction::Logic::Expression::debugOutput(Nebulite::Utility::JSON* current_other){
    // Debug only a certain expression
    if(entries[0].str.starts_with("(global_")){
        std::cout << "\n" << std::endl;
        std::string strSelf  = self->get<std::string>(Nebulite::Constants::keyName.renderObject.textStr.c_str(), "null");
        std::string strOther = current_other->get<std::string>(Nebulite::Constants::keyName.renderObject.textStr.c_str(), "null");
        std::cout << "Updated references. Self is: " << strSelf << " Other is: " << strOther << std::endl;

        std::cout << "After update:" << std::endl;
        if(virtualDoubles_self.size() > 0) {
            std::cout << "Values for self:" << std::endl;
            printCache(virtualDoubles_self);
        }
        if(virtualDoubles_other.size() > 0) {
            std::cout << "Values for other:" << std::endl;
            printCache(virtualDoubles_other);
        }
        if(virtualDoubles_global.size() > 0) {
            std::cout << "Values for global:" << std::endl;
            printCache(virtualDoubles_global);
        }
        if(virtualDoubles_resource.size() > 0) {
            std::cout << "Values for resource:" << std::endl;
            printCache(virtualDoubles_resource);
        }
        std::cout << "Now evaluating expression: " << entries[0].str << std::endl;

        double result = te_eval(entries[0].expression);
        std::cout << "Result: " << result << std::endl;
        std::cout << "\n" << std::endl;
    }
}