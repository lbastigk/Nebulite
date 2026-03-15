/**
 * @file IdGenerator.hpp
 * @brief Contains the declaration of the IdGenerator class, which provides various ID generator functions for generating thread IDs or other unique identifiers in a thread-safe manner.
 */

#ifndef NEBULITE_UTILITY_COORDINATION_ID_GENERATOR_HPP
#define NEBULITE_UTILITY_COORDINATION_ID_GENERATOR_HPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <memory>
#include <mutex>

// External
#include "absl/container/flat_hash_map.h"

//------------------------------------------
namespace Nebulite::Utility::Coordination {
/**
 * @class Nebulite::Utility::Coordination::IdGenerator
 * @brief Provides various ID generator functions for generating thread IDs or other unique identifiers in a thread-safe manner.
 * @details The generator functions return lambda functions that can be used to generate IDs based on different strategies,
 *          such as rolling or incrementing counters. The use of shared pointers ensures that the generators are
 */
class IdGenerator {
public:
    /**
     * @brief A generator function that returns a lambda which generates a uniform distribution of thread IDs.
     * @param distributionSize The size of the distribution.
     * @return A lambda function that generates a uniform distribution of thread IDs.
     */
    static std::function<size_t()> atomicThreadRollGenerator(size_t const& distributionSize);

    /*
     * @brief A generator function that returns a lambda which generates a uniform distribution of thread IDs that do not roll, but increase indefinitely.
     * @details Maximum value is size_t max before  "accidentally" rolling, but this is unlikely to happen in practice.
     * @return A lambda function that generates a uniform distribution of thread IDs that do not roll, but increase indefinitely.
     */
    static std::function<size_t()> atomicThreadIncrementGenerator();

    /**
     * @brief A generator function that returns a lambda which generates unique IDs for strings, rolling back to 1 after reaching UINT32_MAX.
     * @details This generator is thread-safe and can be used to generate unique IDs for strings in a concurrent environment. It uses a mutex to protect access to the internal map and counter.
     * @return A lambda function that generates unique IDs for strings, rolling back to 1 after reaching UINT32_MAX.
     * @todo Change to size_t
     */
    static std::function<uint32_t(std::string_view const&)> stringToRollingIdGenerator();
};
} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_COORDINATION_ID_GENERATOR_HPP
