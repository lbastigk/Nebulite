//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/SimpleValueError.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

Expression::Component::Component(Component&& other) noexcept
            : type(other.type), contextType(other.contextType),
              formatter(other.formatter), stringRepresentation(std::move(other.stringRepresentation)), key(std::move(other.key)),
              expression(other.expression) {
    other.expression = nullptr;
}

Expression::Component& Expression::Component::operator=(Component&& other) noexcept {
    if (this != &other) {
        te_free(expression);
        type = other.type;
        contextType = other.contextType;
        formatter = other.formatter;
        stringRepresentation = std::move(other.stringRepresentation);
        key = std::move(other.key);
        expression = other.expression;
        other.expression = nullptr;
    }
    return *this;
}

bool Expression::Component::handleComponentTypeVariable(std::string& token, ContextScope const& context, size_t const& recursionDepth) const {
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

    // Helper lambda to convert a value to a string representation
    auto getStringValue = []<typename DocumentType, typename KeyType>(DocumentType const& doc, KeyType const& k) -> std::string {
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
    };

    // Now, use the key to get the value from the correct document
    auto const scopedKey = Data::ScopedKey(evaluatedKey);
    switch (source) {
    case ContextDeriver::TargetType::self: // {self:<key><transformations>}
        token = getStringValue(context.self, scopedKey.view());
        break;
    case ContextDeriver::TargetType::other: // {other:<key><transformations>}
        token = getStringValue(context.other, scopedKey.view());
        break;
    case ContextDeriver::TargetType::local:
        {
            Data::JsonScope merged;
            Data::ScopedKey const self("self.");
            Data::ScopedKey const other("other.");
            merged.setSubDoc(self, context.self);
            merged.setSubDoc(other, context.other);
            token = getStringValue(merged, scopedKey.view());
        }
        break;
    case ContextDeriver::TargetType::global: // {global:<key><transformations>}
        token = getStringValue(context.global, scopedKey.view());
        break;
    case ContextDeriver::TargetType::full:
        {
            Data::JsonScope merged;
            Data::ScopedKey const self("self.");
            Data::ScopedKey const other("other.");
            Data::ScopedKey const global("global:");
            merged.setSubDoc(self, context.self);
            merged.setSubDoc(other, context.other);
            merged.setSubDoc(global, context.global);
            token = getStringValue(merged, scopedKey.view());
        }
        break;
    case ContextDeriver::TargetType::resource: // {<link><resource_key_or_transformations>}
        token = getStringValue(Global::instance().getDocCache(), evaluatedKey);
        break;
    case ContextDeriver::TargetType::none: // No document referenced, direct use of transformations: {|my|Transformations|come|directly|at|the|beginning}
        {
            // This requires an empty document that acts as a parsing mechanism for the transformations
            thread_local Data::JsonScope emptyDoc;
            token = getStringValue(emptyDoc, scopedKey.view());
        }
        break;
    default:
        std::unreachable();
    }
    return true;
}

bool Expression::Component::handleComponentTypeVariable(Data::JSON& token, ContextScope const& context, size_t const& recursionDepth) const {
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
    auto const scopedKey = Data::ScopedKey(evaluatedKey);
    switch (source) {
    case ContextDeriver::TargetType::self: // {self:<key><transformations>}
        token = context.self.getSubDoc(scopedKey.view());
        break;
    case ContextDeriver::TargetType::other: // {other:<key><transformations>}
        token = context.other.getSubDoc(scopedKey.view());
        break;
    case ContextDeriver::TargetType::local:
        {
            Data::JsonScope merged;
            context.combineLocal(merged);
            token = merged.getSubDoc(scopedKey.view());
        }
        break;
    case ContextDeriver::TargetType::global: // {global:<key><transformations>}
        token = context.global.getSubDoc(scopedKey.view());
        break;
    case ContextDeriver::TargetType::full:
        {
            Data::JsonScope merged;
            context.combineAll(merged);
            token = merged.getSubDoc(scopedKey.view());
        }
        break;
    case ContextDeriver::TargetType::resource: // {<link><resource_key_or_transformations>}
        token = Global::instance().getDocCache().getSubDoc(evaluatedKey);
        break;
    case ContextDeriver::TargetType::none: // No document referenced, direct use of transformations: {|my|Transformations|come|directly|at|the|beginning}
        {
            // This requires an empty document that acts as a parsing mechanism for the transformations
            thread_local Data::JsonScope emptyDoc;
            token = emptyDoc.getSubDoc(scopedKey.view());
        }
        break;
    default:
        std::unreachable();
    }
    return true;
}

void Expression::Component::handleComponentTypeEval(std::string& token) const {
    //------------------------------------------
    // Handle casting and precision together
    token = formatter.format(te_eval(expression));
}

std::expected<std::string, Expression::Component::KeyEvaluationInfo> Expression::Component::evaluateKey(ContextScope const& context, size_t const& recursionDepth) const {
    // See if the variable contains an inner expression
    if (stringRepresentation.find('$') != std::string::npos || stringRepresentation.find('{') != std::string::npos) {
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

std::optional<std::pair<std::string, ContextDeriver::TargetType>> Expression::Component::handleNesting(ContextScope const& context, size_t const& recursionDepth) const {
    auto s = evaluateKey(context, recursionDepth);

    // If max depth was reached, return false
    if (!s.has_value() && s.error() == KeyEvaluationInfo::maximumDepthReached) {
        return std::nullopt;
    }

    // If the evaluation changed anything, we must re-evaluate the context of the source
    std::string evaluatedKey = s.has_value() ? std::string(ContextDeriver::stripContext(s.value())) : key;
    ContextDeriver::TargetType source = s.has_value() ? ContextDeriver::getTypeFromString(s.value()) : contextType;
    return std::make_pair(evaluatedKey, source);
}

}  // namespace Nebulite::Interaction::Logic
