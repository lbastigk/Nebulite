#include "Data/BroadcastListenContainer/BaseContainer.hpp"

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

    JsonScopeBase dummy;
    dummy.ensureOrderedCacheListMinimalLock(UINT64_MAX,{}); // Reserve the max id, triggering a thread id assignment for this thread
    threadIdAssigned = true;
}

} // namespace Nebulite::Data::BroadcastListenContainer
