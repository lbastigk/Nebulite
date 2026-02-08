#include "Nebulite.hpp"
#include "Data/BroadcastListenContainer/MapContainer.hpp"

namespace Nebulite::Data::BroadcastListenContainer {

//------------------------------------------
// Container Methods

void MapContainer::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) const {
    nextFrame->insertBroadcaster(entry);
}

void MapContainer::listen(Interaction::Execution::Domain& listener, std::string const& topic, uint32_t const& listenerId) const {
    thisFrame->insertListener(listener, topic, listenerId);
}

//------------------------------------------
// Worker Thread Methods

void MapContainer::prepare() {
    // Swap pointers
    std::swap(thisFrame, nextFrame);
}

//------------------------------------------
// Private Methods

void MapContainer::process()  {
    while (!threadState.stopFlag) {
        // Wait for work to be ready
        {
            std::unique_lock lock = thisFrame->lock();
            threadState.condition.wait(lock, [this] {
                return threadState.workReady.load() || threadState.stopFlag.load();
            });
        }

        // Process
        thisFrame->process();

        // Set work flags
        threadState.workReady = false;
        threadState.workFinished = true;
    }
}

} // namespace Nebulite::Data
