/**
 * @file StringMap.hpp
 * @brief Definition of the StringMap class, a wrapper for string-keyed hashmaps with hotkey functionality.
 * @details Experimental, seems to crash sometimes.
 */

#ifndef DATA_MAP_STRINGMAP_HPP
#define DATA_MAP_STRINGMAP_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstddef>
#include <limits>
#include <ranges>
#include <string>
#include <string_view>

// External
#include <absl/container/flat_hash_map.h>

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

    static std::size_t getIndex(std::string_view const key) {
        if (key.empty()) {
            return 0;
        }
        return static_cast<unsigned char>(key.back());
    }

public:
    StringMap() = default;

    ~StringMap() = default;

    StringMap(StringMap&& other) = delete;
    StringMap& operator=(StringMap&& other) = delete;
    StringMap(StringMap const&) = delete;
    StringMap& operator=(StringMap const&) = delete;

    /**
     * @brief Access or insert an element by key.
     * @param key The string key to access.
     * @return Reference to the value associated with the key.
     */
    V& operator[](std::string_view const key) {
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

    /**
     * @brief Applies a function to all values, passing both key and value
     * @tparam F The function type to apply. First argument is the key, second argument is the value (V&).
     * @param func The function to apply
     */
    template<typename F>
    void forall(F func) {
        static_assert(
            std::is_invocable_v<F, std::string const&, V&>,
            "Function must be invocable with (std::string const&, V&)"
        );

        for (auto& bucket : map) {
            for (auto& [key, value] : bucket) {
                func(key, value);
            }
        }
    }

    /**
     * @brief Applies a function to all values
     * @tparam F The function type to apply. First argument is the key, second argument is the value (V&).
     * @param func The function to apply
     */
    template<typename F>
    void forallValues(F func) {
        static_assert(
            std::is_invocable_v<F, V&>,
            "Function must be invocable with (V&)"
        );

        for (auto& bucket : map) {
            for (auto& value : bucket | std::views::values) {
                func(value);
            }
        }
    }
};

} // namespace Nebulite::Data

#endif // DATA_MAP_STRINGMAP_HPP
