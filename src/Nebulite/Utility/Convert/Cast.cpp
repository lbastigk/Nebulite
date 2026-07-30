//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <limits>
#include <optional>
#include <string>

// Nebulite
#include "Nebulite/Utility/Convert/Cast.hpp"

//------------------------------------------
namespace Nebulite::Utility::Convert {

// Bool

std::optional<std::string> Cast::Bool::toString(bool const value){
    return value ? std::optional<std::string>{"true"} : std::optional<std::string>{"false"};
}

// Double

std::optional<bool> Cast::Double::toBool(double const value) {
    return std::optional{std::fabs(value) > std::numeric_limits<double>::epsilon()};
}

} // namespace Nebulite::Utility::Convert
