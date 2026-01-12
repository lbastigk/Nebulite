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
#include "Data/HotKeyMap.hpp"

namespace Nebulite::Data {

template<typename V>
class HotStringKeyMap {
public:
    static constexpr std::size_t BucketCount =
        static_cast<std::size_t>(std::numeric_limits<unsigned char>::max()) + 1;
private:
    // Use 256 buckets for first-byte partitioning
    using MapType = Data::HotKeyMap<std::string, V>;

    // Array of HotKeyMaps, one per possible first-character value.
    std::array<MapType, BucketCount> map;

    // (Optional) sanity check
    static_assert(BucketCount == 256, "Expected 256 buckets for HotStringKeyMap");

    // Deduce iterator types from MapType::begin()
    using iterator_type = decltype(std::declval<MapType&>().begin());
    // If MapType has no const begin(), reuse the non-const iterator type for const_iterator.
    using const_iterator_type = iterator_type;
    using value_type = std::remove_reference_t<decltype(*std::declval<iterator_type>())>;

public:
    auto& getMaps() {
        return map;
    }

    struct iterator {
        decltype(map[0].begin()) mapIterator;
        bool isValid;
    };

    iterator find(std::string const& key) {
        if (key.empty()) {
            auto const it = map[0].find(key);
            return iterator{it, it != map[0].end()};
        }
        auto const firstChar = static_cast<unsigned char>(key[0]);
        auto const it = map[firstChar].find(key);
        return iterator{it, it != map[firstChar].end()};
    }

    V& operator[](std::string const& key) {
        if (key.empty()) {
            return map[0][key];
        }
        auto const firstChar = static_cast<unsigned char>(key[0]);
        return map[firstChar][key];
    }

    bool hasKey(std::string const& key) {
        if (key.empty()) {
            return map[0].find(key) != map[0].end();
        }
        auto const firstChar = static_cast<unsigned char>(key[0]);
        return map[firstChar].find(key) != map[firstChar].end();
    }
};

} // namespace Nebulite::Data

#endif // DATA_HOT_STRING_KEY_MAP_HPP
