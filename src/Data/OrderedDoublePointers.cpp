//------------------------------------------
// Includes

// Standard library
#include <cstddef> // NOLINT
#include <cstdint> // NOLINT
#include <vector>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/ScopedKey.hpp"
#include "Data/OrderedCacheList.hpp"
#include "Utility/Coordination/SharedMutex.hpp"

//------------------------------------------
namespace Nebulite::Data {

double** MappedOrderedCacheList::ensureOrderedCacheList(std::size_t const uniqueId, std::vector<ScopedKeyView> const& keys) {
    Utility::Coordination::WriteLock const lock(mtxMap);
    return fromMap(uniqueId, keys);
}

double** MappedOrderedCacheList::ensureOrderedCacheListNoLock(std::size_t const uniqueId, std::vector<ScopedKeyView> const& keys) {
    return fromMap(uniqueId, keys);
}

double** MappedOrderedCacheList::fromMap(std::size_t const uniqueId, std::vector<ScopedKeyView> const& keys){
    if (auto const it = map.find(uniqueId); it != map.end()) [[likely]] {
        return it->second.data();
    }
    auto [newIt, inserted] = map.try_emplace(uniqueId, OrderedCacheList());
    if (inserted) {
        newIt->second.reserve(keys.size());
        for (auto const& key : keys) {
            newIt->second.push_back(reference.getStableDoublePointer(key));
        }
    }
    return newIt->second.data();
}

} // namespace Nebulite::Data
