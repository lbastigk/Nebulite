#include "Data/Document/JsonScope.hpp"
#include "Data/OrderedDoublePointers.hpp"
#include "Interaction/Logic/VirtualDouble.hpp"

namespace Nebulite::Data {

odpvec* MappedOrderedDoublePointers::ensureOrderedCacheList(uint64_t uniqueId, JsonScope& reference, std::vector<std::shared_ptr<Interaction::Logic::VirtualDouble>> const& contextOther) {
    // Quick-cache path protected by mtxCache
    if (uniqueId < Data::MappedOrderedDoublePointers::quickCacheSize) {
        {
            std::shared_lock<std::shared_mutex> read_quick(mtxCache);
            if (!quickCache[uniqueId].orderedValues.empty()) {
                return &quickCache[uniqueId].orderedValues;
            }
        }

        // upgrade to exclusive to initialize
        std::unique_lock<std::shared_mutex> write_quick(mtxCache);
        if (quickCache[uniqueId].orderedValues.empty()) {
            Data::OrderedDoublePointers newCacheList(contextOther.size());
            for (auto const& vde : contextOther) {
                double* ptr = reference.getStableDoublePointer(vde->getKey());
                newCacheList.orderedValues.push_back(ptr);
            }
            quickCache[uniqueId] = std::move(newCacheList);
        }
        return &quickCache[uniqueId].orderedValues;
    }

    // Map path protected by mtxMap
    {
        std::shared_lock read_map(mtxMap);
        if (auto const it = map.find(uniqueId); it != map.end()) {
            return &it->second.orderedValues;
        }
    }

    // upgrade to exclusive to insert into map
    std::unique_lock<std::shared_mutex> write_map(mtxMap);
    auto [newIt, inserted] = map.try_emplace(uniqueId, Data::OrderedDoublePointers(contextOther.size()));
    if (inserted) {
        for (auto const& vde : contextOther) {
            double* ptr = reference.getStableDoublePointer(vde->getKey());
            newIt->second.orderedValues.push_back(ptr);
        }
    }
    return &newIt->second.orderedValues;
}

odpvec* MappedOrderedDoublePointers::ensureOrderedCacheList(uint64_t const& uniqueId, JsonScope& reference, std::vector<std::string_view> const& keys) {
    // Quick-cache path protected by mtxCache
    if (uniqueId < Data::MappedOrderedDoublePointers::quickCacheSize) {
        {
            std::shared_lock<std::shared_mutex> read_quick(mtxCache);
            if (!quickCache[uniqueId].orderedValues.empty()) {
                return &quickCache[uniqueId].orderedValues;
            }
        }

        // upgrade to exclusive to initialize
        std::unique_lock<std::shared_mutex> write_quick(mtxCache);
        if (quickCache[uniqueId].orderedValues.empty()) {
            Data::OrderedDoublePointers newCacheList(keys.size());
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
        std::shared_lock read_map(mtxMap);
        if (auto const it = map.find(uniqueId); it != map.end()) {
            return &it->second.orderedValues;
        }
    }

    // upgrade to exclusive to insert into map
    std::unique_lock<std::shared_mutex> write_map(mtxMap);
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
