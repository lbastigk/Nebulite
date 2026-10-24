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

MappedOrderedCacheList::MappedOrderedCacheList([[clang::lifetimebound]] JsonScope& ownerReference)
    : reference(ownerReference) {}

std::size_t MappedOrderedCacheList::generateUniqueId(std::string_view const identifier, std::string_view const context) {
    static auto generator = Utility::Coordination::IdGenerator::twoStringsToRollingIdGenerator();
    return generator(identifier, context);
}

void MappedOrderedCacheList::addToVec(std::vector<double*>& vec, JsonScope const& reference, ScopedKeyView const& key){
    vec.push_back(reference.getStableDoublePointer(key));
}

} // namespace Nebulite::Data
