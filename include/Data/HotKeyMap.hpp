/**
 * @file HotKeyMap.hpp
 * @brief Definition of the HotKeyMap class, a wrapper for hashmaps with hotkey functionality.
 */

#ifndef DATA_HOTKEY_MAP_HPP
#define DATA_HOTKEY_MAP_HPP

//------------------------------------------
// Includes

// Standard library
#include <shared_mutex>

// External
#include <absl/container/node_hash_map.h>

//------------------------------------------
namespace Nebulite::Data {

template<typename K, typename V>
class HotKeyMap {
public:
    HotKeyMap() = default;

    // Operator overload for []
    V& operator[](K const& key) {
        std::shared_lock<std::shared_mutex> slock(mtxMap);
        if (hotKeyEntry.active && hotKeyEntry.key == key) {
            return *(hotKeyEntry.value);
        }
        // If not in hotkey, proceed to normal lookup
        auto const it = map.find(key);
        if (it != map.end()) {
            // Update hotkey entry
            {
                std::unique_lock<std::shared_mutex> ulock(mtxMap);
                hotKeyEntry.active = true;
                hotKeyEntry.key = key;
                hotKeyEntry.value = &(it->second);
            }
            return it->second;
        }
        // Entry not found, create standard entry
        slock.unlock();
        std::unique_lock<std::shared_mutex> ulock(mtxMap);
        auto& valueRef = map[key];
        // Update hotkey entry
        hotKeyEntry.active = true;
        hotKeyEntry.key = key;
        hotKeyEntry.value = &valueRef;
        return valueRef;
    }

    auto find(K const& key) {
        std::shared_lock<std::shared_mutex> slock(mtxMap);
        if (hotKeyEntry.active && hotKeyEntry.key == key) {
            return map.find(hotKeyEntry.key); // Return iterator to hotkey entry
        }
        return map.find(key);
    }

    auto begin() {
        std::shared_lock<std::shared_mutex> slock(mtxMap);
        return map.begin();
    }

    auto end() {
        std::shared_lock<std::shared_mutex> slock(mtxMap);
        return map.end();
    }

    auto begin() const {
        return const_cast<HotKeyMap*>(this)->begin();
    }

    auto end() const {
        return const_cast<HotKeyMap*>(this)->end();
    }

private:
    absl::node_hash_map<K, V> map;
    mutable std::shared_mutex mtxMap;

    struct HotKeyEntry {
        bool active = false;
        K key;
        V* value;
    }hotKeyEntry;
};
} // namespace Nebulite::Data
#endif // DATA_HOTKEY_MAP_HPP
