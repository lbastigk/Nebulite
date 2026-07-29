//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint> // NOLINT
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/MappedOrderedCacheList.hpp"
#include "Nebulite/Utility/Coordination/IdGenerator.hpp"

//------------------------------------------
namespace Nebulite::Data {

std::size_t MappedOrderedCacheList::generateUniqueId(std::string_view const identifier) {
    static auto generator = Utility::Coordination::IdGenerator::stringToRollingIdGenerator();
    return generator(identifier);
}

void MappedOrderedCacheList::addToVec(std::vector<double*>& vec, JsonScope const& reference, ScopedKeyView const& key){
    vec.push_back(reference.getStableDoublePointer(key));
}

} // namespace Nebulite::Data
