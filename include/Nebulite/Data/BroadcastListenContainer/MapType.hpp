#ifndef NEBULITE_DATA_BROADCASTLISTENCONTAINER_MAPTYPE_HPP
#define NEBULITE_DATA_BROADCASTLISTENCONTAINER_MAPTYPE_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string>
#include <vector>

//------------------------------------------
#ifdef NEBULITE_USE_INVOKE_CONTAINER_STRINGMAP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Data/Map/StringMap.hpp"

//------------------------------------------
template <typename Storage> using MapType = StringMap<std::vector<std::shared_ptr<Storage>>>;

#else // NEBULITE_USE_INVOKE_CONTAINER_STRINGMAP

//------------------------------------------
// Includes

// Standard library
#include <string_view>

// External
#include <absl/container/flat_hash_map.h>

//------------------------------------------
template <typename V>
class ContainerMap : absl::flat_hash_map<std::string, V> {
public:
    /**
     * @brief Access or insert an element by key.
     * @param key The string key to access.
     * @return Reference to the value associated with the key.
     */
    V& operator[](std::string_view const key) {
        return absl::flat_hash_map<std::string, V>::operator[](std::string(key));
    }

    /**
     * @brief Clear all entries.
     */
    void clear() {
        return absl::flat_hash_map<std::string, V>::clear();
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

        for (auto& [key, value] : *this) {
            func(key, value);
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

        for (auto& [key, value] : *this) {
            func(value);
        }
    }
};

template <typename Storage> using MapType = ContainerMap<std::vector<std::shared_ptr<Storage>>>;

#endif // NEBULITE_USE_INVOKE_CONTAINER_STRINGMAP

#endif // NEBULITE_DATA_BROADCASTLISTENCONTAINER_MAPTYPE_HPP
