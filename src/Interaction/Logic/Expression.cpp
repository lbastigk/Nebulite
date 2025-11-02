#include "Interaction/Logic/Expression.hpp"

#include <cmath>

#include "Core/GlobalSpace.hpp"
#include "Utility/Capture.hpp"


//------------------------------------------
// Private:

Nebulite::Interaction::Logic::Expression::~Expression(){
    // reset all data
    reset();

    // Remove variables
    te_variables.clear();

    // Clear all expressions
    for(auto& component : components){
        if(component->expression != nullptr){
            te_free(component->expression);
            component->expression = nullptr;
        }
    }
}

void Nebulite::Interaction::Logic::Expression::reset(){
    references.documentCache = nullptr;
    references.self = nullptr;
    references.global = nullptr;

    // Clear existing data
    components.clear();
    te_variables.clear();
    fullExpression.clear();
    components.clear();

    // Clear all variable names
    te_names.clear();

    // Clear vds
    virtualDoubles.self.clear();
    virtualDoubles.other.clear();
    virtualDoubles.global.clear();
    virtualDoubles.resource.clear();

    //------------------------------------------
    // Register built-in functions

    //====================================================================================================================
    // Category             Name           Pointer                                               Type           Context
    //====================================================================================================================

    // Logical comparison functions
    te_variables.push_back({"gt",          reinterpret_cast<void*>(expr_custom::gt),             TE_FUNCTION2,  nullptr});
    te_variables.push_back({"lt",          reinterpret_cast<void*>(expr_custom::lt),             TE_FUNCTION2,  nullptr});
    te_variables.push_back({"geq",         reinterpret_cast<void*>(expr_custom::geq),            TE_FUNCTION2,  nullptr});
    te_variables.push_back({"leq",         reinterpret_cast<void*>(expr_custom::leq),            TE_FUNCTION2,  nullptr});
    te_variables.push_back({"eq",          reinterpret_cast<void*>(expr_custom::eq),             TE_FUNCTION2,  nullptr});
    te_variables.push_back({"neq",         reinterpret_cast<void*>(expr_custom::neq),            TE_FUNCTION2,  nullptr});

    // Logical gate functions
    te_variables.push_back({"not",         reinterpret_cast<void*>(expr_custom::logical_not),    TE_FUNCTION1,  nullptr});
    te_variables.push_back({"and",         reinterpret_cast<void*>(expr_custom::logical_and),    TE_FUNCTION2,  nullptr});
    te_variables.push_back({"or",          reinterpret_cast<void*>(expr_custom::logical_or),     TE_FUNCTION2,  nullptr});
    te_variables.push_back({"xor",         reinterpret_cast<void*>(expr_custom::logical_xor),    TE_FUNCTION2,  nullptr});
    te_variables.push_back({"nand",        reinterpret_cast<void*>(expr_custom::logical_nand),   TE_FUNCTION2,  nullptr});
    te_variables.push_back({"nor",         reinterpret_cast<void*>(expr_custom::logical_nor),    TE_FUNCTION2,  nullptr});
    te_variables.push_back({"xnor",        reinterpret_cast<void*>(expr_custom::logical_xnor),   TE_FUNCTION2,  nullptr});

    // Other logical functions
    te_variables.push_back({"to_bipolar",  reinterpret_cast<void*>(expr_custom::to_bipolar),     TE_FUNCTION1,  nullptr});

    // Mapping functions
    te_variables.push_back({"map",         reinterpret_cast<void*>(expr_custom::map),            TE_FUNCTION5,  nullptr});
    te_variables.push_back({"constrain",   reinterpret_cast<void*>(expr_custom::constrain),      TE_FUNCTION3,  nullptr});

    // More mathematical functions
    te_variables.push_back({"sgn",         reinterpret_cast<void*>(expr_custom::sgn),            TE_FUNCTION1,  nullptr});
}

std::string Nebulite::Interaction::Logic::Expression::stripContext(std::string const& key){
    if (key.starts_with("self.")){
        return key.substr(5);
    } else if (key.starts_with("other.")){
        return key.substr(6);
    } else if (key.starts_with("global.")){
        return key.substr(7);
    } else {
        return key;
    }
}

Nebulite::Interaction::Logic::Expression::Component::From Nebulite::Interaction::Logic::Expression::getContext(std::string const& key){
    if (key.starts_with("self.")){
        return Component::From::self;
    } else if (key.starts_with("other.")){
        return Component::From::other;
    } else if (key.starts_with("global.")){
        return Component::From::global;
    } else {
        return Component::From::resource;
    }
}

void Nebulite::Interaction::Logic::Expression::compileIfExpression(std::shared_ptr<Component>& component){
    if (component->type == Component::Type::eval){
        // Compile the expression using TinyExpr
        int error;
        component->expression = te_compile(component->str.c_str(), te_variables.data(), static_cast<int>(te_variables.size()), &error);
        if (error){
            printCompileError(component, error);

            // Resetting expression to nan, as explained in error print:
            // using nan directly is not supported.
            // 0/0 directly yields -nan, so we use abs(0/0)
            te_free(component->expression);
            component->expression = te_compile("abs(0/0)", te_variables.data(), static_cast<int>(te_variables.size()), &error);
        }
    }
}

void Nebulite::Interaction::Logic::Expression::registerVariable(std::string te_name, std::string key, Component::From context){
    // Check if variable exists in variables vector:
    bool found = std::any_of(te_variables.begin(), te_variables.end(), [&](auto const& te_var){ return te_var.name == te_name; });

    if(!found){
        // Initialize with reference to document and cache register
        std::shared_ptr<Nebulite::Interaction::Logic::VirtualDouble> vd = std::make_shared<Nebulite::Interaction::Logic::VirtualDouble>(key, references.documentCache);

        // Register cache directly to json file, if possible
        switch(context){
            case Component::From::self:
                // Type self is remanent, we can register the double directly from document
                vd->setUpExternalCache(references.self);
                virtualDoubles.self.push_back(vd);
                break;
            case Component::From::other:
                // Type other is non-remanent, we need to use the cache that is updated on eval
                virtualDoubles.other.push_back(vd);
                break;
            case Component::From::global:
                // Type global is remanent, we can register the double directly from document
                vd->setUpExternalCache(references.global);
                virtualDoubles.global.push_back(vd);
                break;
            case Component::From::resource:
                // Type resource is non-remanent, we need to use the cache that is updated on eval
                // The reason is that resource-documents may get deloaded,
                // making the direct double reference invalid.
                virtualDoubles.resource.push_back(vd);
                break;
            case Component::From::None:
            default:
                // Should not happen
                Nebulite::Utility::Capture::cerr() << __FUNCTION__ << ": Tried to register variable with no known context!" << Nebulite::Utility::Capture::endl;
                break;
        }

        // Store variable name for tinyexpr
        auto te_name_ptr = std::make_shared<std::string>(te_name);
        te_names.push_back(te_name_ptr);

        // Push back into variable components
        te_variables.push_back({
            te_names.back()->c_str(),
            vd->ptr(),
            TE_VARIABLE,
            nullptr
        });
    }
}

void Nebulite::Interaction::Logic::Expression::parseIntoComponents(std::string const& expr){
    // First, we must split the expression into tokens
    std::vector<std::string> tokensPhase1, tokens;

    // Split, keep delimiter(at start)
    // "abc$def$ghi" -> ["abc", "$def", "$ghi"]
    tokensPhase1 = Nebulite::Utility::StringHandler::split(expr, '$', true);

    // Now we need to split on same depth
    for(auto const& token : tokensPhase1){
        // If the first token starts with '$', it means the string started with '$'
        // If not, the first token is text before the first '$'
        if(token.starts_with('$')){
            // Remove everything until a '('
            // This part represents the '$' + formatter
            // Cannot be used, as splitOnSameDepth expects the first character to be the opening parenthesis
            std::string start = token.substr(0, token.find('('));
            std::string tokenWithoutstart = token.substr(start.length()); // Remove the leading '$'

            // Split on same depth
            std::vector<std::string> subTokens = Nebulite::Utility::StringHandler::splitOnSameDepth(tokenWithoutstart, '(');

            // Add back the '$' + formatter to first subToken
            if(subTokens.size() > 0){
                subTokens[0] = start + subTokens[0];
            }

            // Add all subtokens to the actual list of tokens
            std::copy(subTokens.begin(), subTokens.end(), std::back_inserter(tokens));
        } else {
            // If it doesnt start with a '$', it's a text token / potentially with variables inside
            // Just add the text token
            tokens.push_back(token);
        }
    }

    // Now we have a correct list of tokens. Either:
    // - evaluation
    // - text
    // Now we parse all tokens into a proper component, which further splits and categorizes them:
    // - general type: {variable, eval, text}
    // - cast type
    // - formatting
    // - splitting all text-variable mixes
    // - variable info (from what document, what the key is)
    for (auto const& token : tokens){
        if (!token.empty()){
            if(token.starts_with('$')){
                parseTokenTypeEval(token);
            }
            else{
                parseTokenTypeText(token);
            }
        }
    }
}

void Nebulite::Interaction::Logic::Expression::readFormatter(std::shared_ptr<Component> const& component, std::string const& formatter){
    // Check formatter. Integer cast should not include precision. Is ignored later on in casting but acceptable as input
    // Examples:
    // $i     : leadingZero = false , alignment = -1 , precision = -1
    // $04i   : leadingZero = true  , alignment =  4 , precision = -1
    // $03.5i : leadingZero = true  , alignment =  3 , precision =  5

    if(!formatter.size()){
        return;
    }

    // Read leading zero
    if(formatter.starts_with("0")){
        component->formatter.leadingZero = true;
    }
    if(formatter.size() > 1){
        size_t dotpos = formatter.find('.');
        // Read alignment
        if(dotpos == 0){
            component->formatter.alignment = 0;
        }
        else{
            component->formatter.alignment = std::stoi(formatter.substr(0, dotpos));
        }
        // Read precision
        if(dotpos != std::string::npos){
            component->formatter.precision = std::stoi(formatter.substr(dotpos + 1));
        }
    }
}

void Nebulite::Interaction::Logic::Expression::parseTokenTypeEval(std::string const& token){
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
    std::shared_ptr<Component> currentComponent =  std::make_shared<Component>();

    size_t pos = token.find('(');
    std::string formatter  = token.substr(1, pos - 1); // Remove leading $
    std::string expression = token.substr(pos);

    // Check cast type in formatter:
    if(formatter.ends_with("i")){
        readFormatter(currentComponent, formatter);
        currentComponent->cast = Component::CastType::to_int;
    }
    else if(formatter.ends_with("f")){
        readFormatter(currentComponent, formatter);
        currentComponent->cast = Component::CastType::to_double;
    }
    else{
        currentComponent->cast = Component::CastType::none;
    }

    // Current token is evaluation
    currentComponent->type = Component::Type::eval;

    // Register internal variables
    // And build equivalent expression
    std::string newString = "";
    std::vector<std::string> subTokens = Nebulite::Utility::StringHandler::splitOnSameDepth(expression, '{');

    for (auto const& subToken : subTokens){
        if(subToken.starts_with('{')){
            // 1.) remove {}
            std::string key, te_name;
            key = Nebulite::Utility::StringHandler::replaceAll(subToken, "{", "");
            key = Nebulite::Utility::StringHandler::replaceAll(key     , "}", "");
            te_name = Nebulite::Utility::StringHandler::replaceAll(key, ".", "_");
            // 2.) determine context
            Component::From context = getContext(key);
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
    currentComponent->type = Component::Type::eval;
    currentComponent->str = newString; // Shouldnt contain any $
    currentComponent->from = Component::From::None;
    currentComponent->key = ""; // No key for eval expressions

    // Add to components
    components.push_back(currentComponent);
}

void Nebulite::Interaction::Logic::Expression::parseTokenTypeText(std::string const& token){
    // Current token is Text
    // Perhaps mixed with variables...
    std::vector<std::string> subTokens = Nebulite::Utility::StringHandler::splitOnSameDepth(token, '{');
    for (auto const& subToken : subTokens){
        std::shared_ptr<Component> currentComponent =  std::make_shared<Component>();

        // Token is type variable
        if(subToken.starts_with('{')){
            // 1.) remove {}
            // We keep all other potential {} inside the variable name for later multiresolve
            std::string inner;
            
            inner = subToken.substr(1, subToken.length() - 2);
            // 2.) determine context
            currentComponent->type = Component::Type::variable;
            currentComponent->str = inner;
            currentComponent->from = getContext(inner);
            currentComponent->key = stripContext(inner);
        }
        // Token is type text
        else{
            // Determine context
            currentComponent->type = Component::Type::text;
            currentComponent->str = subToken;
            currentComponent->from = Component::From::None;
            currentComponent->key = ""; // No key for text expressions
        }
        // Add to components
        components.push_back(currentComponent);
    }
}

void Nebulite::Interaction::Logic::Expression::printCompileError(std::shared_ptr<Component> const& component, int const error){
    Nebulite::Utility::Capture::cerr() << "-----------------------------------------------------------------" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Error compiling expression: '" << component->str << "' Error code: " << std::to_string(error) << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "You might see this message multiple times due to expression parallelization." << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "If you only see the start of your expression, make sure to encompass your expression in quotes" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Some functions assume that the expression is inside, e.g. argv[1]." << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Example: " << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "if $(1+1)     echo here! # works" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "if $(1 + 1)   echo here! # doesnt work!" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "if '$(1 + 1)' echo here! # works" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Registered functions and variables:\n";
    for (auto const& var : te_variables){
        Nebulite::Utility::Capture::cerr() << "\t'" << var.name << "'\n";
    }
    Nebulite::Utility::Capture::cerr() << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Resetting expression to always yield 'nan'" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << Nebulite::Utility::Capture::endl;
}

//------------------------------------------
// Public:

Nebulite::Interaction::Logic::Expression::Expression(){
    _isReturnableAsDouble = false;
    _isAlwaysTrue = false;
    uniqueId = 0;
    reset();
}

void Nebulite::Interaction::Logic::Expression::parse(std::string const& expr, Nebulite::Utility::DocumentCache* documentCache, Nebulite::Utility::JSON* self, Nebulite::Utility::JSON* global){
    reset();

    // Set references
    references.self = self;
    references.global = global;
    references.documentCache = documentCache;
    fullExpression = expr;

    // Parse
    parseIntoComponents(expr);

    // Now compile all components:
    for (auto& component : components){
        compileIfExpression(component);
    }

    auto globalspace = self->getGlobalSpace();
    uniqueId = globalspace->getUniqueId(fullExpression, Nebulite::Core::GlobalSpace::UniqueIdType::EXPRESSION);

    // Calculate optimization flags in-expression only if pools are not used
    #if INVOKE_EXPR_POOL_SIZE == 1
        _isReturnableAsDouble = recalculateIsReturnableAsDouble();
        _isAlwaysTrue = recalculateIsAlwaysTrue();
    #endif
}

bool Nebulite::Interaction::Logic::Expression::handleComponentTypeVariable(std::string& token, std::shared_ptr<Component> const& component, Nebulite::Utility::JSON* current_other, uint16_t max_recursion_depth){
    std::string key = component->key;
    Component::From context = component->from;

    // See if the variable contains an inner expression
    if(component->str.find('$') != std::string::npos || component->str.find('{') != std::string::npos){
        if(max_recursion_depth == 0){
            Nebulite::Utility::Capture::cerr() << "Error: Maximum recursion depth reached when evaluating variable: " << component->key << Nebulite::Utility::Capture::endl;
            return false;
        }
        // Create a temporary expression to evaluate the inner expression
        Expression tempExpr;
        tempExpr.parse(component->str, references.documentCache, references.self, references.global);
        key = tempExpr.eval(current_other, max_recursion_depth - 1);

        // Redetermine context and strip it from key
        context = getContext(key);
        key = stripContext(key);
    }

    // Now, use the key to get the value from the correct document
    switch(context){
        case Component::From::self:
            if(references.self == nullptr){
                Nebulite::Utility::Capture::cerr() << "Error: Null self reference in expression: " << key << Nebulite::Utility::Capture::endl;
                return false;
            }
            token = references.self->get<std::string>(key, "0");
            break;
        case Component::From::other:
            if(current_other == nullptr){
                Nebulite::Utility::Capture::cerr() << "Error: Null other reference in expression: " << key << Nebulite::Utility::Capture::endl;
                return false;
            }
            token = current_other->get<std::string>(key, "0");
            break;
        case Component::From::global:
            if (references.global == nullptr){
                Nebulite::Utility::Capture::cerr() << "Error: Null global reference in expression: " << key << Nebulite::Utility::Capture::endl;
                return false;
            }
            token = references.global->get<std::string>(key, "0");
            break;
        case Component::From::resource:
        default:
            if (references.documentCache == nullptr){
                Nebulite::Utility::Capture::cerr() << "Error: Null globalCache reference in expression: " << key  << ". If this shouldn't be a Resource reference, did you forget the prefix self/other/global?" << Nebulite::Utility::Capture::endl;
                return false;
            }
            token = references.documentCache->get<std::string>(key, "0");
            break;
    }
    return true;
}

void Nebulite::Interaction::Logic::Expression::handleComponentTypeEval(std::string& token, std::shared_ptr<Component> const& component){
    //------------------------------------------
    // Handle casting and precision together
    if(component->cast == Component::CastType::to_int){
        token = std::to_string(static_cast<int>(te_eval(component->expression)));
    } else{
        // to_double or none, both use double directly 
        double value = te_eval(component->expression);
        
        // Apply rounding if precision is specified
        if (component->formatter.precision != -1){
            double multiplier = std::pow(10.0, component->formatter.precision);
            value = std::round(value * multiplier) / multiplier;
        }
        
        token = std::to_string(value);
    }

    // Precision formatting (after rounding)
    if (component->formatter.precision != -1){
        size_t dotPos = token.find('.');
        if (dotPos != std::string::npos){
            size_t currentPrecision = token.size() - dotPos - 1;
            if (currentPrecision < static_cast<size_t>(component->formatter.precision)){
                // Add zeros to match the required precision
                token.append(static_cast<size_t>(component->formatter.precision) - currentPrecision, '0');
            } else if (currentPrecision > static_cast<size_t>(component->formatter.precision)){
                // Truncate to the required precision (should be minimal after rounding)
                token.resize(dotPos + static_cast<size_t>(component->formatter.precision) + 1);
            }
        } else {
            // No decimal point, add one and pad with zeros
            token += '.';
            token.append(static_cast<size_t>(component->formatter.precision), '0');
        }
    }

    // Adding padding
    if(component->formatter.alignment > 0 && token.size() < static_cast<size_t>(component->formatter.alignment)){
        // Cast to int, as alignment may be negative (-1 signals no alignment)
        int size = static_cast<int>(token.size());
        for(int i = 0; i < component->formatter.alignment - size; i++){
            token = (component->formatter.leadingZero ? '0' : ' ') + token;
        }
    }
}

std::string Nebulite::Interaction::Logic::Expression::eval(Nebulite::Utility::JSON* current_other, uint16_t max_recursion_depth){
    //------------------------------------------
    // Update caches so that tinyexpr has the correct references
    updateCaches(current_other);

    //------------------------------------------
    // Evaluate expression

    // Concatenate results of each component
    std::string result = "";
    for (auto const& component : components){
        std::string token = "";
        switch (component->type){
            //------------------------------------------
            case Component::Type::variable:
                if(!handleComponentTypeVariable(token, component, current_other, max_recursion_depth)){
                    return "0";
                }
                break;
            //------------------------------------------
            case Component::Type::eval:
                handleComponentTypeEval(token, component);
                break;
            //------------------------------------------
            case Component::Type::text:
                token = component->str;
                break;
            //------------------------------------------
            default:
                break;
        }
        result += token;
    }
    
    return result;
}

double Nebulite::Interaction::Logic::Expression::evalAsDouble(Nebulite::Utility::JSON* current_other){
    // Update caches so that tinyexpr has the correct references
    updateCaches(current_other);

    // Evaluate expression
    return te_eval(components[0]->expression);
}

odpvec* Nebulite::Interaction::Logic::Expression::ensureOtherOrderedCacheList(Nebulite::Utility::JSON* reference){
    auto cache = reference->getExpressionRefsAsOther();
    std::scoped_lock<std::mutex> cache_lock(cache->mtx);
    
    // Check if we can use quickcache, that does not rely on a hashmap lookup
    if(uniqueId < Utility::MappedOrderedDoublePointers::quickCacheSize){
        if(cache->quickCache[uniqueId].orderedValues.empty()){
            // Not initialized yet, create one with exact size
            Nebulite::Utility::OrderedDoublePointers newCacheList(virtualDoubles.other.size());

            // Populate list with all virtual doubles from type other
            for(auto const& vde : virtualDoubles.other){
                double* ptr = reference->getStableDoublePointer(vde->getKey());
                newCacheList.orderedValues.push_back(ptr);
            }
            cache->quickCache[uniqueId] = std::move(newCacheList);
        }
        return &cache->quickCache[uniqueId].orderedValues;
    }

    // If id is too large for quickcache, use hashmap
    auto it = cache->map.find(uniqueId);
    
    // If not, create one
    if(it == cache->map.end()){
        Nebulite::Utility::OrderedDoublePointers newCacheList(virtualDoubles.other.size());

        // Populate list with all virtual doubles from type other
        for(auto const& vde : virtualDoubles.other){
            double* ptr = reference->getStableDoublePointer(vde->getKey());
            newCacheList.orderedValues.push_back(ptr);
        }

        cache->map.emplace(uniqueId, std::move(newCacheList));
        it = cache->map.find(uniqueId);
    }
    return &it->second.orderedValues;
}

void Nebulite::Interaction::Logic::Expression::updateCaches(Nebulite::Utility::JSON* reference){
    //------------------------------------------
    // 1.) Update other

    // Get a list of all references, insert into virtual doubles
    if(!virtualDoubles.other.empty()){
        auto list = ensureOtherOrderedCacheList(reference);
        auto* list_data = list->data();
        const size_t count = virtualDoubles.other.size();
        for(size_t i = 0; i < count; ++i){
            virtualDoubles.other[i]->setDirect(*list_data[i]);
        }
    }

    //------------------------------------------
    // 2.) Update resource

    // Update resource references
    for(auto& vde : virtualDoubles.resource){
        vde->setUpInternalCache(nullptr);
    }
}

//------------------------------------------
// Recalculation helpers:


bool Nebulite::Interaction::Logic::Expression::recalculateIsReturnableAsDouble(){
    return (components.size() == 1) && (components[0]->type == Component::Type::eval) && (components[0]->cast == Component::CastType::none);
}

bool Nebulite::Interaction::Logic::Expression::recalculateIsAlwaysTrue(){
    return (fullExpression == "1");
}