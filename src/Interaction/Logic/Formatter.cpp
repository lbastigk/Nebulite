//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Interaction/Logic/Formatter.hpp"
#include "Nebulite/Utility/TypeConversion.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

Formatter Formatter::readFormatter(std::string_view const formatter) {
    // Check formatter. Integer cast should not include precision. Is ignored later on in casting but acceptable as input
    // Examples:
    // $i     : leadingZero = false , alignment = -1 , precision = -1
    // $04i   : leadingZero = true  , alignment =  4 , precision = -1
    // $03.5f : leadingZero = true  , alignment =  3 , precision =  5

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
        auto const dotPos = formatter.find('.');
        // Read alignment
        if (dotPos != 0) {
            auto end = formatter.substr(0, dotPos).find_last_of("0123456789");
            auto alignmentStr = formatter.substr(0, end + 1);
            fmt.alignment = Utility::TypeConversion::String::to<uint8_t>(alignmentStr);
        }
        // Read precision
        if (dotPos != std::string::npos) {
            auto end = formatter.substr(dotPos + 1).find_last_of("0123456789");
            auto precisionStr = formatter.substr(dotPos + 1, end + 1);
            fmt.precision = Utility::TypeConversion::String::to<uint8_t>(precisionStr);
        }
    }
    return fmt;
}

std::string Formatter::format(double const value) const {
    std::string token;
    if (cast == CastType::to_int) {
        token = std::to_string(static_cast<int>(value));
    } else {
        // to_double or none, both use double directly
        double newValue = value;

        // Apply rounding if precision is specified
        if (precision.has_value()) {
            double const multiplier = std::pow(10.0, precision.value());
            newValue = std::round(value * multiplier) / multiplier;
        }

        token = std::to_string(newValue);
    }

    // Precision formatting (after rounding)
    if (precision.has_value()) {
        if (auto const dotPos = token.find('.'); dotPos != std::string::npos) {
            assert(dotPos > 0);
            if (auto const currentPrecision = token.size() - dotPos - 1; currentPrecision < precision.value()) {
                // Add zeros to match the required precision
                token.append(precision.value() - currentPrecision, '0');
            } else if (currentPrecision > precision.value()) {
                // Truncate to the required precision (should be minimal after rounding)
                token.resize(dotPos + precision.value() + 1);
            }
        } else {
            // No decimal point, add one and pad with zeros
            token += '.';
            token.append(precision.value(), '0');
        }
    }

    // Adding padding
    if (alignment.has_value() && token.size() < alignment.value()) {
        std::string padding;
        for (std::size_t i = 0; i < alignment.value() - token.size(); i++) {
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

} // namespace Nebulite::Interaction::Logic
