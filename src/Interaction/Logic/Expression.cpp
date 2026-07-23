//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint> // NOLINT
#include <iterator>
#include <memory>
#include <ranges>
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
#include "Nebulite/Data/MappedOrderedCacheList.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Interaction/Logic/ExpressionComponent.hpp"
#include "Nebulite/Interaction/Logic/LinkedNumericValue.hpp"
#include "Nebulite/Interaction/Logic/VariableNameGenerator.hpp"
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Math/ExpressionPrimitives.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/CompileTimeEvaluate.hpp"
#include "Nebulite/Utility/Coordination/RecursionAllocator.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

//------------------------------------------
// Helper

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

bool isTypeVariable(std::string_view const str) {
    return str.starts_with('{') && str != "{object}";
}

int countB(std::string_view const token) {
    return static_cast<int>(std::ranges::count(token, '{')) - static_cast<int>(std::ranges::count(token, '}'));
}

int countP(std::string_view const token) {
    return static_cast<int>(std::ranges::count(token, '(')) - static_cast<int>(std::ranges::count(token, ')'));
}

std::vector<std::string_view> getTokens(std::string_view const expr) {
    // First, we must split the expression into tokens
    // Split, keep delimiter(at start)
    // "abc$def$ghi" -> ["abc", "$def", "$ghi"]
    auto const tokensPhase1 = Utility::StringHandler::split(expr, '$', true);

    size_t currentStart = std::string_view::npos;
    size_t currentEnd = 0;

    // Combine tokens where the amount of `(` + `{` and `}` + `)` are not the same
    std::vector<std::string_view> tokensPhase2;
    for (auto const& token : tokensPhase1) {
        // TODO: needs some more logic so that, for example, the following is still split properly:
        //       "Value is: ($(1+1))" -> ["Value is: (", "$(1+1)", ")"]
        //       Currently, this would be split into a single token
        //       Sadly this isn't as straightforward as it seems...
        //       pCount/bCount needs more info to find end of an eval or var
        //       - local pCount 0 (check for every ')' -> pushback
        //       - local bCount 0 and not in eval -> pushback

        auto const diff = static_cast<size_t>(token.data() - expr.data());
        if (currentStart == std::string_view::npos) {
            currentStart = diff;
        }
        currentEnd = diff + token.size();

        // If the current token has no mismatch, push back
        if (countP(token) == 0 && countB(token) == 0) {
            tokensPhase2.emplace_back(
                expr.data() + currentStart,
                currentEnd - currentStart
            );
            currentStart = std::string_view::npos;
        }
    }
    // Push back the last token if it wasn't pushed back yet
    if (currentStart != std::string_view::npos) {
        tokensPhase2.emplace_back(
            expr.data() + currentStart,
            currentEnd - currentStart
        );
    }

    // Now we need to split on same depth
    std::vector<std::string_view> tokens;
    for (auto const& token : tokensPhase2) {
        // If the first token starts with '$', it means the string started with '$'
        // If not, the first token is text before the first '$'
        if (token.starts_with('$')) {
            // Remove everything until a '('
            // This part represents the '$' + formatter
            // Cannot be used, as splitOnSameDepth expects the first character to be the opening parenthesis
            auto const start = token.substr(0, token.find('('));
            auto const tokenWithoutStart = token.substr(start.length()); // Remove the leading '$' + formatter
            if (auto subTokens = Utility::StringHandler::splitOnSameDepthOf(tokenWithoutStart, Utility::StringHandler::Delimiter::parentheses); !subTokens.empty()) {
                // Add the removed part
                auto first = std::string_view(
                    start.data(),
                    start.size() + subTokens[0].size()
                );
                tokens.push_back(first);
                // Add the rest
                std::ranges::copy(
                    subTokens | std::views::drop(1), // Remove the first token, as we already added it
                    std::back_inserter(tokens)
                );
            }
        } else {
            // If it doesn't start with a '$', it's a text token / potentially with variables inside
            // Just add the text token
            tokens.emplace_back(token);
        }
    }
    return tokens;
}

} // anonymous namespace

//------------------------------------------
// General

Expression::~Expression() {
    // reset all data
    reset();

    // Remove variables
    te_variables.clear();

    // Clear all expressions
    for (auto& component : components) {
        if (component.expression != nullptr) {
            te_free(component.expression);
            component.expression = nullptr;
        }
    }
}

Expression::Expression(std::string_view const expr){
    evaluationInfo = {
        .returnableAsDouble = false,
        .returnableAsString = false,
        .alwaysTrue = false
    };
    reset();
    parse(expr);
    cacheId.self = Data::MappedOrderedCacheList::generateUniqueId(std::string("self:") + std::string(expr));
    cacheId.other = Data::MappedOrderedCacheList::generateUniqueId(std::string("other:") + std::string(expr));
    cacheId.global = Data::MappedOrderedCacheList::generateUniqueId(std::string("global:") + std::string(expr));
}

//------------------------------------------
// Actual evaluation functions

std::string Expression::eval(ContextScope const& context, std::size_t const recursionDepth) const {
    //------------------------------------------
    // Update caches so that tinyexpr has the correct references
    updateCaches(context);

    //------------------------------------------
    // Resource allocators

    // Avoid reallocation of string for each component, as this is a hot path
    thread_local Utility::Coordination::RecursionAllocator<std::string, void, allocatedRecursionDepth> tokenWrapper;
    thread_local Utility::Coordination::RecursionAllocator<std::string, std::string, allocatedRecursionDepth> resultWrapper;

    //------------------------------------------
    // Evaluate expression
    // Concatenate results of each component
    return resultWrapper.use(
        [](auto& result) noexcept {
            result.resize(0);
        },
        [&](auto& result) {
            for (auto const& component : components) {
                tokenWrapper.use(
                    [](auto& token) noexcept {
                        token.resize(0);
                    },
                    [&](auto& token) {
                        switch (component.type) {
                        //------------------------------------------
                        case ExpressionComponent::Type::variable:
                            if (!component.handleComponentTypeVariable(token, context, recursionDepth)) {
                                token = "null";
                            }
                            break;
                        //------------------------------------------
                        case ExpressionComponent::Type::eval:
                            component.handleComponentTypeEval(token);
                            break;
                        case ExpressionComponent::Type::text:
                            token = component.stringRepresentation;
                            break;
                        default:
                            break;
                        }
                        result += token;
                    }
                );
            }
            return result;
        }
    );
}

double Expression::evalAsDouble(ContextScope const& context) const {
    updateCaches(context);
    return te_eval(components[0].expression);
}

int64_t Expression::evalAsInt(ContextScope const& context) const {
    updateCaches(context);
    return static_cast<int64_t>(te_eval(components[0].expression));
}

bool Expression::evalAsBool(ContextScope const& context) const {
    double const result = evalAsDouble(context);
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    return !Math::isZero(result);
}

Data::JSON Expression::evalAsJson(ContextScope const& context, std::size_t const recursionDepth) const {
    if (components.size() == 1 && components[0].type != ExpressionComponent::Type::text) {
        if (components[0].type == ExpressionComponent::Type::eval) {
            Data::JSON jsonResult;
            jsonResult.set<double>("", evalAsDouble(context));
            return jsonResult;
        }
        if (components[0].type == ExpressionComponent::Type::variable) {
            Data::JSON jsonResult;
            components[0].handleComponentTypeVariable(jsonResult, context, recursionDepth);
            return jsonResult;
        }
    }
    Data::JSON jsonResult;
    jsonResult.set<std::string>("", eval(context, recursionDepth));
    return jsonResult;
}

//------------------------------------------
// Static functions for one-time evaluation

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
// Getter

std::string const& Expression::getFullExpression() const noexcept {
    return fullExpression;
}

//------------------------------------------
// Evaluation info

bool Expression::recalculateIsReturnableAsDouble() const {
    return components.size() == 1
           && components[0].type == ExpressionComponent::Type::eval
           && components[0].formatter.cast == Formatter::CastType::none; // no formatter allowed!
}

bool Expression::recalculateIsReturnableAsInt() const {
    return components.size() == 1
        && components[0].type == ExpressionComponent::Type::eval
        && components[0].formatter.cast == Formatter::CastType::to_int
        && !components[0].formatter.alignment
        && !components[0].formatter.leadingZero
        && !components[0].formatter.precision;
}

bool Expression::recalculateIsReturnableAsString() const {
    return components.size() != 1 || components[0].type != ExpressionComponent::Type::variable;
}

bool Expression::recalculateIsAlwaysTrue() const {
    if (fullExpression.starts_with("$(") && fullExpression.ends_with(")")){
        std::string const innerExpression = fullExpression.substr(2, fullExpression.size() - 3);
        return Utility::StringHandler::isNumber(innerExpression) && !Math::isEqual(0.0,std::stod(innerExpression));
    }
    return Utility::StringHandler::isNumber(fullExpression) && !Math::isEqual(0.0,std::stod(fullExpression));
}

//------------------------------------------
// Caching

void Expression::LinkedNumericValueLists::registerLnv(ContextDeriver::TargetType const contextType, std::string_view const key, double& v){
    auto vd = std::make_unique<LinkedNumericValue>(key, v);
    switch (contextType) {
    case ContextDeriver::TargetType::self:
        if (isAvailableAsDoublePtr(key)) {
            stable.self.push_back(std::move(vd));
        } else {
            unstable.self.push_back(std::move(vd));
        }
        break;
    case ContextDeriver::TargetType::other:
        // Type other is always non-remanent, as other document reference can change
        // However, we need to distinguish between stable and unstable double pointers
        // Meaning the ones we can get from an ordered list, and the ones we need to resolve each time
        // (e.g. with multi-resolve or transformations)
        if (isAvailableAsDoublePtr(key)) {
            stable.other.push_back(std::move(vd));
        } else {
            unstable.other.push_back(std::move(vd));
        }
        break;
    case ContextDeriver::TargetType::local:
        unstable.local.push_back(std::move(vd));
        break;
    case ContextDeriver::TargetType::global:
        if (isAvailableAsDoublePtr(key)) {
            stable.global.push_back(std::move(vd));
        } else {
            unstable.global.push_back(std::move(vd));
        }
        break;
    case ContextDeriver::TargetType::full:
        unstable.full.push_back(std::move(vd));
        break;
    case ContextDeriver::TargetType::resource:
        unstable.resource.push_back(std::move(vd));
        break;
    case ContextDeriver::TargetType::none:
        // Use an empty document
        unstable.none.push_back(std::move(vd));
        break;
    default:
        // Should not happen
        Global::capture().error.println(__FUNCTION__, ": Tried to register variable with no known context!");
        std::unreachable();
    }
}

void Expression::addTeVariable(ContextDeriver::TargetType const contextType, std::string_view const k, ShortName const& teName, double& v) {
    // Register cache based on context
    linkedNumericValues.registerLnv(contextType, k, v);

    // Push back into variable components
    te_variables.push_back({
        .name=teName.data.data(), // NOLINT
        .address=&v,
        .type=TE_VARIABLE,
        .context=nullptr
    });
}

void Expression::updateCaches(ContextScope const& context) const {
    updateStableValues(context);
    updateUnstableValues(context);
}

void Expression::updateStableValues(ContextScope const& context) const {
    auto updateFromCacheList = [](auto& jsonScope, auto& vdList, uint64_t id) {
        if (vdList.empty()) {
            return;
        }
        auto* v = jsonScope.ensureOrderedCacheList(
            id,
            vdList | std::views::transform([](auto const& vde) { return vde->getScopedKey(); })
        );
        for (auto [i, vde] : vdList | Utility::Ranges::enumerate) {
            vde->setDirect(*v[i]);
        }
    };
    updateFromCacheList(context.self, linkedNumericValues.stable.self, cacheId.self);
    updateFromCacheList(context.other, linkedNumericValues.stable.other, cacheId.other);
    updateFromCacheList(context.global, linkedNumericValues.stable.global, cacheId.global);
}

void Expression::updateUnstableValues(ContextScope const& context) const {
    auto updateFromJSON = [&]<typename DataType>(DataType const& jsonScope, auto& vdList) {
        for (auto& vde : vdList) {
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
    updateFromJSON(context.self, linkedNumericValues.unstable.self);
    updateFromJSON(context.other, linkedNumericValues.unstable.other);
    updateFromJSON(context.global, linkedNumericValues.unstable.global);
    updateFromJSON(Global::instance().getDocCache(), linkedNumericValues.unstable.resource);
    updateFromJSON(emptyDoc(), linkedNumericValues.unstable.none);

    // Merge docs for context-married values using a temporary JSON
    if (!linkedNumericValues.unstable.local.empty()) {
        Data::JsonScope merged;
        context.combineLocal(merged);
        updateFromJSON(merged, linkedNumericValues.unstable.local);
    }
    if (!linkedNumericValues.unstable.full.empty()) {
        Data::JsonScope merged;
        context.combineAll(merged);
        updateFromJSON(merged, linkedNumericValues.unstable.full);
    }
}

//------------------------------------------
// Core Helper functions

Data::JsonScope const& Expression::emptyDoc() {
    thread_local const Data::JsonScope emptyDoc;
    return emptyDoc;
}

void Expression::reset() {
    // Clear existing data
    components.clear();
    te_variables.clear();
    fullExpression.clear();
    components.clear();

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

//------------------------------------------
// Parsing and compiling

void Expression::parse(std::string_view const expr) {
    reset();
    fullExpression = expr;
    parseIntoComponents();
    for (auto& component : components) {
        compileIfExpression(component);
    }
    evaluationInfo.returnableAsDouble = recalculateIsReturnableAsDouble();
    evaluationInfo.returnableAsInt = recalculateIsReturnableAsInt();
    evaluationInfo.returnableAsString = recalculateIsReturnableAsString();
    evaluationInfo.alwaysTrue = recalculateIsAlwaysTrue();
}

void Expression::compileIfExpression(ExpressionComponent& component) const {
    if (component.type == ExpressionComponent::Type::eval) {
        // Compile the expression using TinyExpr
        int error{};
        component.expression = te_compile(component.stringRepresentation.c_str(), te_variables.data(), static_cast<int>(te_variables.size()), &error);
        if (error) {
            printCompileError(component, error);

            // Resetting expression to nan, as explained in error print:
            // using nan directly is not supported.
            // 0/0 directly yields -nan, so we use abs(0/0)
            te_free(component.expression);
            component.expression = te_compile("abs(0/0)", te_variables.data(), static_cast<int>(te_variables.size()), &error);
        }
    }
}

void Expression::parseIntoComponents() {
    VariableNameGenerator variableNameGenerator;
    std::vector<LateRegistration> lateRegistrations;
    for (auto const& token : getTokens(fullExpression)) {
        if (token.starts_with('$')) {
            parseTokenTypeEval(token, lateRegistrations, variableNameGenerator);
        } else {
            // Current token is Text
            // Perhaps mixed with variables...
            for (auto const& subToken : Utility::StringHandler::splitOnSameDepthOf(token, Utility::StringHandler::Delimiter::brace)) {
                if (isTypeVariable(subToken)) {
                    parseTokenTypeVariable(subToken);
                }
                else {
                    parseTokenTypeText(subToken);
                }
            }
        }
    }
    cache.values.resize(lateRegistrations.size());
    cache.teNames.resize(lateRegistrations.size());
    for (auto [i, lr] : lateRegistrations | Utility::Ranges::enumerate) {
        cache.teNames[i] = lr.teName;
        addTeVariable(lr.contextType, lr.key, cache.teNames[i], cache.values[i]);
    }
}

void Expression::parseTokenTypeEval(std::string_view const token, std::vector<LateRegistration>& lateRegistrations, VariableNameGenerator& varNameGen) {
    // Extract formatter and expression
    std::size_t const exprStart = token.find('('); // Opening parenthesis of the expression
    auto const formatter = token.substr(1, exprStart - 1); // Remove leading $
    auto const expression = token.substr(exprStart);

    // Write basic component data
    ExpressionComponent currentComponent;
    currentComponent.formatter = Formatter::readFormatter(formatter);
    currentComponent.type = ExpressionComponent::Type::eval;
    currentComponent.contextType = ContextDeriver::TargetType::none; // None, since this is an eval expression
    currentComponent.key = ""; // No key for eval expressions

    // Register internal variables
    // And build equivalent expression using new variable names
    // New string length is hard to estimate; every shortened variable is  ~1 character in size compared to the arbitrary length of the original variable name.
    currentComponent.stringRepresentation.reserve(expression.length() / 4);
    for (auto const& subToken : Utility::StringHandler::splitOnSameDepthOf(expression, Utility::StringHandler::Delimiter::brace)) {
        if (subToken.starts_with('{')) {
            auto const te_name = varNameGen.getUniqueName(subToken);
            auto key = ContextDeriver::stripContext(subToken.substr(1, subToken.length() - 2));
            auto const contextType = ContextDeriver::getTypeFromString(subToken.substr(1, subToken.length() - 2));
            registerVariable(te_name, key, contextType, lateRegistrations);
            currentComponent.stringRepresentation += te_name;
        } else {
            currentComponent.stringRepresentation += subToken;
        }
    }

    // Add to components
    components.push_back(std::move(currentComponent));
}

void Expression::parseTokenTypeVariable(std::string_view const token) {
    ExpressionComponent currentComponent;

    // 1.) remove {}
    // We keep all other potential {} inside the variable name for later MultiResolve
    auto inner = token.substr(1, token.length() - 2);

    // 2.) Check if inner starts with a number followed by an exclamation mark, if so, this is an evaluation wait specifier,
    // and we set the evaluation wait of the component accordingly, and remove the specifier from the inner string
    if (auto const exclamationMarkPosition = inner.find('!'); exclamationMarkPosition != std::string::npos) {
        if (auto const beforeExclamation = inner.substr(0, exclamationMarkPosition); Utility::StringHandler::isNumber(beforeExclamation)) {
            if (!beforeExclamation.empty()) {
                currentComponent.evaluationWait = std::stoul(std::string(beforeExclamation));
            }
            inner = inner.substr(exclamationMarkPosition + 1);
        }
    }

    // 3.) determine context
    currentComponent.type = ExpressionComponent::Type::variable;
    currentComponent.stringRepresentation = inner;
    currentComponent.contextType = ContextDeriver::getTypeFromString(inner);
    currentComponent.key = ContextDeriver::stripContext(inner);

    components.push_back(std::move(currentComponent));
}

void Expression::parseTokenTypeText(std::string_view const token) {
    ExpressionComponent currentComponent;
    currentComponent.type = ExpressionComponent::Type::text;
    currentComponent.stringRepresentation = token;
    currentComponent.contextType = ContextDeriver::TargetType::none;
    currentComponent.key = ""; // No key for text expressions
    components.push_back(std::move(currentComponent));
}

void Expression::printCompileError(ExpressionComponent const& component, int const error) const {
    std::string offendingChar;
    if (error <= 0 || static_cast<size_t>(error) > component.stringRepresentation.size()) {
        offendingChar = "N/A (error position out of bounds)";
    } else {
        offendingChar = std::string(1, component.stringRepresentation[static_cast<size_t>(error) - 1]);
    }
    std::stringstream ss;
    ss << "-----------------------------------------------------------------" << "\n";
    ss << "Error compiling expression: '" << component.stringRepresentation << "' At position: " << std::to_string(error) << ", offending character: " << offendingChar << "\n";
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

void Expression::registerVariable(std::string_view const te_name, std::string_view const key, ContextDeriver::TargetType const contextType, std::vector<LateRegistration>& lateRegistrations) {
    // Check if variable exists in variables vector:
    bool const found = std::ranges::any_of(te_variables, [&](auto const& te_var) {
        return te_var.name == te_name;
    });

    if (!found) {
        lateRegistrations.emplace_back(LateRegistration{
            .key = std::string(key),
            .teName = ShortName(te_name),
            .contextType = contextType,
        });
    }
}

}   // namespace Nebulite::Interaction::Logic
