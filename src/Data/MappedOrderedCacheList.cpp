//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint> // NOLINT
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Data/MappedOrderedCacheList.hpp"
#include "Nebulite/Utility/Coordination/IdGenerator.hpp"
#include "Nebulite/Utility/Coordination/SharedMutex.hpp"

//------------------------------------------
namespace Nebulite::Data {

std::size_t MappedOrderedCacheList::generateUniqueId(std::string_view const identifier) {
    static auto generator = Utility::Coordination::IdGenerator::stringToRollingIdGenerator();
    return generator(identifier);
}

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
