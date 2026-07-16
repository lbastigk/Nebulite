#ifndef NEBULITE_DATA_ORDEREDCACHELIST_HPP
#define NEBULITE_DATA_ORDEREDCACHELIST_HPP

//------------------------------------------
// Includes

// Standard library
#include <concepts>
#include <cstddef>
#include <ranges>
#include <string_view>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Utility/Coordination/SharedMutex.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class ScopedKeyView;
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------

namespace Nebulite::Data {
/**
 * @class MappedOrderedCacheList
 * @brief A thread-safe map from strings to OrderedCacheList objects.
 */
class MappedOrderedCacheList {
public:
    explicit MappedOrderedCacheList(JsonScope& ownerReference)
        : reference(ownerReference) {
    }

    using OrderedCacheList = std::vector<double*>;

    /**
     * @brief Generates a unique ID for a list of keys based on a given string.
     * @param identifier The name of the ruleset, or any other string to generate a unique ID from.
     * @return A unique ID corresponding to the given string.
     */
    static std::size_t generateUniqueId(std::string_view identifier);

    /**
     * @brief Ensures the existence of an ordered cache list of double pointers for a set of keys.
     * @param uniqueId The unique ID for the ordered cache list.
     * @param keys The vector of keys to populate the cache with.
     * @return A pointer to the ordered vector of double pointers for the specified keys.
     */
    template <std::ranges::input_range R> requires std::convertible_to<std::ranges::range_reference_t<R>, ScopedKeyView>
    double** ensureOrderedCacheList(std::size_t uniqueId, R&& keys) {
        Utility::Coordination::WriteLock const lock(mtxMap);
        return fromMap(uniqueId, std::forward<R>(keys));
    }

    /**
     * @brief Ensures the existence of an ordered cache list of double pointers for a set of keys. Non-locking version.
     * @param uniqueId The unique ID for the ordered cache list.
     * @param keys The vector of keys to populate the cache with.
     * @return A pointer to the ordered vector of double pointers for the specified keys.
     */
    template <std::ranges::input_range R> requires std::convertible_to<std::ranges::range_reference_t<R>, ScopedKeyView>
    double** ensureOrderedCacheListNoLock(std::size_t uniqueId, R&& keys) {
        return fromMap(uniqueId, std::forward<R>(keys));
    }

private:
    /**
     * @brief Reference to the JSON document that owns this cache.
     */
    JsonScope& reference;

    /**
     * @brief Map from unique IDs to OrderedCacheList objects.
     */
    absl::flat_hash_map<std::size_t, OrderedCacheList> map;

    /**
     * @brief Mutex for thread-safe access to the map.
     */
    Utility::Coordination::SharedMutex mtxMap;

    /**
     * @brief Retrieves or creates an ordered cache list of double pointers for a set of keys from the map.
     * @details Not protected by any mutex
     * @param uniqueId The unique id of the entry
     * @param keys The keys to potentially create the entry with
     * @return An ordered vector of double pointers corresponding to the keys, either retrieved from the map or newly created if it did not exist.
     */
    template <std::ranges::input_range R> requires std::convertible_to<std::ranges::range_reference_t<R>, ScopedKeyView>
    double** fromMap(std::size_t uniqueId, R&& keys){
        if (auto const it = map.find(uniqueId); it != map.end()) [[likely]] {
            return it->second.data();
        }
        auto [newIt, inserted] = map.try_emplace(uniqueId, OrderedCacheList());
        if (inserted) {
            newIt->second.reserve(keys.size());
            for (auto const& key : std::forward<R>(keys)) {
                addToVec(newIt->second, reference, key);
            }
        }
        return newIt->second.data();
    }

    static void addToVec(std::vector<double*>& vec, JsonScope const& reference, ScopedKeyView const& key);
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_ORDEREDCACHELIST_HPP
