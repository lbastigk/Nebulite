#ifndef NEBULITE_MATH_EQUALITY_HPP
#define NEBULITE_MATH_EQUALITY_HPP

//------------------------------------------
// Includes

// Standard library
#include <cmath>

// Nebulite
#include "Utility/CompileTimeEvaluate.hpp"

//------------------------------------------
namespace Nebulite::Math {

/**
 * @brief Compares to types for equality, using an appropriate method based on the type.
 * @details For floating-point types, an epsilon comparison is used to account for precision issues.
 * @tparam T The type to compare
 * @param a First value
 * @param b Second value
 * @return True if the values are considered equal, false otherwise.
 */
template <typename T> bool isEqual(T const& a, T const& b) {
    // For any floating point, use an epsilon comparison to account for precision issues
    if constexpr (std::is_floating_point_v<T>) {
        return std::fabs(a - b) < std::numeric_limits<T>::epsilon();
    }
    // For any T with an equality operator, use it
    else if constexpr (requires(T x, T y) { { x == y } -> std::convertible_to<bool>; }) {
        return a == b;
    }
    else {
        static_assert(Utility::CompileTimeEvaluate::always_false(), "isEqual does not support this type. It must either be a floating-point type or support operator==.");
        return false;
    }
}

/**
 * @brief Floating-Point safe check if a double value is nonzero
 * @param value The value to check
 * @return True if the value is considered nonzero, false if it is considered zero (within an epsilon threshold).
 */
inline bool isNonZero(double const& value) {
    return std::fabs(value) > std::numeric_limits<double>::epsilon();
}

} // namespace Nebulite::Math
#endif // NEBULITE_MATH_EQUALITY_HPP
