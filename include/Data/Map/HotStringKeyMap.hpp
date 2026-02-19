/**
 * @file HotStringKeyMap.hpp
 * @brief Definition of the HotStringKeyMap class, a wrapper for string-keyed hashmaps with hotkey functionality.
 * @details Experimental, seems to crash sometimes.
 */

#ifndef DATA_HOT_STRING_KEY_MAP_HPP
#define DATA_HOT_STRING_KEY_MAP_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstddef>
#include <limits>
#include <new>
#include <string>
#include <utility>
#include <functional>

// Nebulite
#include "Data/Map/HotKeyMap.hpp"
#include "Utility/SharedMutex.hpp"

//------------------------------------------
namespace Nebulite::Data {

template<typename V>
/**
 * @note We use the last character for better distribution in case of common prefixes.
 *       E.g.: As this class is used for Rulesets. static ones always start with "::"
 *       Meaning the first character is often the same.
 */
class HotStringKeyMap {
public:
    static auto constexpr BucketCount = static_cast<std::size_t>(std::numeric_limits<unsigned char>::max()) + 1;
private:
    // Use 256 buckets for first-byte partitioning
    using MapType = HotKeyMap<std::string, V>;

    // Array of HotKeyMaps, one per possible first-character value.
    std::array<MapType, BucketCount> map;
    static_assert(BucketCount == 256, "Expected 256 buckets for HotStringKeyMap");

    std::array<Utility::SharedMutex, BucketCount> bucketMutex; // Mutexes for each character bucket
public:
    HotStringKeyMap() = default;

    // Move ctor: move-construct each bucket from other (use placement-new so we don't require assignment)
    HotStringKeyMap(HotStringKeyMap&& other) noexcept {
        for (std::size_t i = 0; i < BucketCount; ++i) {
            map[i].~MapType();
            new (&map[i]) MapType(std::move(other.map[i]));
        }
    }

    // Move assign: destroy current buckets and move-construct from other into them
    HotStringKeyMap& operator=(HotStringKeyMap&& other) noexcept {
        if (this != &other) {
            for (std::size_t i = 0; i < BucketCount; ++i) {
                map[i].~MapType();
                new (&map[i]) MapType(std::move(other.map[i]));
            }
        }
        return *this;
    }

    HotStringKeyMap(HotStringKeyMap const&) = delete;
    HotStringKeyMap& operator=(HotStringKeyMap const&) = delete;

    /**
     * @brief Get underlying maps.
     * @return Reference to the array of underlying HotKeyMaps.
     */
    auto& getMaps() {
        return map;
    }

    /**
     * @brief The iterator structure for HotStringKeyMap.
     * @details Contains the underlying map iterator and a validity flag.
     */
    struct iterator {
        decltype(map[0].begin()) mapIterator;
        bool isValid;

        // Return the wrapped iterator's pointer-like value so `it->second` works
        decltype(mapIterator.operator->()) operator->() const noexcept {
            return mapIterator.operator->();
        }

        // Dereference to the underlying value_type so `*it` works
        decltype(auto) operator*() const noexcept {
            return *mapIterator;
        }

        bool operator!=(iterator const& other) const {
            return isValid != other.isValid || mapIterator != other.mapIterator;
        }
        bool operator==(iterator const& other) const {
            return !(*this != other);
        }
    };

    /**
     * @brief Find an element by key.
     * @param key The string key to search for.
     * @return An iterator to the found element, or an invalid iterator if not found.
     */
    iterator find(std::string const& key) {
        if (key.empty()) {
            auto const it = map[0].find(key);
            return iterator{it, it != map[0].end()};
        }
        auto const lastChar = static_cast<unsigned char>(key.back());
        auto const it = map[lastChar].find(key);
        return iterator{it, it != map[lastChar].end()};
    }

    iterator find(std::string_view const& key) {
        return find(std::string{key});
    }

    iterator end() {
        return iterator{ {}, false };
    }

    auto lock(std::string const& key) {
        if (key.empty()) {
            return Utility::SharedLock(bucketMutex[0]);
        }
        auto const lastChar = static_cast<unsigned char>(key.back());
        return Utility::SharedLock(bucketMutex[lastChar]);
    }

    /**
     * @brief Access or insert an element by key.
     * @param key The string key to access.
     * @return Reference to the value associated with the key.
     */
    V& operator[](std::string const& key) {
        if (key.empty()) {
            return map[0][key];
        }
        auto const lastChar = static_cast<unsigned char>(key.back());
        return map[lastChar][key];
    }

    /**
     * @brief Clear all entries in the HotStringKeyMap.
     */
    void clear() {
        for (auto& bucket : map) {
            bucket.clear();
        }
    }

    void erase(std::string const& key) {
        if (key.empty()) {
            map[0].erase(key);
            return;
        }
        auto const lastChar = static_cast<unsigned char>(key.back());
        map[lastChar].erase(key);
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
