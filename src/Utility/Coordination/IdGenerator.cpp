//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <memory>
#include <mutex>

// External
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Utility/Coordination/IdGenerator.hpp"

//------------------------------------------
namespace Nebulite::Utility::Coordination {

std::function<size_t()> IdGenerator::atomicThreadRollGenerator(size_t const& distributionSize) {
    // Each call gets its own shared counter
    // Using a shared pointer is required so the lambda is copyable
    auto counter = std::make_shared<std::atomic<size_t>>(0);

    return [counter, distributionSize] {
        thread_local size_t idx =
            counter->fetch_add(1, std::memory_order_relaxed) % distributionSize;
        return idx;
    };
}


std::function<size_t()> IdGenerator::atomicThreadIncrementGenerator() {
    // Each call gets its own shared counter
    // Using a shared pointer is required so the lambda is copyable
    auto counter = std::make_shared<std::atomic<size_t>>(0);

    return [counter] {
        thread_local size_t idx =
            counter->fetch_add(1, std::memory_order_relaxed);
        return idx;
    };
}

std::function<uint32_t(std::string_view const&)> IdGenerator::stringToRollingIdGenerator() {
    // Each call gets its own shared map and counter
    // Using a shared pointer is required so the lambda is copyable
    auto mutex = std::make_shared<std::mutex>();
    auto idMap = std::make_shared<absl::flat_hash_map<std::string, uint32_t>>();
    auto counter = std::make_shared<uint32_t>(1);

    return [mutex, idMap, counter](std::string_view const& key) {
        if (*counter > UINT32_MAX - 1) {
            // Throw an error or handle overflow as needed
            throw std::overflow_error("Nebulite::Utility::Coordination::IdGenerator::stringToRollingIdGenerator has exceeded maximum limit.");
        }
        std::scoped_lock lock(*mutex);
        if (auto const& it = idMap->find(key); it != idMap->end()) {
            return it->second;
        }
        uint32_t const id = (*counter)++;
        (*idMap)[std::string(key)] = id;
        return id;
    };
}

} // namespace Nebulite::Utility::Coordination
