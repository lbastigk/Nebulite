#include "Data/BroadcastListenContainer/BaseContainer.hpp"

namespace Nebulite::Data::BroadcastListenContainer {

BaseContainer::BaseContainer(std::atomic<bool>& stopFlag, uint32_t const& workerIndex, uint32_t const& workerCount)
: workerInfo{workerIndex, workerCount}, threadState{ .stopFlag = stopFlag } {}

BaseContainer::~BaseContainer() noexcept {
    stopWorkerThread();
}

} // namespace Nebulite::Data::BroadcastListenContainer
