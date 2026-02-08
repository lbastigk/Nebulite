#include "Data/BroadcastListenContainer/BaseContainer.hpp"

namespace Nebulite::Data::BroadcastListenContainer {

BaseContainer::BaseContainer(std::atomic<bool>& stopFlag) : threadState{ .stopFlag = stopFlag } {}

BaseContainer::~BaseContainer() noexcept {
    stopWorkerThread();
}

} // namespace Nebulite::Data::BroadcastListenContainer
