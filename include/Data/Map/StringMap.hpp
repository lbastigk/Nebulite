/**
 * @file StringMap.hpp
 * @brief Definition of the StringMap class, a wrapper for string-keyed hashmaps with hotkey functionality.
 * @details Experimental, seems to crash sometimes.
 */

#ifndef DATA_HOT_STRING_KEY_MAP_HPP
#define DATA_HOT_STRING_KEY_MAP_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstddef>
#include <functional>
#include <limits>
#include <string>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Utility/Coordination/SharedMutex.hpp"

//------------------------------------------
namespace Nebulite::Data {

template<typename V>
/**
 * @note We use the last character for better distribution in case of common prefixes.
 *       E.g.: As this class is used for Rulesets. static ones always start with "::"
 *       Meaning the first character is often the same.
 */
class StringMap {
public:
    static auto constexpr BucketCount = static_cast<std::size_t>(std::numeric_limits<unsigned char>::max()) + 1;

private:
    using MapType = absl::flat_hash_map<std::string, V>;

    // Array of HotKeyMaps, one per possible first-character value.
    std::array<MapType, BucketCount> map;
    static_assert(BucketCount == 256, "Expected 256 buckets for StringMap");

    std::array<Utility::Coordination::SharedMutex, BucketCount> bucketMutex; // Mutexes for each character bucket

    static size_t getIndex(std::string_view const& key) {
        if (key.empty()) {
            return 0;
        }
        return static_cast<unsigned char>(key.back());
    }

public:
    StringMap() = default;

    StringMap(StringMap&& other) = delete;
    StringMap& operator=(StringMap&& other) = delete;
    StringMap(StringMap const&) = delete;
    StringMap& operator=(StringMap const&) = delete;

    auto lock(std::string_view const& key) {
        return Utility::Coordination::SharedLock(bucketMutex[getIndex(key)]);
    }


    /**
     * @brief Access or insert an element by key.
     * @param key The string key to access.
     * @return Reference to the value associated with the key.
     */
    V& operator[](std::string_view const& key) {
        return map[getIndex(key)][key];
    }

    /**
     * @brief Clear all entries in the StringMap.
     */
    void clear() {
        for (auto& bucket : map) {
            bucket.clear();
        }
    }

    void forall(std::function<void(V&)> func) {
        for (auto& bucket : map) {
            for (auto& [_, value] : bucket) {
                func(value);
            }
        }
    }
    void forall(std::function<void(std::string const&, V&)> func) {
        for (auto& bucket : map) {
            for (auto& [key, value] : bucket) {
                func(key, value);
            }
        }
    }
};

} // namespace Nebulite::Data

#endif // DATA_HOT_STRING_KEY_MAP_HPP
