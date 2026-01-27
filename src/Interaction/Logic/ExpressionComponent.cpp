#include "Nebulite.hpp"
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

bool Expression::Component::handleComponentTypeVariable(std::string& token, Data::JsonScopeBase& selfScope, Core::JsonScope& otherScope, uint16_t const& maximumRecursionDepth) const {
    std::string strippedKey = key;
    ContextType context = contextType;

    // See if the variable contains an inner expression
    if (str.find('$') != std::string::npos || str.find('{') != std::string::npos) {
        if (maximumRecursionDepth == 0) {
            Error::println("Error: Maximum recursion depth reached when evaluating variable: ", key);
            return false;
        }
        // Create a temporary expression to evaluate the inner expression
        Expression const tempExpr(str, selfScope);
        strippedKey = tempExpr.eval(otherScope, maximumRecursionDepth - 1);

        // Redetermine context and strip it from key
        context = getContext(strippedKey);
        strippedKey = stripContext(strippedKey);
    }

    // Now, use the key to get the value from the correct document
    auto const scopedKey = Data::ScopedKey(strippedKey);
    switch (context) {
    case ContextType::self: // {self.<key><transformations>}
        token = selfScope.get<std::string>(scopedKey.view(), "null");
        break;
    case ContextType::other: // {other.<key><transformations>}
        token = otherScope.get<std::string>(scopedKey.view(), "null");
        break;
    case ContextType::global: // {global.<key><transformations>}
        token = Global::instance().domainScope.get<std::string>(scopedKey.view(), "null");
        break;
    case ContextType::resource: // {<link><resource_key_or_transformations>}
        token = Global::instance().getDocCache().get<std::string>(strippedKey, "null");
        break;
    case ContextType::None: // No document referenced, direct use of transformations: {|my|Transformations|come|directly|at|the|beginning}
        {
            // This requires an empty document that acts as a parsing mechanism for the transformations
            thread_local Core::JsonScope emptyDoc;
            token = emptyDoc.get<std::string>(scopedKey.view(), "null");
        }
        break;
    default:
        Error::println("Error: Unknown context in expression: ", strippedKey);
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

}  // namespace Nebulite::Interaction::Logic