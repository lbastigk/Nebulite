#include "Data/Document/JsonScope.hpp"
#include "Data/OrderedDoublePointers.hpp"
#include "Interaction/Logic/VirtualDouble.hpp"

namespace Nebulite::Data {

odpvec* MappedOrderedDoublePointers::ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys) {
    // Quick-cache path protected by mtxCache
    if (uniqueId < quickCacheSize) {
        Utility::Coordination::WriteLock lock(mtxCache);
        return fromQuickCache(uniqueId, keys);
    }

    // Map path protected by mtxMap
    Utility::Coordination::WriteLock lock(mtxMap);
    return fromMap(uniqueId, keys);
}

odpvec* MappedOrderedDoublePointers::ensureOrderedCacheListNoLock(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys) {
    if (uniqueId < quickCacheSize) {
        return fromQuickCache(uniqueId, keys);
    }
    return fromMap(uniqueId, keys);
}

odpvec* MappedOrderedDoublePointers::fromQuickCache(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys) {
    if (!quickCache[uniqueId].orderedValues.empty()) [[likely]] {
        return &quickCache[uniqueId].orderedValues;
    }
    if (quickCache[uniqueId].orderedValues.empty()) {
        OrderedDoublePointers newCacheList(keys.size());
        for (auto const& key : keys) {
            double* ptr = reference.getStableDoublePointer(key);
            newCacheList.orderedValues.push_back(ptr);
        }
        quickCache[uniqueId] = std::move(newCacheList);
    }
    return &quickCache[uniqueId].orderedValues;
}

odpvec* MappedOrderedDoublePointers::fromMap(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys){
    if (auto const it = map.find(uniqueId); it != map.end()) [[likely]] {
        return &it->second.orderedValues;
    }
    auto [newIt, inserted] = map.try_emplace(uniqueId, OrderedDoublePointers(keys.size()));
    if (inserted) {
        for (auto const& key : keys) {
            double* ptr = reference.getStableDoublePointer(key);
            newIt->second.orderedValues.push_back(ptr);
        }
    }
    return &newIt->second.orderedValues;
}

} // namespace Nebulite::Data
