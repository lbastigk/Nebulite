/**
 * @file Asserts.hpp
 * @brief Compile-time assertion utilities for Nebulite.
 */

#ifndef CONSTANTS_ASSERTS_HPP
#define CONSTANTS_ASSERTS_HPP

#include <string_view>

namespace Nebulite::Constants {

class Assert {
public:
    /**
     * @brief Checks if a string ends with a newline character.
     * @param str The string to check.
     * @return true if the string ends with a newline character, false otherwise.
     */
    static bool constexpr endsWithNewline(std::string_view const& str) {
        return !str.empty() && str.back() == '\n';
    }

    /**
     * @brief A helper function that always evaluates to false.
     * @return false
     */
    static bool constexpr always_false() {
        return false;
    }

    /**
     * @brief Checks if a number is a power of two.
     * @param n The number to check.
     * @return true if the number is a power of two, false otherwise.
     */
    static bool constexpr isPowerOfTwo(std::size_t const& n) {
        // NOLINTNEXTLINE
        return n > 0 && (n & (n - 1)) == 0;
    }
};
} // namespace Nebulite::Constants

#endif // CONSTANTS_ASSERTS_HPP
