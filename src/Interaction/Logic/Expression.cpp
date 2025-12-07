#include "Interaction/Logic/Expression.hpp"

#include <cmath>

#include "Nebulite.hpp"

//------------------------------------------
// Private:

Nebulite::Interaction::Logic::Expression::~Expression() {
    // reset all data
    reset();

    // Remove variables
    te_variables.clear();

    // Clear all expressions
    for (auto const& component : components) {
        if (component->expression != nullptr) {
            te_free(component->expression);
            component->expression = nullptr;
        }
    }
}

void Nebulite::Interaction::Logic::Expression::reset() {
    references.self = nullptr;

    // Clear existing data
    components.clear();
    te_variables.clear();
    fullExpression.clear();
    components.clear();

    // Clear all variable names
    te_names.clear();

    // Clear all vds
    virtualDoubles.remanent.self.clear();
    virtualDoubles.remanent.global.clear();
    virtualDoubles.nonRemanent.self.clear();
    virtualDoubles.nonRemanent.other.clear();
    virtualDoubles.nonRemanent.global.clear();
    virtualDoubles.nonRemanent.resource.clear();

    //------------------------------------------
    // Register built-in functions

    //====================================================================================================================
    // Category             Name           Pointer                                               Type           Context
    //====================================================================================================================

    // Logical comparison functions
    te_variables.push_back({"gt", reinterpret_cast<void*>(expr_custom::gt), TE_FUNCTION2, nullptr});
    te_variables.push_back({"lt", reinterpret_cast<void*>(expr_custom::lt), TE_FUNCTION2, nullptr});
    te_variables.push_back({"geq", reinterpret_cast<void*>(expr_custom::geq), TE_FUNCTION2, nullptr});
    te_variables.push_back({"leq", reinterpret_cast<void*>(expr_custom::leq), TE_FUNCTION2, nullptr});
    te_variables.push_back({"eq", reinterpret_cast<void*>(expr_custom::eq), TE_FUNCTION2, nullptr});
    te_variables.push_back({"neq", reinterpret_cast<void*>(expr_custom::neq), TE_FUNCTION2, nullptr});

    // Logical gate functions
    te_variables.push_back({"not", reinterpret_cast<void*>(expr_custom::logical_not), TE_FUNCTION1, nullptr});
    te_variables.push_back({"and", reinterpret_cast<void*>(expr_custom::logical_and), TE_FUNCTION2, nullptr});
    te_variables.push_back({"or", reinterpret_cast<void*>(expr_custom::logical_or), TE_FUNCTION2, nullptr});
    te_variables.push_back({"xor", reinterpret_cast<void*>(expr_custom::logical_xor), TE_FUNCTION2, nullptr});
    te_variables.push_back({"nand", reinterpret_cast<void*>(expr_custom::logical_nand), TE_FUNCTION2, nullptr});
    te_variables.push_back({"nor", reinterpret_cast<void*>(expr_custom::logical_nor), TE_FUNCTION2, nullptr});
    te_variables.push_back({"xnor", reinterpret_cast<void*>(expr_custom::logical_xnor), TE_FUNCTION2, nullptr});

    // Other logical functions
    te_variables.push_back({"to_bipolar", reinterpret_cast<void*>(expr_custom::to_bipolar), TE_FUNCTION1, nullptr});

    // Mapping functions
    te_variables.push_back({"map", reinterpret_cast<void*>(expr_custom::map), TE_FUNCTION5, nullptr});
    te_variables.push_back({"constrain", reinterpret_cast<void*>(expr_custom::constrain), TE_FUNCTION3, nullptr});

    // More mathematical functions
    te_variables.push_back({"sgn", reinterpret_cast<void*>(expr_custom::sgn), TE_FUNCTION1, nullptr});
}

std::string Nebulite::Interaction::Logic::Expression::stripContext(std::string const& key) {
    constexpr std::array<std::string_view, 3> prefixes = {"self.", "other.", "global."};
    auto const it = std::ranges::find_if(prefixes, [&](std::string_view const p) {
        return key.size() >= p.size() && std::equal(p.begin(), p.end(), key.begin());
    });
    if (it != prefixes.end()) {
        return key.substr(it->size());
    }
    return key;
}

Nebulite::Interaction::Logic::Expression::Component::From Nebulite::Interaction::Logic::Expression::getContext(std::string const& key) {
    if (key.starts_with("self.")) {
        return Component::From::self;
    }
    if (key.starts_with("other.")) {
        return Component::From::other;
    }
    if (key.starts_with("global.")) {
        return Component::From::global;
    }
    return Component::From::resource;
}

void Nebulite::Interaction::Logic::Expression::compileIfExpression(std::shared_ptr<Component> const& component) const {
    if (component->type == Component::Type::eval) {
        // Compile the expression using TinyExpr
        int error;
        component->expression = te_compile(component->str.c_str(), te_variables.data(), static_cast<int>(te_variables.size()), &error);
        if (error) {
            printCompileError(component, error);

            // Resetting expression to nan, as explained in error print:
            // using nan directly is not supported.
            // 0/0 directly yields -nan, so we use abs(0/0)
            te_free(component->expression);
            component->expression = te_compile("abs(0/0)", te_variables.data(), static_cast<int>(te_variables.size()), &error);
        }
    }
}

/**
 * @brief Checks if the expression can be returned as a double pointer.
 *        Meaning no multi-resolve or transformations are used.
 */
bool isAvailableAsDoublePtr(std::string const& key) {
    return (key.find('{') == std::string::npos && key.find('}') == std::string::npos && key.find('|') == std::string::npos);
}

void Nebulite::Interaction::Logic::Expression::registerVariable(std::string te_name, std::string const& key, Component::From const& context) {
    // Check if variable exists in variables vector:
    bool const found = std::ranges::any_of(te_variables, [&](auto const& te_var) {
        if (te_var.name == te_name) {
            return true;
        }
        return false;
    });

    if (!found) {
        // Initialize with reference to document and cache register
        auto const vd = std::make_shared<VirtualDouble>(key);

        // Register cache based on context
        switch (context) {
        case Component::From::self:
            if (isAvailableAsDoublePtr(key)) {
                vd->setUpExternalCache(references.self);
                virtualDoubles.remanent.self.push_back(vd);
            } else {
                vd->setUpExternalCache(references.self);
                virtualDoubles.nonRemanent.self.push_back(vd);
            }
            break;
        case Component::From::other:
            // Type other is always non-remanent, as other document reference can change
            // However, we need to distinguish between stable and unstable double pointers
            // Meaning the ones we can get from an ordered list, and the ones we need to resolve each time
            // (e.g. with multi-resolve or transformations)
            if (isAvailableAsDoublePtr(key)) {
                virtualDoubles.nonRemanent.other.push_back(vd);
            } else {
                virtualDoubles.nonRemanent.otherUnStable.push_back(vd);
            }
            break;
        case Component::From::global:
            if (isAvailableAsDoublePtr(key)) {
                vd->setUpExternalCache(Nebulite::global().getDoc());
                virtualDoubles.remanent.global.push_back(vd);
            } else {
                virtualDoubles.nonRemanent.global.push_back(vd);
            }
            break;
        case Component::From::resource:
            // Type resource is always non-remanent, as document cache can change (?)
            // TODO: double check if remanent resource variables are possible
            virtualDoubles.nonRemanent.resource.push_back(vd);
            break;
        case Component::From::None:
        default:
            // Should not happen
            Nebulite::cerr() << __FUNCTION__ << ": Tried to register variable with no known context!" << Nebulite::endl;
            break;
        }

        // Store variable name for tinyexpr
        auto const te_name_ptr = std::make_shared<std::string>(te_name);
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

void Nebulite::Interaction::Logic::Expression::parseIntoComponents(std::string const& expr) {
    // First, we must split the expression into tokens
    // Split, keep delimiter(at start)
    // "abc$def$ghi" -> ["abc", "$def", "$ghi"]
    std::vector<std::string> const tokensPhase1 = Utility::StringHandler::split(expr, '$', true);
    std::vector<std::string> tokens;

    // Now we need to split on same depth
    for (auto const& token : tokensPhase1) {
        // If the first token starts with '$', it means the string started with '$'
        // If not, the first token is text before the first '$'
        if (token.starts_with('$')) {
            // Remove everything until a '('
            // This part represents the '$' + formatter
            // Cannot be used, as splitOnSameDepth expects the first character to be the opening parenthesis
            std::string start = token.substr(0, token.find('('));
            std::string tokenWithoutStart = token.substr(start.length()); // Remove the leading '$'

            // Split on same depth
            std::vector<std::string> subTokens = Utility::StringHandler::splitOnSameDepth(tokenWithoutStart, '(');

            // Add back the '$' + formatter to first subToken
            if (!subTokens.empty()) {
                subTokens[0] = start + subTokens[0];
            }

            // Add all subtokens to the actual list of tokens
            std::ranges::copy(subTokens, std::back_inserter(tokens));
        } else {
            // If it doesn't start with a '$', it's a text token / potentially with variables inside
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
    for (auto const& token : tokens) {
        if (!token.empty()) {
            if (token.starts_with('$')) {
                parseTokenTypeEval(token);
            } else {
                parseTokenTypeText(token);
            }
        }
    }
}

void Nebulite::Interaction::Logic::Expression::readFormatter(std::shared_ptr<Component> const& component, std::string const& formatter) {
    // Check formatter. Integer cast should not include precision. Is ignored later on in casting but acceptable as input
    // Examples:
    // $i     : leadingZero = false , alignment = -1 , precision = -1
    // $04i   : leadingZero = true  , alignment =  4 , precision = -1
    // $03.5i : leadingZero = true  , alignment =  3 , precision =  5

    if (formatter.empty()) {
        return;
    }

    // Read leading zero
    if (formatter.starts_with("0")) {
        component->formatter.leadingZero = true;
    }
    if (formatter.size() > 1) {
        size_t const dotPos = formatter.find('.');
        // Read alignment
        if (dotPos == 0) {
            component->formatter.alignment = 0;
        } else {
            component->formatter.alignment = std::stoi(formatter.substr(0, dotPos));
        }
        // Read precision
        if (dotPos != std::string::npos) {
            component->formatter.precision = std::stoi(formatter.substr(dotPos + 1));
        }
    }
}

void Nebulite::Interaction::Logic::Expression::parseTokenTypeEval(std::string const& token) {
    // $[leading zero][alignment][.][precision]<type:f,i>
    // - bool leading zero   : on/off
    // - int alignment       : <0 means no formatting
    // - int precision       : <0 means no formatting
    // - CastType::none is then used to determine if we can simply use the double return from tinyexpr

    // 1.) find next '(' and split into formatter and token
    // Examples:
    // input        formatter       expression
    // $(1+1)       ""              "(1+1)"
    // $f(1.23)     "f"             "(1.23)"
    // $i(42)       "i"             "(42)"
    // $4.2f(2/3)   "4.2f"          "(2/3)"
    auto const currentComponent = std::make_shared<Component>();

    size_t const pos = token.find('(');
    std::string const formatter = token.substr(1, pos - 1); // Remove leading $
    std::string const expression = token.substr(pos);

    // Check cast type in formatter:
    if (formatter.ends_with("i")) {
        readFormatter(currentComponent, formatter);
        currentComponent->cast = Component::CastType::to_int;
    } else if (formatter.ends_with("f")) {
        readFormatter(currentComponent, formatter);
        currentComponent->cast = Component::CastType::to_double;
    } else {
        currentComponent->cast = Component::CastType::none;
    }

    // Register internal variables
    // And build equivalent expression using new variable names
    for (auto const& subToken : Utility::StringHandler::splitOnSameDepth(expression, '{')) {
        if (subToken.starts_with('{')) {
            std::string te_name = varNameGen.getUniqueName(subToken);
            std::string key = subToken.substr(1, subToken.length() - 2);
            Component::From context = getContext(key);
            key = stripContext(key);
            registerVariable(te_name, key, context);
            currentComponent->str += te_name;
        } else {
            currentComponent->str += subToken;
        }
    }

    // Write component data
    currentComponent->type = Component::Type::eval;
    currentComponent->from = Component::From::None; // None, since this is an eval expression
    currentComponent->key = ""; // No key for eval expressions

    // Add to components
    components.push_back(currentComponent);
}

void Nebulite::Interaction::Logic::Expression::parseTokenTypeText(std::string const& token) {
    // Current token is Text
    // Perhaps mixed with variables...
    for (auto const& subToken : Utility::StringHandler::splitOnSameDepth(token, '{')) {
        auto currentComponent = std::make_shared<Component>();

        // Token is type variable
        if (subToken.starts_with('{')) {
            // 1.) remove {}
            // We keep all other potential {} inside the variable name for later MultiResolve
            std::string inner;

            inner = subToken.substr(1, subToken.length() - 2);
            // 2.) determine context
            currentComponent->type = Component::Type::variable;
            currentComponent->str = inner;
            currentComponent->from = getContext(inner);
            currentComponent->key = stripContext(inner);
        }
        // Token is type text
        else {
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

void Nebulite::Interaction::Logic::Expression::printCompileError(std::shared_ptr<Component> const& component, int const& error) const {
    std::string offendingChar;
    if (error <= 0 || static_cast<size_t>(error) > component->str.size()) {
        offendingChar = "N/A (error position out of bounds)";
    } else {
        offendingChar = std::string(1, component->str[error - 1]);
    }
    Nebulite::cerr() << "-----------------------------------------------------------------" << Nebulite::endl;
    Nebulite::cerr() << "Error compiling expression: '" << component->str << "' At position: " << std::to_string(error) << ", offending character: " << offendingChar << Nebulite::endl;
    Nebulite::cerr() << "You might see this message multiple times due to expression parallelization." << Nebulite::endl;
    Nebulite::cerr() << Nebulite::endl;
    Nebulite::cerr() << "If you only see the start of your expression, make sure to encompass your expression in quotes" << Nebulite::endl;
    Nebulite::cerr() << "Some functions assume that the expression is inside, e.g. argv[1]." << Nebulite::endl;
    Nebulite::cerr() << "Example: " << Nebulite::endl;
    Nebulite::cerr() << "if $(1+1)     echo here! # works" << Nebulite::endl;
    Nebulite::cerr() << "if $(1 + 1)   echo here! # doesnt work!" << Nebulite::endl;
    Nebulite::cerr() << "if '$(1 + 1)' echo here! # works" << Nebulite::endl;
    Nebulite::cerr() << Nebulite::endl;
    Nebulite::cerr() << "Registered functions and variables:\n";
    for (auto const& var : te_variables) {
        Nebulite::cerr() << "\t'" << var.name << "'\n";
    }
    Nebulite::cerr() << Nebulite::endl;
    Nebulite::cerr() << "Resetting expression to always yield 'nan'" << Nebulite::endl;
    Nebulite::cerr() << Nebulite::endl;
    Nebulite::cerr() << Nebulite::endl;
}

//------------------------------------------
// Public:

Nebulite::Interaction::Logic::Expression::Expression() {
    _isReturnableAsDouble = false;
    _isAlwaysTrue = false;
    uniqueId = 0;
    reset();
}

void Nebulite::Interaction::Logic::Expression::parse(std::string const& expr, Data::JSON* self) {
    reset();
    references.self = self;
    fullExpression = expr;
    uniqueId = global().getUniqueId(fullExpression, Core::GlobalSpace::UniqueIdType::expression);
    parseIntoComponents(expr);
    for (auto& component : components) {
        compileIfExpression(component);
    }

    // Calculate optimization flags in-expression only if pools are not used
#if INVOKE_EXPR_POOL_SIZE == 1
    _isReturnableAsDouble = recalculateIsReturnableAsDouble();
    _isAlwaysTrue = recalculateIsAlwaysTrue();
#endif

    // Reset variable name generator, data is only needed during parsing
    varNameGen.clear();
}

bool Nebulite::Interaction::Logic::Expression::handleComponentTypeVariable(std::string& token, std::shared_ptr<Component> const& component, Data::JSON* current_other, uint16_t const& maximumRecursionDepth) const {
    std::string key = component->key;
    Component::From context = component->from;

    // See if the variable contains an inner expression
    if (component->str.find('$') != std::string::npos || component->str.find('{') != std::string::npos) {
        if (maximumRecursionDepth == 0) {
            Nebulite::cerr() << "Error: Maximum recursion depth reached when evaluating variable: " << component->key << Nebulite::endl;
            return false;
        }
        // Create a temporary expression to evaluate the inner expression
        Expression tempExpr;
        tempExpr.parse(component->str, references.self);
        key = tempExpr.eval(current_other, maximumRecursionDepth - 1);

        // Redetermine context and strip it from key
        context = getContext(key);
        key = stripContext(key);
    }

    // Now, use the key to get the value from the correct document
    switch (context) {
    case Component::From::self:
        if (references.self == nullptr) {
            Nebulite::cerr() << "Error: Null self reference in expression: " << key << Nebulite::endl;
            return false;
        }
        token = references.self->get<std::string>(key, "null");
        break;
    case Component::From::other:
        if (current_other == nullptr) {
            Nebulite::cerr() << "Error: Null other reference in expression: " << key << Nebulite::endl;
            return false;
        }
        token = current_other->get<std::string>(key, "null");
        break;
    case Component::From::global:
        token = Nebulite::global().getDoc()->get<std::string>(key, "null");
        break;
    case Component::From::resource:
    default:
        token = Nebulite::global().getDocCache()->get<std::string>(key, "null");
        break;
    }
    return true;
}

void Nebulite::Interaction::Logic::Expression::handleComponentTypeEval(std::string& token, std::shared_ptr<Component> const& component) {
    //------------------------------------------
    // Handle casting and precision together
    if (component->cast == Component::CastType::to_int) {
        token = std::to_string(static_cast<int>(te_eval(component->expression)));
    } else {
        // to_double or none, both use double directly
        double value = te_eval(component->expression);

        // Apply rounding if precision is specified
        if (component->formatter.precision != -1) {
            double const multiplier = std::pow(10.0, component->formatter.precision);
            value = std::round(value * multiplier) / multiplier;
        }

        token = std::to_string(value);
    }

    // Precision formatting (after rounding)
    if (component->formatter.precision != -1) {
        if (size_t const dotPos = token.find('.'); dotPos != std::string::npos) {
            if (size_t const currentPrecision = token.size() - dotPos - 1; currentPrecision < static_cast<size_t>(component->formatter.precision)) {
                // Add zeros to match the required precision
                token.append(static_cast<size_t>(component->formatter.precision) - currentPrecision, '0');
            } else if (currentPrecision > static_cast<size_t>(component->formatter.precision)) {
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
    if (component->formatter.alignment > 0 && token.size() < static_cast<size_t>(component->formatter.alignment)) {
        // Cast to int, as alignment may be negative (-1 signals no alignment)
        int const size = static_cast<int>(token.size());
        std::string padding;
        for (int i = 0; i < component->formatter.alignment - size; i++) {
            if (component->formatter.leadingZero) {
                padding += "0";
            } else {
                padding += " ";
            }
        }
        token.insert(0, padding);
    }
}

std::string Nebulite::Interaction::Logic::Expression::eval(Data::JSON* current_other, uint16_t const& max_recursion_depth) {
    //------------------------------------------
    // Update caches so that tinyexpr has the correct references
    updateCaches(current_other);

    //------------------------------------------
    // Evaluate expression

    // Concatenate results of each component
    std::string result;
    for (auto const& component : components) {
        std::string token;
        switch (component->type) {
        //------------------------------------------
        case Component::Type::variable:
            if (!handleComponentTypeVariable(token, component, current_other, max_recursion_depth)) {
                return "null";
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

double Nebulite::Interaction::Logic::Expression::evalAsDouble(Data::JSON* current_other) {
    // Update caches so that tinyexpr has the correct references
    updateCaches(current_other);

    // Evaluate expression
    return te_eval(components[0]->expression);
}

void Nebulite::Interaction::Logic::Expression::updateCaches(Data::JSON* reference) {

    // Update self references that are non-remanent
    for (auto const& vde : virtualDoubles.nonRemanent.self) {
        // One-time handle of multi-resolve and transformations
        Expression tempExpr;
        tempExpr.parse(vde->getKey(), references.self);
        std::string const evalResult = tempExpr.eval(reference);
        vde->setDirect(reference->get<double>(evalResult, 0.0));
    }

    // Updating context other: Values with stable double pointers
    // We can use a cached list of double pointers for this, speeding up access significantly
    // Document other stores a list of ordered double pointers for our expression
    // So we only have one query to get all pointers instead of querying each variable individually
    if (!virtualDoubles.nonRemanent.other.empty()) {
        auto const* listData = reference->getExpressionRefs()->ensureOrderedCacheList(uniqueId, reference, virtualDoubles.nonRemanent.other)->data();
        const size_t count = virtualDoubles.nonRemanent.other.size();
        for (size_t i = 0; i < count; ++i) {
            virtualDoubles.nonRemanent.other[i]->setDirect(*listData[i]);
        }
    }

    // Updating context other: Values without stable double pointers
    for (auto const& vde : virtualDoubles.nonRemanent.otherUnStable) {
        // One-time handle of multi-resolve and transformations
        Expression tempExpr;
        tempExpr.parse(vde->getKey(), references.self);
        std::string const evalResult = tempExpr.eval(reference);
        vde->setDirect(reference->get<double>(evalResult, 0.0));
    }

    // Update global references that are non-remanent
    for (auto const& vde : virtualDoubles.nonRemanent.global) {
        // One-time handle of multi-resolve and transformations
        Expression tempExpr;
        tempExpr.parse(vde->getKey(), references.self);
        std::string const evalResult = tempExpr.eval(reference);
        auto const val = Nebulite::global().getDoc()->get<double>(evalResult, 0.0);
        vde->setDirect(val);
    }

    // Update resource references
    for (auto const& vde : virtualDoubles.nonRemanent.resource) {
        if (isAvailableAsDoublePtr(vde->getKey())) {
            vde->setUpInternalCache(nullptr);
        } else {
            // One-time handle of multi-resolve and transformations
            Expression tempExpr;
            tempExpr.parse(vde->getKey(), references.self);
            std::string const evalResult = tempExpr.eval(reference);
            vde->setDirect(Nebulite::global().getDocCache()->get<double>(evalResult, 0.0));
        }
    }
}

//------------------------------------------
// Recalculation helpers:


bool Nebulite::Interaction::Logic::Expression::recalculateIsReturnableAsDouble() const {
    return components.size() == 1
           && components[0]->type == Component::Type::eval
           && components[0]->cast == Component::CastType::none;
}

bool Nebulite::Interaction::Logic::Expression::recalculateIsAlwaysTrue() const {
    return fullExpression == "1";
}