#include "Data/Document/JsonScope.hpp"
#include "Data/OrderedCacheList.hpp"

namespace Nebulite::Data {

odpvec* MappedOrderedCacheList::ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys) {
    Utility::Coordination::WriteLock lock(mtxMap);
    return fromMap(uniqueId, keys);
}

odpvec* MappedOrderedCacheList::ensureOrderedCacheListNoLock(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys) {
    return fromMap(uniqueId, keys);
}

odpvec* MappedOrderedCacheList::fromMap(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys){
    if (auto const it = map.find(uniqueId); it != map.end()) [[likely]] {
        return &it->second.orderedValues;
    }
    auto [newIt, inserted] = map.try_emplace(uniqueId, OrderedCacheList(keys.size()));
    if (inserted) {
        for (auto const& key : keys) {
            double* ptr = reference.getStableDoublePointer(key);
            newIt->second.orderedValues.push_back(ptr);
        }
    }
    return &newIt->second.orderedValues;
}

} // namespace Nebulite::Data
