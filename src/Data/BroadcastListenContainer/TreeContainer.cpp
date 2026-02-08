#include "Nebulite.hpp"
#include "Data/BroadcastListenContainer/TreeContainer.hpp"

namespace Nebulite::Data::BroadcastListenContainer {

//------------------------------------------
// Container Methods

void TreeContainer::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    nextFrame->insertBroadcaster(entry);
}

void TreeContainer::listen(Interaction::Execution::Domain& listener, std::string const& topic, uint32_t const& listenerId) {
    thisFrame->insertListener(listener, topic, listenerId);
}

//------------------------------------------
// Worker Thread Methods

void TreeContainer::prepare() {
    // Swap pointers
    std::swap(thisFrame, nextFrame);
}

//------------------------------------------
// Private Methods

void TreeContainer::process()  {
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
