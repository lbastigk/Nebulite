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

    // Copy ctor: snapshot other under its read-lock, rebuild pointer into the new map
    HotKeyMap(HotKeyMap const& other) {
        Utility::ReadLock rlock(other.mtxMap);
        map = other.map;
        hotKeyEntry.active = other.hotKeyEntry.active;
        hotKeyEntry.key = other.hotKeyEntry.key;
        if (hotKeyEntry.active) {
            auto it = map.find(hotKeyEntry.key);
            hotKeyEntry.value = it != map.end() ? &it->second : nullptr;
        } else {
            hotKeyEntry.value = nullptr;
        }
    }

    // Copy assign: copy-and-swap
    HotKeyMap& operator=(HotKeyMap const& other) {
        if (this == &other) return *this;
        HotKeyMap tmp(other);
        swap(tmp);
        return *this;
    }

    // Move ctor: take other's map under its write-lock and fix pointers
    HotKeyMap(HotKeyMap&& other) noexcept(std::is_nothrow_move_constructible_v<absl::node_hash_map<K, V>>) {
        Utility::WriteLock wlock(other.mtxMap);
        map = std::move(other.map);
        hotKeyEntry.active = other.hotKeyEntry.active;
        hotKeyEntry.key = std::move(other.hotKeyEntry.key);
        if (hotKeyEntry.active) {
            auto it = map.find(hotKeyEntry.key);
            hotKeyEntry.value = it != map.end() ? &it->second : nullptr;
        } else {
            hotKeyEntry.value = nullptr;
        }
        // leave other in a valid empty state
        other.hotKeyEntry.active = false;
        other.hotKeyEntry.value = nullptr;
    }

    // Move assign: move-and-swap
    HotKeyMap& operator=(HotKeyMap&& other) noexcept(std::is_nothrow_move_assignable_v<absl::node_hash_map<K, V>>) {
        if (this == &other) return *this;
        HotKeyMap tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    // swap helper: locks both mutexes (std::scoped_lock uses deadlock-free std::lock)
    void swap(HotKeyMap& other) noexcept {
        if (this == &other) return;
        // NOLINTNEXTLINE
        std::scoped_lock lock(mtxMap, other.mtxMap);
        using std::swap;
        swap(map, other.map);
        swap(hotKeyEntry.active, other.hotKeyEntry.active);
        swap(hotKeyEntry.key, other.hotKeyEntry.key);

        if (hotKeyEntry.active) {
            auto it = map.find(hotKeyEntry.key);
            hotKeyEntry.value = it != map.end() ? &it->second : nullptr;
        } else {
            hotKeyEntry.value = nullptr;
        }

        if (other.hotKeyEntry.active) {
            auto it2 = other.map.find(other.hotKeyEntry.key);
            other.hotKeyEntry.value = it2 != other.map.end() ? &it2->second : nullptr;
        } else {
            other.hotKeyEntry.value = nullptr;
        }
    }

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

    void clear() {
        Utility::WriteLock ulock(mtxMap);
        map.clear();
        hotKeyEntry.active = false;
        hotKeyEntry.value = nullptr;
    }

    void erase(K const& key) {
        Utility::WriteLock ulock(mtxMap);
        map.erase(key);
        if (hotKeyEntry.active && hotKeyEntry.key == key) {
            hotKeyEntry.active = false;
            hotKeyEntry.value = nullptr;
        }
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
        V* value = nullptr;
    }hotKeyEntry;
};
} // namespace Nebulite::Data
#endif // DATA_HOTKEY_MAP_HPP
