//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

// External
#include <tinyexpr.h>

// Nebulite
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Interaction/Logic/ExpressionComponent.hpp"
#include "Nebulite/Interaction/Logic/VariableNameGenerator.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

ExpressionComponent ExpressionComponent::parseEval(std::string_view token, VariableNameGenerator& varNameGen, std::function<void(std::string_view, std::string_view, ContextDeriver::TargetType)> const& registerVariableCallback) {
    // Extract formatter and expression
    std::size_t const exprStart = token.find('('); // Opening parenthesis of the expression
    auto const formatter = token.substr(1, exprStart - 1); // Remove leading $
    auto const expression = token.substr(exprStart);

    // Write basic component data
    ExpressionComponent currentComponent;
    currentComponent.formatter = Formatter::readFormatter(formatter);
    currentComponent.type = Type::eval;
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
            registerVariableCallback(te_name, key, contextType);
            currentComponent.stringRepresentation += te_name;
        } else {
            currentComponent.stringRepresentation += subToken;
        }
    }
    return currentComponent;
}

ExpressionComponent ExpressionComponent::parseVariable(std::string_view token){
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
    currentComponent.type = Type::variable;
    currentComponent.stringRepresentation = inner;
    currentComponent.contextType = ContextDeriver::getTypeFromString(inner);
    currentComponent.key = ContextDeriver::stripContext(inner);
    return currentComponent;
}

ExpressionComponent ExpressionComponent::parseText(std::string_view token){
    ExpressionComponent currentComponent;
    currentComponent.type = Type::text;
    currentComponent.stringRepresentation = token;
    currentComponent.contextType = ContextDeriver::TargetType::none;
    currentComponent.key = ""; // No key for text expressions
    return currentComponent;
}

namespace {

template<typename DocumentType, typename KeyType>
std::string getStringValue(DocumentType const& doc, KeyType const& k) {
    std::expected<std::string, Data::SimpleValueRetrievalError> value = doc.template get<std::string>(k);
    if (value.has_value()) {
        return value.value();
    }
    switch (value.error()) {
    case Data::SimpleValueRetrievalError::CONVERSION_FAILURE:
        return "[TO_STRING_CONVERSION_FAILURE]";
    case Data::SimpleValueRetrievalError::TRANSFORMATION_FAILURE:
        return "[TRANSFORMATION_FAILURE]";
    case Data::SimpleValueRetrievalError::MALFORMED_KEY:
        return "[MALFORMED_KEY]";
    case Data::SimpleValueRetrievalError::IS_NULL:
        return "null";
    case Data::SimpleValueRetrievalError::IS_ARRAY:
        return "[array]";
    case Data::SimpleValueRetrievalError::IS_OBJECT:
        return "{object}";
    default:
        std::unreachable();
    }
}

void setToken(std::string& token, std::string const& evaluatedKey, ContextScope const& context, ContextDeriver::TargetType const source) {
    auto const scopedKey = Data::ScopedKey(evaluatedKey);
    auto keyView = scopedKey.view();
    switch (source) {
    case ContextDeriver::TargetType::self: // {self:<key><transformations>}
        token = getStringValue(context.self, scopedKey.view());
        break;
    case ContextDeriver::TargetType::other: // {other:<key><transformations>}
        token = getStringValue(context.other, scopedKey.view());
        break;
    case ContextDeriver::TargetType::local:{
        Data::JsonScope merged;
        Data::ScopedKey const self("self.");
        Data::ScopedKey const other("other.");
        merged.setSubDoc(self, context.self);
        merged.setSubDoc(other, context.other);
        token = getStringValue(merged, scopedKey.view());
        break;
    }
    case ContextDeriver::TargetType::global: // {global:<key><transformations>}
        token = getStringValue(context.global, keyView);
        break;
    case ContextDeriver::TargetType::full: {
        Data::JsonScope merged;
        Data::ScopedKey const self("self.");
        Data::ScopedKey const other("other.");
        Data::ScopedKey const global("global:");
        merged.setSubDoc(self, context.self);
        merged.setSubDoc(other, context.other);
        merged.setSubDoc(global, context.global);
        token = getStringValue(merged, scopedKey.view());
        break;
    }
    case ContextDeriver::TargetType::resource: // {<link><resource_key_or_transformations>}
        token = getStringValue(Global::instance().getDocCache(), evaluatedKey);
        break;
    case ContextDeriver::TargetType::none: {
        // No document referenced, direct use of transformations: {|my|Transformations|come|directly|at|the|beginning}
        // This requires an empty document that acts as a parsing mechanism for the transformations
        thread_local const Data::JsonScope emptyDoc;
        token = getStringValue(emptyDoc, scopedKey.view());
        break;
    }
    default:
        std::unreachable();
    }
}

void setToken(Data::JSON& token, std::string const& evaluatedKey, ContextScope const& context, ContextDeriver::TargetType const source) {
    auto const scopedKey = Data::ScopedKey(evaluatedKey);
    switch (source) {
    case ContextDeriver::TargetType::self: // {self:<key><transformations>}
        token = context.self.getSubDoc(scopedKey.view());
        break;
    case ContextDeriver::TargetType::other: // {other:<key><transformations>}
        token = context.other.getSubDoc(scopedKey.view());
        break;
    case ContextDeriver::TargetType::local: {
        Data::JsonScope merged;
        context.combineLocal(merged);
        token = merged.getSubDoc(scopedKey.view());
        break;
    }
    case ContextDeriver::TargetType::global: // {global:<key><transformations>}
        token = context.global.getSubDoc(scopedKey.view());
        break;
    case ContextDeriver::TargetType::full: {
        Data::JsonScope merged;
        context.combineAll(merged);
        token = merged.getSubDoc(scopedKey.view());
        break;
    }
    case ContextDeriver::TargetType::resource: // {<link><resource_key_or_transformations>}
        token = Global::instance().getDocCache().getSubDoc(evaluatedKey);
        break;
    case ContextDeriver::TargetType::none: {
        // No document referenced, direct use of transformations: {|my|Transformations|come|directly|at|the|beginning}
        // This requires an empty document that acts as a parsing mechanism for the transformations
        thread_local const Data::JsonScope emptyDoc;
        token = emptyDoc.getSubDoc(scopedKey.view());
        break;
    }
    default:
        std::unreachable();
    }
}

} // namespace

void ExpressionComponent::eval(std::string& result, ContextScope const& context, std::size_t recursionDepth) const {
    switch (type) {
        //------------------------------------------
    case Type::variable:
        if (!evalComponentTypeVariable(result, context, recursionDepth)) {
            result = "null";
        }
        break;
        //------------------------------------------
    case Type::eval:
        evalComponentTypeEval(result);
        break;
    case Type::text:
        result = stringRepresentation;
        break;
    default:
        break;
    }
}

Data::JSON ExpressionComponent::evalAsJson(ContextScope const& context, std::size_t recursionDepth) const {
    Data::JSON jsonResult;
    if (type == Type::eval) {
        if (formatter.cast == Formatter::CastType::none) {
            jsonResult.set<double>("", te_eval(expression));
        }
        else {
            std::string result;
            evalComponentTypeEval(result);
            jsonResult.set<std::string>("", result);
        }
    }
    if (type == Type::variable) {
        evalComponentTypeVariable(jsonResult, context, recursionDepth);
    }
    if (type == Type::text) {
        jsonResult.set<std::string>("", stringRepresentation);
    }
    return jsonResult;
}

bool ExpressionComponent::evalComponentTypeVariable(std::string& token, ContextScope const& context, std::size_t const recursionDepth) const {
    // Do not evaluate if wait is active
    if (evaluationWait > 1) {
        token = "{" + std::to_string(evaluationWait - 1) + "!" + stringRepresentation + "}";
        return true;
    }
    if (evaluationWait == 1) {
        token = "{" + stringRepresentation + "}";
        return true;
    }

    // Evaluate inner variables/expressions if necessary, for example in {global:{self:info.requiredKey}}
    auto nestedEvalResult = handleNesting(context, recursionDepth);
    if (!nestedEvalResult.has_value()) {
        return false;
    }
    auto const& [evaluatedKey, source] = nestedEvalResult.value();

    // Now, use the key to get the value from the correct document
    setToken(token, evaluatedKey, context, source);
    return true;
}

bool ExpressionComponent::evalComponentTypeVariable(Data::JSON& token, ContextScope const& context, std::size_t const recursionDepth) const {
    // Do not evaluate if wait is active
    if (evaluationWait > 1) {
        token.set<std::string>("", "{" + std::to_string(evaluationWait - 1) + "!" + stringRepresentation + "}");
        return true;
    }
    if (evaluationWait == 1) {
        token.set<std::string>("", "{" + stringRepresentation + "}");
        return true;
    }

    // Evaluate inner variables/expressions if necessary, for example in {global:{self:info.requiredKey}}
    auto nestedEvalResult = handleNesting(context, recursionDepth);
    if (!nestedEvalResult.has_value()) {
        return false;
    }
    auto const& [evaluatedKey, source] = nestedEvalResult.value();

    // Now, use the key to get the value from the correct document
    setToken(token, evaluatedKey, context, source);
    return true;
}

void ExpressionComponent::evalComponentTypeEval(std::string& token) const {
    token = formatter.format(te_eval(expression));
}

std::expected<std::string, ExpressionComponent::KeyEvaluationInfo> ExpressionComponent::evaluateKey(ContextScope const& context, std::size_t const recursionDepth) const {
    // See if the variable contains an inner expression
    if (stringRepresentation.contains('$') || stringRepresentation.contains('{')) {
        if (recursionDepth == 0) {
            Global::capture().error.println("Error: Maximum recursion depth reached when evaluating variable: ", key);
            return std::unexpected(KeyEvaluationInfo::maximumDepthReached);
        }
        // Create a temporary expression to evaluate the inner expression
        Expression const tempExpr(stringRepresentation);
        return tempExpr.eval(context, recursionDepth - 1);
    }
    return std::unexpected(KeyEvaluationInfo::noNesting);
}

std::optional<std::pair<std::string, ContextDeriver::TargetType>> ExpressionComponent::handleNesting(ContextScope const& context, std::size_t const recursionDepth) const {
    auto s = evaluateKey(context, recursionDepth);

    // If max depth was reached, return false
    if (!s.has_value() && s.error() == KeyEvaluationInfo::maximumDepthReached) {
        return std::nullopt;
    }

    // If the evaluation changed anything, we must re-evaluate the context of the source
    std::string const evaluatedKey = s.has_value() ? ContextDeriver::stripContext(s.value()) : key;
    ContextDeriver::TargetType const source = s.has_value() ? ContextDeriver::getTypeFromString(s.value()) : contextType;
    return std::make_pair(evaluatedKey, source);
}

}  // namespace Nebulite::Interaction::Logic
