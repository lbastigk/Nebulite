#include "Data/OrderedDoublePointers.hpp"
#include "Data/JSON.hpp"
#include "Interaction/Logic/VirtualDouble.hpp"

namespace Nebulite::Data {

odpvec* MappedOrderedDoublePointers::ensureOrderedCacheList(uint64_t uniqueId, Nebulite::Data::JSON* reference, std::vector<std::shared_ptr<Interaction::Logic::VirtualDouble>> const& contextOther) {
    std::scoped_lock cache_lock(mtx);

    // Check if we can use quickcache, that does not rely on a hashmap lookup
    if (uniqueId < Data::MappedOrderedDoublePointers::quickCacheSize) {
        if (quickCache[uniqueId].orderedValues.empty()) {
            // Not initialized yet, create one with exact size
            Data::OrderedDoublePointers newCacheList(contextOther.size());

            // Populate list with all virtual doubles from type other
            for (auto const& vde : contextOther) {
                double* ptr = reference->getStableDoublePointer(vde->getKey());
                newCacheList.orderedValues.push_back(ptr);
            }
            quickCache[uniqueId] = std::move(newCacheList);
        }
        return &quickCache[uniqueId].orderedValues;
    }

    // If id is too large for quickcache, use hashmap
    auto it = map.find(uniqueId);

    // If not, create one
    if (it == map.end()) {
        Data::OrderedDoublePointers newCacheList(contextOther.size());

        // Populate list with all virtual doubles from type other
        for (auto const& vde : contextOther) {
            double* ptr = reference->getStableDoublePointer(vde->getKey());
            newCacheList.orderedValues.push_back(ptr);
        }

        map.emplace(uniqueId, std::move(newCacheList));
        it = map.find(uniqueId);
    }
    return &it->second.orderedValues;
}

}