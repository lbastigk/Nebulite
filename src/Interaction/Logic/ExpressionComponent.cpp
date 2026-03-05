#include "Nebulite.hpp"
#include "Data/Document/JsonScopeBase.hpp"
#include "Data/Document/SimpleValueError.hpp"
#include "Interaction/Logic/Expression.hpp"

namespace Nebulite::Interaction::Logic {

Expression::Component::Component(Component&& other) noexcept
            : type(other.type), contextType(other.contextType), cast(other.cast),
              formatter(other.formatter), str(std::move(other.str)), key(std::move(other.key)),
              expression(other.expression) {
    other.expression = nullptr;
}

Expression::Component& Expression::Component::operator=(Component&& other) noexcept {
    if (this != &other) {
        te_free(expression);
        type = other.type;
        contextType = other.contextType;
        cast = other.cast;
        formatter = other.formatter;
        str = std::move(other.str);
        key = std::move(other.key);
        expression = other.expression;
        other.expression = nullptr;
    }
    return *this;
}

bool Expression::Component::handleComponentTypeVariable(std::string& token, ContextScopeBase const& context, size_t const& recursionDepth) const {
    auto s = evaluateKey(context, key, contextType, recursionDepth);
    if (!s) {return false;}
    auto [strippedKey, destination] = s.value();

    // Helper lambda to convert a value to a string representation
    auto getStringValue = []<typename DocumentType, typename KeyType>(DocumentType const& doc, KeyType const& k) -> std::string {
        std::expected<std::string, Data::SimpleValueRetrievalError> value = doc.template get<std::string>(k);
        if (value.has_value()) {
            return value.value();
        }
        switch (value.error()) {
            case Data::SimpleValueRetrievalError::CONVERSION_FAILURE:
                return "<TO_STRING_CONVERSION_FAILURE>";
            case Data::SimpleValueRetrievalError::TRANSFORMATION_FAILURE:
                return "<TRANSFORMATION_FAILURE>";
            case Data::SimpleValueRetrievalError::MALFORMED_KEY:
                return "<MALFORMED_KEY>";
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
    auto const scopedKey = Data::ScopedKey(strippedKey);
    switch (destination) {
    case ContextType::self: // {self.<key><transformations>}
        token = getStringValue(context.self, scopedKey.view());
        break;
    case ContextType::other: // {other.<key><transformations>}
        token = getStringValue(context.other, scopedKey.view());
        break;
    case ContextType::global: // {global.<key><transformations>}
        token = getStringValue(context.global, scopedKey.view());
        break;
    case ContextType::resource: // {<link><resource_key_or_transformations>}
        token = getStringValue(Global::instance().getDocCache(), strippedKey);
        break;
    case ContextType::None: // No document referenced, direct use of transformations: {|my|Transformations|come|directly|at|the|beginning}
    {
        // This requires an empty document that acts as a parsing mechanism for the transformations
        thread_local Data::JsonScopeBase emptyDoc;
        token = getStringValue(emptyDoc, scopedKey.view());
    }
        break;
    default:
        std::unreachable();
    }
    return true;
}

bool Expression::Component::handleComponentTypeVariable(Data::JSON& token, ContextScopeBase const& context, size_t const& recursionDepth) const {
    auto s = evaluateKey(context, key, contextType, recursionDepth);
    if (!s) {return false;}
    auto [strippedKey, destination] = s.value();

    // Now, use the key to get the value from the correct document
    auto const scopedKey = Data::ScopedKey(strippedKey);
    switch (destination) {
    case ContextType::self: // {self.<key><transformations>}
        token = context.self.getSubDoc(scopedKey.view());
        break;
    case ContextType::other: // {other.<key><transformations>}
        token = context.other.getSubDoc(scopedKey.view());
        break;
    case ContextType::global: // {global.<key><transformations>}
        token = context.global.getSubDoc(scopedKey.view());
        break;
    case ContextType::resource: // {<link><resource_key_or_transformations>}
        token = Global::instance().getDocCache().getSubDoc(strippedKey);
        break;
    case ContextType::None: // No document referenced, direct use of transformations: {|my|Transformations|come|directly|at|the|beginning}
        {
            // This requires an empty document that acts as a parsing mechanism for the transformations
            thread_local Data::JsonScopeBase emptyDoc;
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
    if (cast == CastType::to_int) {
        token = std::to_string(static_cast<int>(te_eval(expression)));
    } else {
        // to_double or none, both use double directly
        double value = te_eval(expression);

        // Apply rounding if precision is specified
        if (formatter.precision != -1) {
            double const multiplier = std::pow(10.0, formatter.precision);
            value = std::round(value * multiplier) / multiplier;
        }

        token = std::to_string(value);
    }

    // Precision formatting (after rounding)
    if (formatter.precision != -1) {
        if (size_t const dotPos = token.find('.'); dotPos != std::string::npos) {
            if (size_t const currentPrecision = token.size() - dotPos - 1; currentPrecision < static_cast<size_t>(formatter.precision)) {
                // Add zeros to match the required precision
                token.append(static_cast<size_t>(formatter.precision) - currentPrecision, '0');
            } else if (currentPrecision > static_cast<size_t>(formatter.precision)) {
                // Truncate to the required precision (should be minimal after rounding)
                token.resize(dotPos + static_cast<size_t>(formatter.precision) + 1);
            }
        } else {
            // No decimal point, add one and pad with zeros
            token += '.';
            token.append(static_cast<size_t>(formatter.precision), '0');
        }
    }

    // Adding padding
    if (formatter.alignment > 0 && token.size() < static_cast<size_t>(formatter.alignment)) {
        // Cast to int, as alignment may be negative (-1 signals no alignment)
        int const size = static_cast<int>(token.size());
        std::string padding;
        for (int i = 0; i < formatter.alignment - size; i++) {
            if (formatter.leadingZero) {
                padding += "0";
            } else {
                padding += " ";
            }
        }
        token.insert(0, padding);
    }
}

std::optional<std::pair<std::string, Expression::Component::ContextType>> Expression::Component::evaluateKey(ContextScopeBase const& context, std::string const& initialKey, ContextType const& initialDestination, size_t const& recursionDepth) const {
    std::string strippedKey = initialKey;
    ContextType destination = initialDestination;

    // See if the variable contains an inner expression
    if (str.find('$') != std::string::npos || str.find('{') != std::string::npos) {
        if (recursionDepth == 0) {
            Error::println("Error: Maximum recursion depth reached when evaluating variable: ", key);
            return std::nullopt;
        }
        // Create a temporary expression to evaluate the inner expression
        Expression const tempExpr(str);
        strippedKey = tempExpr.eval(context, recursionDepth - 1);

        // Redetermine context and strip it from key
        destination = getContextType(strippedKey);
        strippedKey = stripContext(strippedKey);
    }
    return std::make_pair(strippedKey, destination);
}

}  // namespace Nebulite::Interaction::Logic
