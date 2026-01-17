#include "Core/JsonScope.hpp"
#include "Data/OrderedDoublePointers.hpp"
#include "Interaction/Logic/VirtualDouble.hpp"

namespace Nebulite::Data {

odpvec* MappedOrderedDoublePointers::ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<std::shared_ptr<Interaction::Logic::VirtualDouble>> const& contextOther) {
    // Quick-cache path protected by mtxCache
    if (uniqueId < quickCacheSize) {
        {
            Utility::ReadLock read_quick(mtxCache);
            if (!quickCache[uniqueId].orderedValues.empty()) {
                return &quickCache[uniqueId].orderedValues;
            }
        }

        // upgrade to exclusive to initialize
        Utility::WriteLock write_quick(mtxCache);
        if (quickCache[uniqueId].orderedValues.empty()) {
            OrderedDoublePointers newCacheList(contextOther.size());
            for (auto const& vde : contextOther) {
                double* ptr = reference.getStableDoublePointer(vde->getScopedKey());
                newCacheList.orderedValues.push_back(ptr);
            }
            quickCache[uniqueId] = std::move(newCacheList);
        }
        return &quickCache[uniqueId].orderedValues;
    }

    // Map path protected by mtxMap
    {
        Utility::ReadLock read_map(mtxMap);
        if (auto const it = map.find(uniqueId); it != map.end()) {
            return &it->second.orderedValues;
        }
    }

    // upgrade to exclusive to insert into map
    Utility::WriteLock write_map(mtxMap);
    auto [newIt, inserted] = map.try_emplace(uniqueId, OrderedDoublePointers(contextOther.size()));
    if (inserted) {
        for (auto const& vde : contextOther) {
            double* ptr = reference.getStableDoublePointer(vde->getScopedKey());
            newIt->second.orderedValues.push_back(ptr);
        }
    }
    return &newIt->second.orderedValues;
}

odpvec* MappedOrderedDoublePointers::ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys) {
    // Quick-cache path protected by mtxCache
    if (uniqueId < quickCacheSize) {
        {
            Utility::ReadLock read_quick(mtxCache);
            if (!quickCache[uniqueId].orderedValues.empty()) {
                return &quickCache[uniqueId].orderedValues;
            }
        }

        // upgrade to exclusive to initialize
        Utility::WriteLock write_quick(mtxCache);
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

    // Map path protected by mtxMap
    {
        Utility::ReadLock read_map(mtxMap);
        if (auto const it = map.find(uniqueId); it != map.end()) {
            return &it->second.orderedValues;
        }
    }

    // upgrade to exclusive to insert into map
    Utility::WriteLock write_map(mtxMap);
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
