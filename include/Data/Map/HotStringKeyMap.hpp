/**
 * @file HotStringKeyMap.hpp
 * @brief Definition of the HotStringKeyMap class, a wrapper for string-keyed hashmaps with hotkey functionality.
 * @details Experimental, seems to crash sometimes.
 */

#ifndef DATA_HOT_STRING_KEY_MAP_HPP
#define DATA_HOT_STRING_KEY_MAP_HPP

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

// Nebulite
#include "HotKeyMap.hpp"

namespace Nebulite::Data {

template<typename V>
/**
 * @note We use the last character for better distribution in case of common prefixes.
 *       E.g.: As this class is used for Rulesets. static ones always start with "::"
 *       Meaning the first character is often the same.
 */
class HotStringKeyMap {
public:
    static constexpr std::size_t BucketCount =
        static_cast<std::size_t>(std::numeric_limits<unsigned char>::max()) + 1;
private:
    // Use 256 buckets for first-byte partitioning
    using MapType = Data::HotKeyMap<std::string, V>;

    // Array of HotKeyMaps, one per possible first-character value.
    std::array<MapType, BucketCount> map;
    static_assert(BucketCount == 256, "Expected 256 buckets for HotStringKeyMap");
public:
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
};

} // namespace Nebulite::Data

#endif // DATA_HOT_STRING_KEY_MAP_HPP
