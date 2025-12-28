#include "Data/Branch.hpp"

#include <memory>

namespace Nebulite::Data {

template<typename StoreType, typename idType, std::size_t MaxBits>
std::shared_ptr<StoreType> Branch<StoreType, idType, MaxBits>::at(idType const& id){
    const std::size_t index = idToIndex(id);

    // -------- Fast path: shared read --------
    {
        std::shared_lock<std::shared_mutex> slock(mutex);

        if (index < storage.size()) {
            auto ptr = storage[index];
            if (ptr) {
                markAccessed(index);
                return ptr;
            }
        }
    }

    // -------- Slow path: resize / create --------

    // Create outside lock to minimize exclusive hold time
    auto newObj = std::make_shared<StoreType>();

    {
        std::unique_lock<std::shared_mutex> ulock(mutex);

        // Ensure storage is large enough
        if (storage.size() <= index) {
            storage.resize(index + 1);
        }

        // Another thread may have created it already
        auto& slot = storage[index];
        if (!slot) {
            slot = std::move(newObj);
        }

        markAccessed(index);
        return slot;
    }
}

template<typename StoreType, typename idType, std::size_t MaxBits>
void Branch<StoreType, idType, MaxBits>::cleanup()  {
    thread_local std::uniform_int_distribution<size_t> distribution(0, MaxSize - 1);
    size_t const index = distribution(randNum);
    std::scoped_lock lock(mutex);
    if (index < storage.size() && !wasAccessed(index)) {
        storage[index] = nullptr;
    }
}

template<typename StoreType, typename idType, std::size_t MaxBits>
void Branch<StoreType, idType, MaxBits>::apply() {
    std::scoped_lock lock(mutex);
    size_t const n = storage.size();
    for (size_t i = 0; i < n; ++i) {
        if (wasAccessed(i)) {   // Storage validity is guarenteed through ::at()
            storage[i]->apply();
        }
    }

    // Reset access tracking
    resetAccessed();
}

} // namespace Nebulite::Data
