/**
 * @file Threading.hpp
 * @brief Threading utilities for Nebulite.
 */

#ifndef NEBULITE_UTILITY_THREADING_HPP
#define NEBULITE_UTILITY_THREADING_HPP

//------------------------------------------
// Includes

// Standard library
#include <thread>


//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::Threading
 * @brief Provides threading utilities.
 */
class Threading {
public:
    static size_t threadIdToUniformDistribution(size_t const& distributionSize) {
        size_t const h = std::hash<std::thread::id>{}(std::this_thread::get_id());
        return mix(h) % distributionSize;
    }

    static size_t atomicThreadRoll(size_t const& distributionSize) {
        static std::atomic<size_t> next{0};
        thread_local const size_t idx = next.fetch_add(1, std::memory_order_relaxed) % distributionSize;
        return idx;
    }

private:
    static size_t mix(size_t x) {
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return x;
    }
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_THREADING_HPP
