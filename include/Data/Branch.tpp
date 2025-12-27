#include "Data/Branch.hpp"

#include <memory>

namespace Nebulite::Data {

template<typename StoreType, typename idType, std::size_t MaxBits>
std::shared_ptr<StoreType> Branch<StoreType, idType, MaxBits>::at(idType const& id) {
    std::size_t const index = idToIndex(id);

    // Fast path: shared read lock — no resize or creation
    {
        std::shared_lock<std::shared_mutex> slock(mutex);
        if (index < storage.size()) {
            auto ptr = storage[index]; // safe while holding shared lock
            if (ptr) {
                // mark accessed under a small dedicated lock to avoid upgrading the main lock
                std::lock_guard<std::mutex> lg(accessedMutex);
                wasAccessed[index] = true;
                return ptr;
            }
        }
    }

    // Need to resize or create: acquire exclusive lock and re-check
    {
        std::unique_lock<std::shared_mutex> ulock(mutex);
        if (storage.size() <= index) {
            storage.resize(index + 1);
            wasAccessed.resize(index + 1);
        }
        if (!storage[index]) {
            storage[index] = std::make_shared<StoreType>();
        }
        // mark accessed under accessedMutex (consistent locking order: storage mutex then accessedMutex)
        std::lock_guard<std::mutex> lg(accessedMutex);
        wasAccessed[index] = true;
        return storage[index];
    }
}
} // namespace Nebulite::Data
