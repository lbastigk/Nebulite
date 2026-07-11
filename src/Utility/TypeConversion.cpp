//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <limits>
#include <optional>
#include <string>

// Nebulite
#include "Nebulite/Utility/TypeConversion.hpp"

//------------------------------------------
namespace Nebulite::Utility {

// Bool

std::optional<std::string> TypeConversion::Bool::toString(bool const value){
    return value ? std::optional<std::string>{"true"} : std::optional<std::string>{"false"};
}

// Double

std::optional<bool> TypeConversion::Double::toBool(double const value) {
    return std::optional{std::fabs(value) > std::numeric_limits<double>::epsilon()};
}

} // namespace Nebulite::Utility
