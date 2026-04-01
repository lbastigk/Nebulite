/**
 * @file Alignment.hpp
 * @brief Constants for data alignment in Nebulite.
 */

#ifndef NEBULITE_CONSTANTS_ALIGNMENT_HPP
#define NEBULITE_CONSTANTS_ALIGNMENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <new> // for std::hardware_destructive_interference_size (if available)

// Nebulite
#include "Utility/CompileTimeEvaluate.hpp"

//------------------------------------------
namespace Nebulite::Constants {
class Alignment {
public:
#ifndef __cpp_lib_hardware_interference_size
    static constexpr std::size_t SIMD_ALIGN = 64; // fallback: 64 bytes covers AVX-512/AVX2
#else
    static constexpr std::size_t SIMD_ALIGN = std::hardware_destructive_interference_size;
#endif
private:
    //------------------------------------------
    // Assertions

    // Check if SIMD_ALIGN is a power of two
    static_assert(Utility::CompileTimeEvaluate::isPowerOfTwo(SIMD_ALIGN), "SIMD_ALIGN must be a power of two.");
};

} // namespace Nebulite::Constants
#endif // NEBULITE_CONSTANTS_ALIGNMENT_HPP
