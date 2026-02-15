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
    /**
     * @brief A generator function that returns a lambda which generates a uniform distribution of thread IDs.
     * @param distributionSize The size of the distribution.
     * @return A lambda function that generates a uniform distribution of thread IDs.
     */
    static auto atomicThreadRollGenerator(size_t const& distributionSize) {
        // Each call gets its own shared counter
        // Using a shared pointer is required so the lambda is copyable
        auto counter = std::make_shared<std::atomic<size_t>>(0);

        return [counter, distributionSize] {
            thread_local size_t idx =
                counter->fetch_add(1, std::memory_order_relaxed) % distributionSize;
            return idx;
        };
    }
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_THREADING_HPP
