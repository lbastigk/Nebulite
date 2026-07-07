//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <cstddef>
#include <cstdint> // NOLINT
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Utility/Coordination/IdGenerator.hpp"

//------------------------------------------
namespace Nebulite::Utility::Coordination {

std::function<size_t()> IdGenerator::atomicRollingIdGenerator(std::size_t const distributionSize) {
    // Each call gets its own shared counter
    // Using a shared pointer is required so the lambda is copyable
    auto counter = std::make_shared<std::atomic<size_t>>(0);

    return [counter, distributionSize] {
        return counter->fetch_add(1, std::memory_order_relaxed) % distributionSize;
    };
}


std::function<size_t()> IdGenerator::atomicIncrementIdGenerator() {
    // Each call gets its own shared counter
    // Using a shared pointer is required so the lambda is copyable
    auto counter = std::make_shared<std::atomic<size_t>>(0);

    return [counter] {
        return counter->fetch_add(1, std::memory_order_relaxed);
    };
}

std::function<std::size_t(std::string_view)> IdGenerator::stringToRollingIdGenerator() {
    // Each call gets its own shared map and counter
    // Using a shared pointer is required so the lambda is copyable
    auto mutex = std::make_shared<std::mutex>();
    auto idMap = std::make_shared<absl::flat_hash_map<std::string, std::size_t>>();
    auto counter = std::make_shared<std::size_t>(1);

    return [mutex, idMap, counter](std::string_view const key) {
        if (*counter > std::numeric_limits<std::size_t>::max() - 1) {
            // Throw an error or handle overflow as needed
            throw std::overflow_error("Nebulite::Utility::Coordination::IdGenerator::stringToRollingIdGenerator has exceeded maximum limit.");
        }
        std::scoped_lock const lock(*mutex);
        if (auto const& it = idMap->find(key); it != idMap->end()) {
            return it->second;
        }
        std::size_t const id = (*counter)++;
        (*idMap)[std::string(key)] = id;
        return id;
    };
}

} // namespace Nebulite::Utility::Coordination
