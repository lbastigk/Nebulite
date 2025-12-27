#include "Data/Branch.hpp"

#include <memory>

namespace Nebulite::Data {

template<typename StoreType, typename idType, std::size_t MaxBits>
std::shared_ptr<StoreType> Branch<StoreType, idType, MaxBits>::at(idType const& id) {
    std::size_t const index = idToIndex(id);
    if (index >= MaxSize) {
        throw std::out_of_range(std::string(__FUNCTION__) + " - index exceeds MaxSize");
    }
    std::scoped_lock lock(mutex);

    if (storage.size() <= index) {
        storage.resize(index + 1);
    }

    wasAccessed[index] = true;
    if (!storage[index]) {
        storage[index] = std::make_shared<StoreType>();
    }
    return storage[index];
}

} // namespace Nebulite::Data
