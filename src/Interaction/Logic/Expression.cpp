//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint> // NOLINT
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// External
#include <tinyexpr.h>

// Nebulite
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Interaction/Logic/ExpressionComponent.hpp"
#include "Nebulite/Interaction/Logic/LinkedNumericValue.hpp"
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Math/ExpressionPrimitives.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/CompileTimeEvaluate.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

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
    linkedNumericValues.stable.self.clear();
    linkedNumericValues.stable.other.clear();
    linkedNumericValues.stable.global.clear();

    linkedNumericValues.unstable.self.clear();
    linkedNumericValues.unstable.other.clear();
    linkedNumericValues.unstable.global.clear();
    linkedNumericValues.unstable.resource.clear();

    //------------------------------------------
    // Register built-in functions
    Math::ExpressionPrimitives::registerExpressions(te_variables);
}

void Expression::compileIfExpression(std::shared_ptr<ExpressionComponent> const& component) const {
    if (component->type == ExpressionComponent::Type::eval) {
        // Compile the expression using TinyExpr
        int error{};
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
 * @param key The key to check, without context!
 * @return true if the expression can be returned as a double pointer, false otherwise.
 */
bool isAvailableAsDoublePtr(std::string_view const key) {
    return !key.contains('{')
        && !key.contains('}')
        && !key.contains('|');
}
} // anonymous namespace

double* Expression::LinkedNumericValueLists::registerVariable(ContextDeriver::TargetType const contextType, std::string_view const key){
    auto const vd = std::make_shared<LinkedNumericValue>(key);
    switch (contextType) {
    case ContextDeriver::TargetType::self:
        if (isAvailableAsDoublePtr(key)) {
            stable.self.push_back(vd);
        } else {
            unstable.self.push_back(vd);
        }
        break;
    case ContextDeriver::TargetType::other:
        // Type other is always non-remanent, as other document reference can change
        // However, we need to distinguish between stable and unstable double pointers
        // Meaning the ones we can get from an ordered list, and the ones we need to resolve each time
        // (e.g. with multi-resolve or transformations)
        if (isAvailableAsDoublePtr(key)) {
            stable.other.push_back(vd);
        } else {
            unstable.other.push_back(vd);
        }
        break;
    case ContextDeriver::TargetType::local:
        unstable.local.push_back(vd);
        break;
    case ContextDeriver::TargetType::global:
        if (isAvailableAsDoublePtr(key)) {
            stable.global.push_back(vd);
        } else {
            unstable.global.push_back(vd);
        }
        break;
    case ContextDeriver::TargetType::full:
        unstable.full.push_back(vd);
        break;
    case ContextDeriver::TargetType::resource:
        unstable.resource.push_back(vd);
        break;
    case ContextDeriver::TargetType::none:
        // Use an empty document
        unstable.none.push_back(vd);
        break;
    default:
        // Should not happen
        Global::capture().error.println(__FUNCTION__, ": Tried to register variable with no known context!");
        std::unreachable();
    }
    return vd->ptr();
}

void Expression::registerVariable(std::string te_name, std::string_view const key, ContextDeriver::TargetType const contextType) {
    // Check if variable exists in variables vector:
    bool const found = std::ranges::any_of(te_variables, [&](auto const& te_var) {
        return te_var.name == te_name;
    });

    if (!found) {
        // Register cache based on context
        auto* const ptr = linkedNumericValues.registerVariable(contextType, key);

        // Store variable name for tinyexpr
        auto const te_name_ptr = std::make_shared<std::string>(te_name);
        te_names.push_back(te_name_ptr);

        // Push back into variable components
        te_variables.push_back({
            .name=te_names.back()->c_str(),
            .address=ptr,
            .type=TE_VARIABLE,
            .context=nullptr
        });
    }
}

namespace {

bool isTypeVariable(std::string_view const str) {
    return str.starts_with('{') && str != "{object}";
}

std::vector<std::string> getTokens(std::string_view const expr) {
    // First, we must split the expression into tokens
    // Split, keep delimiter(at start)
    // "abc$def$ghi" -> ["abc", "$def", "$ghi"]
    auto tokensPhase1 = Utility::StringHandler::split(expr, '$', true);

    // Combine tokens where the amount of `(` + `{` and `}` + `)` are not the same
    std::vector<std::string> tokensPhase2;
    std::string currentToken;
    for (auto& token : tokensPhase1) {
        int pCount = 0;
        int bCount = 0;
        pCount += static_cast<int>(std::ranges::count(token, '('));
        pCount -= static_cast<int>(std::ranges::count(token, ')'));
        bCount += static_cast<int>(std::ranges::count(token, '{'));
        bCount -= static_cast<int>(std::ranges::count(token, '}'));

        // If the current token has no mismatch, push back
        if (pCount == 0 && bCount == 0 && !currentToken.empty()) {
            tokensPhase2.push_back(currentToken);
            currentToken = "";
        }

        // TODO: needs some more logic so that, for example, the following is still split properly:
        //       "Value is: ($(1+1))" -> ["Value is: (", "$(1+1)", ")"]
        //       Currently, this would be split into a single token
        //       Sadly this isn't as straightforward as it seems...

        // Add the token to the current token
        currentToken += token;
    }
    if (!currentToken.empty()) {
        tokensPhase2.push_back(currentToken);
    }

    // Now we need to split on same depth
    std::vector<std::string> tokens;
    for (auto const& token : tokensPhase2) {
        // If the first token starts with '$', it means the string started with '$'
        // If not, the first token is text before the first '$'
        if (token.starts_with('$')) {
            // Remove everything until a '('
            // This part represents the '$' + formatter
            // Cannot be used, as splitOnSameDepth expects the first character to be the opening parenthesis
            auto const start = token.substr(0, token.find('('));
            auto const tokenWithoutStart = token.substr(start.length()); // Remove the leading '$'

            // Split on same depth
            auto subTokens = Utility::StringHandler::splitOnSameDepthOf(tokenWithoutStart, Utility::StringHandler::Delimiter::parentheses);

            // Add back the '$' + formatter to first subToken
            if (!subTokens.empty()) {
                tokens.push_back(std::string(start) + std::string(subTokens[0]));
                subTokens.erase(subTokens.begin()); // Remove the first token, as we already added it with the start
            }

            // Add all subtokens to the actual list of tokens
            std::ranges::for_each(subTokens, [&tokens](std::string_view const entry) {
                tokens.emplace_back(entry);
            });
        } else {
            // If it doesn't start with a '$', it's a text token / potentially with variables inside
            // Just add the text token
            tokens.emplace_back(token);
        }
    }
    return tokens;
}

} // namespace

void Expression::parseIntoComponents(std::string_view const expr) {
    for (auto const& token : getTokens(expr)) {
        if (!token.empty()) {
            if (token.starts_with('$')) {
                parseTokenTypeEval(token);
            } else {
                // Current token is Text
                // Perhaps mixed with variables...
                for (auto const& subToken : Utility::StringHandler::splitOnSameDepthOf(token, Utility::StringHandler::Delimiter::brace)) {
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


void Expression::parseTokenTypeEval(std::string_view const token) {
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
    auto const currentComponent = std::make_shared<ExpressionComponent>();

    std::size_t const pos = token.find('(');
    auto const formatter = token.substr(1, pos - 1); // Remove leading $
    auto const expression = token.substr(pos);
    currentComponent->formatter = Formatter::readFormatter(formatter);

    // Register internal variables
    // And build equivalent expression using new variable names
    for (auto const& subToken : Utility::StringHandler::splitOnSameDepthOf(expression, Utility::StringHandler::Delimiter::brace)) {
        if (subToken.starts_with('{')) {
            auto const te_name = varNameGen.getUniqueName(subToken);
            auto key = ContextDeriver::stripContext(subToken.substr(1, subToken.length() - 2));
            auto const contextType = ContextDeriver::getTypeFromString(subToken.substr(1, subToken.length() - 2));
            registerVariable(te_name, key, contextType);
            currentComponent->stringRepresentation += te_name;
        } else {
            currentComponent->stringRepresentation += subToken;
        }
    }

    // Write component data
    currentComponent->type = ExpressionComponent::Type::eval;
    currentComponent->contextType = ContextDeriver::TargetType::none; // None, since this is an eval expression
    currentComponent->key = ""; // No key for eval expressions

    // Add to components
    components.push_back(currentComponent);
}

void Expression::parseTokenTypeVariable(std::string_view const token) {
    auto const currentComponent = std::make_shared<ExpressionComponent>();

    // 1.) remove {}
    // We keep all other potential {} inside the variable name for later MultiResolve
    auto inner = token.substr(1, token.length() - 2);

    // 2.) Check if inner starts with a number followed by an exclamation mark, if so, this is an evaluation wait specifier,
    // and we set the evaluation wait of the component accordingly, and remove the specifier from the inner string
    if (auto const exclamationMarkPosition = inner.find('!'); exclamationMarkPosition != std::string::npos) {
        if (auto const beforeExclamation = inner.substr(0, exclamationMarkPosition); Utility::StringHandler::isNumber(beforeExclamation)) {
            if (!beforeExclamation.empty()) {
                currentComponent->evaluationWait = std::stoul(std::string(beforeExclamation));
            }
            inner = inner.substr(exclamationMarkPosition + 1);
        }
    }

    // 3.) determine context
    currentComponent->type = ExpressionComponent::Type::variable;
    currentComponent->stringRepresentation = inner;
    currentComponent->contextType = ContextDeriver::getTypeFromString(inner);
    currentComponent->key = ContextDeriver::stripContext(inner);

    components.push_back(currentComponent);
}

void Expression::parseTokenTypeText(std::string_view const token) {
    auto const currentComponent = std::make_shared<ExpressionComponent>();
    // Determine context
    currentComponent->type = ExpressionComponent::Type::text;
    currentComponent->stringRepresentation = token;
    currentComponent->contextType = ContextDeriver::TargetType::none;
    currentComponent->key = ""; // No key for text expressions
    components.push_back(currentComponent);
}

void Expression::printCompileError(std::shared_ptr<ExpressionComponent> const& component, int const error) const {
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

Expression::Expression(std::string_view const expr){
    evaluationInfo = {
        .returnableAsDouble = false,
        .returnableAsString = false,
        .alwaysTrue = false
    };
    reset();
    parse(expr);
}

void Expression::parse(std::string_view const expr) {
    reset();
    fullExpression = expr;
    parseIntoComponents(fullExpression);
    for (auto& component : components) {
        compileIfExpression(component);
    }
    evaluationInfo.returnableAsDouble = recalculateIsReturnableAsDouble();
    evaluationInfo.returnableAsInt = recalculateIsReturnableAsInt();
    evaluationInfo.returnableAsString = recalculateIsReturnableAsString();
    evaluationInfo.alwaysTrue = recalculateIsAlwaysTrue();

    // Reset variable name generator, data is only needed during parsing
    varNameGen.clear();
}

//------------------------------------------
// Static one-time evaluation

std::string Expression::eval(std::string_view const input, ContextScope const& context) {
    Expression const expr(input);
    return expr.eval(context);
}

double Expression::evalAsDouble(std::string_view const input, ContextScope const& context) {
    Expression const expr(input);
    return expr.evalAsDouble(context);
}

bool Expression::evalAsBool(std::string_view const input, ContextScope const& context) {
    Expression const expr(input);
    return expr.evalAsBool(context);
}

Data::JSON Expression::evalAsJson(std::string_view const input, ContextScope const& context) {
    Expression const expr(input);
    return expr.evalAsJson(context);
}

//------------------------------------------
// Private helper functions

void Expression::updateCaches(ContextScope const& context) const {
    updateStableValues(context);
    updateUnstableValues(context);
}

void Expression::updateStableValues(ContextScope const& context) const {
    auto updateContext = [&](auto const& currentContext, auto const& vdList) {
        for (auto const& vde : vdList) {
            vde->copyFromJson(currentContext);
        }
    };
    updateContext(context.self, linkedNumericValues.stable.self);
    updateContext(context.other, linkedNumericValues.stable.other);
    updateContext(context.global, linkedNumericValues.stable.global);
}

void Expression::updateUnstableValues(ContextScope const& context) const {
    auto updateContext = [&]<typename DataType>(DataType const& jsonScope, auto& vdList) {
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
    updateContext(context.self, linkedNumericValues.unstable.self);
    updateContext(context.other, linkedNumericValues.unstable.other);
    updateContext(context.global, linkedNumericValues.unstable.global);
    updateContext(Global::instance().getDocCache(), linkedNumericValues.unstable.resource);
    updateContext(emptyDoc(), linkedNumericValues.unstable.none);
    if (!linkedNumericValues.unstable.local.empty()) {
        Data::JsonScope merged;
        context.combineLocal(merged);
        updateContext(merged, linkedNumericValues.unstable.local);
    }
    if (!linkedNumericValues.unstable.full.empty()) {
        Data::JsonScope merged;
        context.combineAll(merged);
        updateContext(merged, linkedNumericValues.unstable.full);
    }
}

//------------------------------------------
// Recalculation helpers:

bool Expression::recalculateIsReturnableAsDouble() const {
    return components.size() == 1
           && components[0]->type == ExpressionComponent::Type::eval
           && components[0]->formatter.cast == Formatter::CastType::none; // no formatter allowed!
}

bool Expression::recalculateIsReturnableAsInt() const {
    return components.size() == 1
        && components[0]->type == ExpressionComponent::Type::eval
        && !components[0]->formatter.alignment
        && !components[0]->formatter.leadingZero
        && !components[0]->formatter.precision
        && components[0]->formatter.cast == Formatter::CastType::to_int;
}

bool Expression::recalculateIsReturnableAsString() const {
    return components.size() != 1 || components[0]->type != ExpressionComponent::Type::variable;
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

std::string Expression::eval(ContextScope const& context, std::size_t const recursionDepth) const {
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
        case ExpressionComponent::Type::variable:
            if (!component->handleComponentTypeVariable(token, context, recursionDepth)) {
                token = "null";
            }
            break;
            //------------------------------------------
        case ExpressionComponent::Type::eval:
            component->handleComponentTypeEval(token);
            break;
            //------------------------------------------
        case ExpressionComponent::Type::text:
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

int64_t Expression::evalAsInt(ContextScope const& context) const {
    updateCaches(context);
    return static_cast<int64_t>(te_eval(components[0]->expression));
}

bool Expression::evalAsBool(ContextScope const& context) const {
    double const result = evalAsDouble(context);
    return !Math::isZero(result);
}

Data::JSON Expression::evalAsJson(ContextScope const& context, std::size_t const recursionDepth) const {
    if (components.size() == 1 && components[0]->type != ExpressionComponent::Type::text) {
        if (components[0]->type == ExpressionComponent::Type::eval) {
            Data::JSON jsonResult;
            jsonResult.set<double>("", evalAsDouble(context));
            return jsonResult;
        }
        if (components[0]->type == ExpressionComponent::Type::variable) {
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

Data::JsonScope const& Expression::emptyDoc() {
    thread_local const Data::JsonScope emptyDoc;
    return emptyDoc;
}

}   // namespace Nebulite::Interaction::Logic
