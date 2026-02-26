#include "Data/BroadcastListenContainer/BaseContainer.hpp"
#include "Data/Document/JsonScopeBase.hpp"

namespace Nebulite::Data::BroadcastListenContainer {

BaseContainer::BaseContainer(std::atomic<bool>& stopFlag, uint32_t const& workerIndex, uint32_t const& workerCount)
: workerInfo{workerIndex, workerCount}, threadState{ .stopFlag = stopFlag } {
    // Ensure an early thread ID even for the base container
    ensureEarlyThreadId();
}

BaseContainer::~BaseContainer() noexcept {
    stopWorkerThread();
}

void BaseContainer::ensureEarlyThreadId() {
    thread_local bool threadIdAssigned = false;
    if (threadIdAssigned) return; // Already assigned for this thread
    if (size_t const id = JsonScopeBase::assignThreadIndex(); id >= JsonScopeBase::noLockArraySize) {
        throw std::runtime_error("Assigned thread ID for BroadcastListenContainer exceeds the non-locking id size. This means another class assigned a thread ID before this container. Ensure that the BroadcastListenContainer is initialized before any other class that assigns thread IDs!");
    }
    threadIdAssigned = true;
}

} // namespace Nebulite::Data::BroadcastListenContainer
