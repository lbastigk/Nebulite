#ifndef CONSTANTS_ALIGNMENT_HPP
#define CONSTANTS_ALIGNMENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <new>

// Nebulite
#include "Utility/CompileTimeEvaluate.hpp"

//------------------------------------------
namespace Nebulite::Constants {
/**
 * @class Alignment
 * @brief Constants for data alignment in Nebulite.
 */
class Alignment {
public:
#ifndef __cpp_lib_hardware_interference_size
    static constexpr std::size_t SIMD_ALIGN = 64; // fallback: 64 bytes covers AVX-512/AVX2
#else // __cpp_lib_hardware_interference_size
    static constexpr std::size_t SIMD_ALIGN = std::hardware_destructive_interference_size;
#endif // __cpp_lib_hardware_interference_size

    static_assert(Utility::CompileTimeEvaluate::isPowerOfTwo(SIMD_ALIGN), "SIMD_ALIGN must be a power of two.");
};

} // namespace Nebulite::Constants
#endif // CONSTANTS_ALIGNMENT_HPP
