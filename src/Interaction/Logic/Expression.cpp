//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Interaction/Logic/VirtualDouble.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Math/Equality.hpp"
#include "Math/ExpressionPrimitives.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

//------------------------------------------
// Private:

Expression::~Expression() {
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

void Expression::reset() {
    // Clear existing data
    components.clear();
    te_variables.clear();
    fullExpression.clear();
    components.clear();

    // Clear all variable names
    te_names.clear();

    // Clear all vds
    virtualDoubles.stable.self.clear();
    virtualDoubles.stable.other.clear();
    virtualDoubles.stable.global.clear();

    virtualDoubles.unstable.self.clear();
    virtualDoubles.unstable.other.clear();
    virtualDoubles.unstable.global.clear();
    virtualDoubles.unstable.resource.clear();

    //------------------------------------------
    // Register built-in functions
    Math::ExpressionPrimitives::registerExpressions(te_variables);
}

void Expression::compileIfExpression(std::shared_ptr<Component> const& component) const {
    if (component->type == Component::Type::eval) {
        // Compile the expression using TinyExpr
        int error;
        component->expression = te_compile(component->stringRepresentation.c_str(), te_variables.data(), static_cast<int>(te_variables.size()), &error);
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

namespace {
/**
 * @brief Checks if the expression can be returned as a double pointer.
 *        Meaning no multi-resolve or transformations are used.
 * @param key The key to check.
 * @return true if the expression can be returned as a double pointer, false otherwise.
 */
bool isAvailableAsDoublePtr(std::string const& key) {
    return key.find('{') == std::string::npos
        && key.find('}') == std::string::npos
        && key.find('|') == std::string::npos;
}
} // anonymous namespace

void Expression::registerVariable(std::string te_name, std::string const& key, ContextDeriver::TargetType const& contextType) {
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
        switch (contextType) {
        case ContextDeriver::TargetType::self:
            if (isAvailableAsDoublePtr(key)) {
                virtualDoubles.stable.self.push_back(vd);
            } else {
                virtualDoubles.unstable.self.push_back(vd);
            }
            break;
        case ContextDeriver::TargetType::other:
            // Type other is always non-remanent, as other document reference can change
            // However, we need to distinguish between stable and unstable double pointers
            // Meaning the ones we can get from an ordered list, and the ones we need to resolve each time
            // (e.g. with multi-resolve or transformations)
            if (isAvailableAsDoublePtr(key)) {
                virtualDoubles.stable.other.push_back(vd);
            } else {
                virtualDoubles.unstable.other.push_back(vd);
            }
            break;
        case ContextDeriver::TargetType::local:
            virtualDoubles.unstable.local.push_back(vd);
            break;
        case ContextDeriver::TargetType::global:
            if (isAvailableAsDoublePtr(key)) {
                virtualDoubles.stable.global.push_back(vd);
            } else {
                virtualDoubles.unstable.global.push_back(vd);
            }
            break;
        case ContextDeriver::TargetType::full:
            virtualDoubles.unstable.full.push_back(vd);
            break;
        case ContextDeriver::TargetType::resource:
            virtualDoubles.unstable.resource.push_back(vd);
            break;
        case ContextDeriver::TargetType::none:
            // Use an empty document
            virtualDoubles.unstable.none.push_back(vd);
            break;
        default:
            // Should not happen
            Global::capture().error.println(__FUNCTION__, ": Tried to register variable with no known context!");
            std::unreachable();
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

namespace {
bool isTypeVariable(std::string_view const& str) {
    return str.starts_with('{') && str != "{object}";
}
} // namespace


void Expression::parseIntoComponents(std::string const& expr) {
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
                // Current token is Text
                // Perhaps mixed with variables...
                for (auto const& subToken : Utility::StringHandler::splitOnSameDepth(token, '{')) {
                    // Token is type variable
                    if (isTypeVariable(subToken)) {
                        parseTokenTypeVariable(subToken);
                    }
                    // Token is type text
                    else {
                        parseTokenTypeText(subToken);
                    }
                }
            }
        }
    }
}

Expression::Formatter Expression::Formatter::readFormatter(std::string const& formatter) {
    // Check formatter. Integer cast should not include precision. Is ignored later on in casting but acceptable as input
    // Examples:
    // $i     : leadingZero = false , alignment = -1 , precision = -1
    // $04i   : leadingZero = true  , alignment =  4 , precision = -1
    // $03.5i : leadingZero = true  , alignment =  3 , precision =  5

    Formatter fmt;

    if (formatter.empty()) {
        return fmt;
    }

    // Format cast
    if (formatter.ends_with("i")) {
        fmt.cast = CastType::to_int;
    }
    else if (formatter.ends_with("f")) {
        fmt.cast = CastType::to_double;
    }

    // Read leading zero
    if (formatter.starts_with("0")) {
        fmt.leadingZero = true;
    }
    if (formatter.size() > 1) {
        size_t const dotPos = formatter.find('.');
        // Read alignment
        if (dotPos == 0) {
            fmt.alignment = 0;
        } else {
            fmt.alignment = std::stoi(formatter.substr(0, dotPos));
        }
        // Read precision
        if (dotPos != std::string::npos) {
            fmt.precision = std::stoi(formatter.substr(dotPos + 1));
        }
    }
    return fmt;
}

std::string Expression::Formatter::format(double const& value) const {
    std::string token;
    if (cast == CastType::to_int) {
        token = std::to_string(static_cast<int>(value));
    } else {
        // to_double or none, both use double directly
        double newValue = value;

        // Apply rounding if precision is specified
        if (precision != -1) {
            double const multiplier = std::pow(10.0, precision);
            newValue = std::round(value * multiplier) / multiplier;
        }

        token = std::to_string(newValue);
    }

    // Precision formatting (after rounding)
    if (precision != -1) {
        if (size_t const dotPos = token.find('.'); dotPos != std::string::npos) {
            if (size_t const currentPrecision = token.size() - dotPos - 1; currentPrecision < static_cast<size_t>(precision)) {
                // Add zeros to match the required precision
                token.append(static_cast<size_t>(precision) - currentPrecision, '0');
            } else if (currentPrecision > static_cast<size_t>(precision)) {
                // Truncate to the required precision (should be minimal after rounding)
                token.resize(dotPos + static_cast<size_t>(precision) + 1);
            }
        } else {
            // No decimal point, add one and pad with zeros
            token += '.';
            token.append(static_cast<size_t>(precision), '0');
        }
    }

    // Adding padding
    if (alignment > 0 && token.size() < static_cast<size_t>(alignment)) {
        // Cast to int, as alignment may be negative (-1 signals no alignment)
        int const size = static_cast<int>(token.size());
        std::string padding;
        for (int i = 0; i < alignment - size; i++) {
            if (leadingZero) {
                padding += "0";
            } else {
                padding += " ";
            }
        }
        token.insert(0, padding);
    }
    return token;
}

void Expression::parseTokenTypeEval(std::string const& token) {
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
    currentComponent->formatter = Formatter::readFormatter(formatter);

    // Register internal variables
    // And build equivalent expression using new variable names
    for (auto const& subToken : Utility::StringHandler::splitOnSameDepth(expression, '{')) {
        if (subToken.starts_with('{')) {
            std::string const te_name = varNameGen.getUniqueName(subToken);
            std::string key = subToken.substr(1, subToken.length() - 2);
            ContextDeriver::TargetType contextType = ContextDeriver::getTypeFromString(key);
            key = ContextDeriver::stripContext(key);
            registerVariable(te_name, key, contextType);
            currentComponent->stringRepresentation += te_name;
        } else {
            currentComponent->stringRepresentation += subToken;
        }
    }

    // Write component data
    currentComponent->type = Component::Type::eval;
    currentComponent->contextType = ContextDeriver::TargetType::none; // None, since this is an eval expression
    currentComponent->key = ""; // No key for eval expressions

    // Add to components
    components.push_back(currentComponent);
}

void Expression::parseTokenTypeVariable(std::string const& token) {
    auto const currentComponent = std::make_shared<Component>();

    // 1.) remove {}
    // We keep all other potential {} inside the variable name for later MultiResolve
    std::string inner = token.substr(1, token.length() - 2);

    // 2.) Check if inner starts with a number followed by an exclamation mark, if so, this is an evaluation wait specifier,
    // and we set the evaluation wait of the component accordingly, and remove the specifier from the inner string
    if (auto const exclamationMarkPosition = inner.find('!'); exclamationMarkPosition != std::string::npos) {
        if (std::string const beforeExclamation = inner.substr(0, exclamationMarkPosition); Utility::StringHandler::isNumber(beforeExclamation)) {
            if (!beforeExclamation.empty()) {
                currentComponent->evaluationWait = std::stoul(std::string(beforeExclamation));
            }
            inner = inner.substr(exclamationMarkPosition + 1);
        }
    }

    // 3.) determine context
    currentComponent->type = Component::Type::variable;
    currentComponent->stringRepresentation = inner;
    currentComponent->contextType = ContextDeriver::getTypeFromString(inner);
    currentComponent->key = ContextDeriver::stripContext(inner);

    components.push_back(currentComponent);
}

void Expression::parseTokenTypeText(std::string const& token) {
    auto const currentComponent = std::make_shared<Component>();
    // Determine context
    currentComponent->type = Component::Type::text;
    currentComponent->stringRepresentation = token;
    currentComponent->contextType = ContextDeriver::TargetType::none;
    currentComponent->key = ""; // No key for text expressions
    components.push_back(currentComponent);
}

void Expression::printCompileError(std::shared_ptr<Component> const& component, int const& error) const {
    std::string offendingChar;
    if (error <= 0 || static_cast<size_t>(error) > component->stringRepresentation.size()) {
        offendingChar = "N/A (error position out of bounds)";
    } else {
        offendingChar = std::string(1, component->stringRepresentation[static_cast<size_t>(error) - 1]);
    }
    std::stringstream ss;
    ss << "-----------------------------------------------------------------" << "\n";
    ss << "Error compiling expression: '" << component->stringRepresentation << "' At position: " << std::to_string(error) << ", offending character: " << offendingChar << "\n";
    ss << "You might see this message multiple times due to expression parallelization." << "\n";
    ss << "\n";
    ss << "If you only see the start of your expression, make sure to encompass your expression in quotes" << "\n";
    ss << "Some functions assume that the expression is inside, e.g. argv[1]." << "\n";
    ss << "Example: " << "\n";
    ss << "if $(1+1)     echo here! # works" << "\n";
    ss << "if $(1 + 1)   echo here! # doesnt work!" << "\n";
    ss << "if '$(1 + 1)' echo here! # works" << "\n";
    ss << "\n";
    ss << "Registered functions and variables:\n";
    for (auto const& var : te_variables) {
        ss << "\t'" << var.name << "'\n";
    }
    ss << "\n";
    ss << "Resetting expression to always yield 'nan'" << "\n";
    ss << "\n";
    ss << "\n";

    // Send whole message to cerr at once, to avoid interleaving with other messages
    Global::capture().error.println(ss.str());
}

//------------------------------------------
// Public:

Expression::Expression(std::string const& expr){
    evaluationInfo = {
        .returnableAsDouble = false,
        .returnableAsString = false,
        .alwaysTrue = false
    };
    reset();
    parse(expr);
}

void Expression::parse(std::string const& expr) {
    reset();
    fullExpression = expr;
    parseIntoComponents(expr);
    for (auto& component : components) {
        compileIfExpression(component);
    }
    evaluationInfo.returnableAsDouble = recalculateIsReturnableAsDouble();
    evaluationInfo.returnableAsString = recalculateIsReturnableAsString();
    evaluationInfo.alwaysTrue = recalculateIsAlwaysTrue();

    // Reset variable name generator, data is only needed during parsing
    varNameGen.clear();
}

//------------------------------------------
// Static one-time evaluation

// With context

std::string Expression::eval(std::string const& input, ContextScope const& context) {
    Expression const expr(input);
    return expr.eval(context);
}

double Expression::evalAsDouble(std::string const& input, ContextScope const& context) {
    Expression const expr(input);
    return expr.evalAsDouble(context);
}

bool Expression::evalAsBool(std::string const& input, ContextScope const& context) {
    Expression const expr(input);
    return expr.evalAsBool(context);
}

Data::JSON Expression::evalAsJson(std::string const& input, ContextScope const& context) {
    Expression const expr(input);
    return expr.evalAsJson(context);
}

// Global-only as context

std::string Expression::eval(std::string const& input) {
    ContextScope const context{emptyDoc(), emptyDoc(), globalDoc()};
    return eval(input, context);
}

double Expression::evalAsDouble(std::string const& input) {
    ContextScope const context{emptyDoc(), emptyDoc(), globalDoc()};
    return evalAsDouble(input, context);
}

bool Expression::evalAsBool(std::string const& input) {
    ContextScope const context{emptyDoc(), emptyDoc(), globalDoc()};
    return evalAsBool(input, context);
}

Data::JSON Expression::evalAsJson(std::string const& input) {
    ContextScope const context{emptyDoc(), emptyDoc(), globalDoc()};
    return evalAsJson(input, context);
}

//------------------------------------------
// Private helper functions

void Expression::updateCaches(ContextScope const& context) const {
    setupFirstContext(context);
    updateStableValues(context);
    updateUnstableValues(context);
}

void Expression::setupFirstContext(ContextScope const& context) const {
    if (!firstEvaluationContext.self) {
        firstEvaluationContext.self = &context.self;
        for (auto const& vde : virtualDoubles.stable.self) {
            vde->linkExternalCache(context.self);
        }

        firstEvaluationContext.other = &context.other;
        for (auto const& vde : virtualDoubles.stable.other) {
            vde->linkExternalCache(context.other);
        }

        firstEvaluationContext.global = &context.global;
        for (auto const& vde : virtualDoubles.stable.global) {
            vde->linkExternalCache(context.global);
        }
    }
}

void Expression::updateStableValues(ContextScope const& context) const {
    auto updateContext = [&](auto const& currentContext, auto const& firstContext, auto const& vdList) {
        if (&currentContext == firstContext) {
            for (auto const& vde : vdList) {
                vde->copyExternalCache();
            }
        }
        else {
            for (auto const& vde : vdList) {
                vde->copyFromJson(currentContext);
            }
        }
    };
    updateContext(context.self, firstEvaluationContext.self, virtualDoubles.stable.self);
    updateContext(context.other, firstEvaluationContext.other, virtualDoubles.stable.other);
    updateContext(context.global, firstEvaluationContext.global, virtualDoubles.stable.global);
}

void Expression::updateUnstableValues(ContextScope const& context) const {
    auto updateContext = [&]<typename DataType>(DataType& jsonScope, auto& vdList) {
        for (auto const& vde : vdList) {
            if constexpr (auto const evaluatedKey = eval(vde->getKey(), context); requires { jsonScope.template get<double>(Data::ScopedKey(evaluatedKey)); }) {
                auto key = Data::ScopedKey(evaluatedKey);
                vde->setDirect(jsonScope.template get<double>(key).value_or(0.0));
            } else if constexpr (requires { jsonScope.template get<double>(evaluatedKey); }) {
                vde->setDirect(jsonScope.template get<double>(evaluatedKey).value_or(0.0));
            } else {
                static_assert(Utility::CompileTimeEvaluate::always_false(), "Unsupported key type for get()");
            }
        }
    };
    updateContext(context.self, virtualDoubles.unstable.self);
    updateContext(context.other, virtualDoubles.unstable.other);
    updateContext(context.global, virtualDoubles.unstable.global);
    updateContext(Global::instance().getDocCache(), virtualDoubles.unstable.resource);
    updateContext(emptyDoc(), virtualDoubles.unstable.none);
    if (!virtualDoubles.unstable.local.empty()) {
        Data::JsonScope merged;
        context.combineLocal(merged);
        updateContext(merged, virtualDoubles.unstable.local);
    }
    if (!virtualDoubles.unstable.full.empty()) {
        Data::JsonScope merged;
        context.combineAll(merged);
        updateContext(merged, virtualDoubles.unstable.full);
    }
}

//------------------------------------------
// Recalculation helpers:

bool Expression::recalculateIsReturnableAsDouble() const {
    return components.size() == 1
           && components[0]->type == Component::Type::eval
           && components[0]->formatter.cast == Formatter::CastType::none;
}

bool Expression::recalculateIsReturnableAsString() const {
    return !(components.size() == 1 && components[0]->type == Component::Type::variable);
}

bool Expression::recalculateIsAlwaysTrue() const {
    if (fullExpression.starts_with("$(") && fullExpression.ends_with(")")){
        std::string const innerExpression = fullExpression.substr(2, fullExpression.size() - 3);
        return Utility::StringHandler::isNumber(innerExpression) && !Math::isEqual(0.0,std::stod(innerExpression));
    }
    return Utility::StringHandler::isNumber(fullExpression) && !Math::isEqual(0.0,std::stod(fullExpression));
}

//------------------------------------------
// Actual expression evaluation

std::string Expression::eval(ContextScope const& context, size_t const& recursionDepth) const {
    //------------------------------------------
    // Update caches so that tinyexpr has the correct references
    updateCaches(context);

    //------------------------------------------
    // Evaluate expression
    // Concatenate results of each component
    std::string result;
    for (auto const& component : components) {
        std::string token;
        switch (component->type) {
            //------------------------------------------
        case Component::Type::variable:
            if (!component->handleComponentTypeVariable(token, context, recursionDepth)) {
                token = "null";
            }
            break;
            //------------------------------------------
        case Component::Type::eval:
            component->handleComponentTypeEval(token);
            break;
            //------------------------------------------
        case Component::Type::text:
            token = component->stringRepresentation;
            break;
            //------------------------------------------
        default:
            break;
        }
        result += token;
    }
    return result;
}

double Expression::evalAsDouble(ContextScope const& context) const {
    updateCaches(context);
    return te_eval(components[0]->expression);
}

bool Expression::evalAsBool(ContextScope const& context) const {
    double const result = evalAsDouble(context);
    return !Math::isZero(result);
}

Data::JSON Expression::evalAsJson(ContextScope const& context, size_t const& recursionDepth) const {
    if (components.size() == 1 && components[0]->type != Component::Type::text) {
        if (components[0]->type == Component::Type::eval) {
            Data::JSON jsonResult;
            jsonResult.set<double>("", evalAsDouble(context));
            return jsonResult;
        }
        if (components[0]->type == Component::Type::variable) {
            Data::JSON jsonResult;
            components[0]->handleComponentTypeVariable(jsonResult, context, recursionDepth);
            return jsonResult;
        }
    }
    Data::JSON jsonResult;
    jsonResult.set<std::string>("", eval(context, recursionDepth));
    return jsonResult;
}

//------------------------------------------
// Static document access

Data::JsonScope& Expression::emptyDoc() {
    thread_local Data::JsonScope emptyDoc;
    return emptyDoc;
}

Data::JsonScope& Expression::globalDoc() {
    static auto accessToken = ScopeAccessor::Full();
    static auto& globalDoc = Global::shareScope(accessToken, "");
    return globalDoc;
}

}   // namespace Nebulite::Interaction::Logic
