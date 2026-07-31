#ifndef NEBULITE_UTILITY_CONVERT_BITS_HPP
#define NEBULITE_UTILITY_CONVERT_BITS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <limits>
#include <string>
#include <type_traits>

//------------------------------------------
namespace Nebulite::Utility::Convert::Bits {

/**
 * @brief Reverses the bits of a given input
 * @param input The bits to reverse
 * @param bitCount The amount of bits to reverse
 * @return The reversed bits
 */
std::size_t constexpr reverse(std::size_t input, std::size_t const bitCount) {
    std::size_t result = 0;
    for (std::size_t i = 0; i < bitCount; ++i) {
        result <<= 1u;
        result |= input & 1u;
        input >>= 1u;
    }
    return result;
}

template<typename T>
std::string constexpr toString(T input) {
    // T must be unsigned
    static_assert(std::is_integral_v<T>, "T must be integral");
    static_assert(std::is_unsigned_v<T>, "T must be unsigned");

    std::string result;
    result.reserve(sizeof(T) * 8);
    for (T bitMask = std::bit_floor(std::numeric_limits<T>::max()); bitMask; bitMask >>= 1) {
        result += input & bitMask ? '1' : '0';
    }
    return result;
}
} // namespace Nebulite::Utility::Convert::Bits
#endif // NEBULITE_UTILITY_CONVERT_BITS_HPP
