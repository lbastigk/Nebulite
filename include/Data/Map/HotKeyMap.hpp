/**
 * @file HotKeyMap.hpp
 * @brief Definition of the HotKeyMap class, a wrapper for hashmaps with hotkey functionality.
 */

#ifndef DATA_HOTKEY_MAP_HPP
#define DATA_HOTKEY_MAP_HPP

//------------------------------------------
// Includes

// External
#include <absl/container/node_hash_map.h>

// Nebulite
#include <Utility/SharedMutex.hpp>

//------------------------------------------
namespace Nebulite::Data {

template<typename K, typename V>
class HotKeyMap {
public:
    HotKeyMap() = default;

    // Operator overload for []
    V& operator[](K const& key) {
        // Fast path and lookup under shared lock
        {
            Utility::ReadLock slock(mtxMap);
            if (hotKeyEntry.active && hotKeyEntry.key == key) {
                return *hotKeyEntry.value;
            }
        }

        // Acquire exclusive lock and re-check/create safely
        Utility::WriteLock ulock(mtxMap);
        auto it2 = map.find(key);
        if (it2 != map.end()) {
            hotKeyEntry.active = true;
            hotKeyEntry.key = key;
            hotKeyEntry.value = &it2->second;
            return it2->second;
        }

        // Create new entry
        auto& valueRef = map[key];
        hotKeyEntry.active = true;
        hotKeyEntry.key = key;
        hotKeyEntry.value = &valueRef;
        return valueRef;
    }

    /**
     * @brief Find an entry by key, utilizing hotkey optimization.
     * @param key The key to search for.
     * @return Iterator to the found entry or map.end() if not found.
     */
    auto find(K const& key) {
        Utility::ReadLock slock(mtxMap);
        if (hotKeyEntry.active && hotKeyEntry.key == key) {
            return map.find(hotKeyEntry.key); // Return iterator to hotkey entry
        }
        return map.find(key);
    }

    /**
     * @brief Begin iterator for the map.
     * @return Iterator to the beginning of the map.
     */
    auto begin() {
        Utility::ReadLock slock(mtxMap);
        return map.begin();
    }

    /**
     * @brief End iterator for the map.
     * @return Iterator to the end of the map.
     */
    auto end() {
        Utility::ReadLock slock(mtxMap);
        return map.end();
    }

    /*
     * @brief Begin iterator for const map.
     * @return Const iterator to the beginning of the map.
     */
    auto begin() const {
        return const_cast<HotKeyMap*>(this)->begin();
    }

    /**
     * @brief End iterator for const map.
     * @return Const iterator to the end of the map.
     */
    auto end() const {
        return const_cast<HotKeyMap*>(this)->end();
    }

private:
    absl::node_hash_map<K, V> map;
    mutable Utility::SharedMutex mtxMap;

    /**
     * @brief Holds the last accessed key-value pair for hotkey optimization.
     */
    struct HotKeyEntry {
        bool active = false;
        K key;
        V* value;
    }hotKeyEntry;
};
} // namespace Nebulite::Data
#endif // DATA_HOTKEY_MAP_HPP
