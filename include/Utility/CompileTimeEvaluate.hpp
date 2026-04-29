/**
 * @file CompileTimeEvaluate.hpp
 * @brief Compile-time evaluation utilities for Nebulite.
 */

#ifndef UTILITY_COMPILE_TIME_EVALUATE_HPP
#define UTILITY_COMPILE_TIME_EVALUATE_HPP

//------------------------------------------
// Includes

// Standard library
#include <string_view>

//------------------------------------------
namespace Nebulite::Utility {

class CompileTimeEvaluate {
public:
    /**
     * @brief Checks if a string ends with a newline character.
     * @param str The string to check.
     * @return true if the string ends with a newline character, false otherwise.
     */
    static bool consteval endsWithNewline(std::string_view const& str) {
        return !str.empty() && str.back() == '\n';
    }

    /**
     * @brief A helper function that always evaluates to false.
     * @return false
     */
    static bool consteval always_false() {
        return false;
    }

    /**
     * @brief Checks if a number is a power of two.
     * @param n The number to check.
     * @return true if the number is a power of two, false otherwise.
     */
    static bool consteval isPowerOfTwo(std::size_t const& n) {
        // NOLINTNEXTLINE
        return n > 0 && (n & (n - 1)) == 0;
    }
};
} // namespace Nebulite::Utility
#endif // UTILITY_COMPILE_TIME_EVALUATE_HPP
