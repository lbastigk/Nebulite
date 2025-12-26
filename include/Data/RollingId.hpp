/**
 * @file RollingId.hpp
 * @brief This file contains the declaration of the RollingId class, which provides
 *        a thread-safe mechanism for generating unique rolling IDs.
 */

#ifndef NEBULITE_DATA_ROLLING_ID_HPP
#define NEBULITE_DATA_ROLLING_ID_HPP

//------------------------------------------
// Includes

// Standard library
#include <mutex>
#include <string>

// External
#include "absl/container/flat_hash_map.h"

// Nebulite

//------------------------------------------
namespace Nebulite::Data {
class RollingId {
public:
    /**
     * @brief Constructor - initializes the rolling ID generator.
     */
    RollingId() = default;

    /**
     * @brief Generates or retrieves a unique rolling ID for the given key.
     * @param key The key for which to generate/retrieve the ID.
     * @return A unique rolling ID associated with the key.
     */
    uint32_t getId(std::string const& key) {
        if (counter > UINT32_MAX - 1) {
            // Throw an error or handle overflow as needed
            throw std::overflow_error("RollingId counter has exceeded maximum limit.");
        }
        std::scoped_lock lock(mutex);
        auto const& it = idMap.find(key);
        if (it != idMap.end()) {
            return it->second;
        }
        uint32_t id = counter++;
        idMap[key] = id;
        return id;
    }

private:
    // Mutex for thread-safe access
    mutable std::mutex mutex;

    // Map to store the current ID for each key
    absl::flat_hash_map<std::string, uint32_t> idMap;

    // Counter for generating unique IDs
    uint32_t counter = 1;
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_ROLLING_ID_HPP
