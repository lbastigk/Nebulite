#include "Data/RulesetPairings.hpp"

#include "Nebulite.hpp"
#include "Core/RenderObject.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Data {

//------------------------------------------
// Container Methods

void BroadCastListenPairs::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    nextFrame->insertBroadcaster(entry);
}

void BroadCastListenPairs::listen(Interaction::Execution::DomainBase& listener, std::string const& topic, uint32_t const& listenerId) {
    thisFrame->insertListener(listener, topic, listenerId);
}

//------------------------------------------
// Worker Thread Methods

void BroadCastListenPairs::prepare() {
    // Swap pointers
    std::swap(thisFrame, nextFrame);
}

void BroadCastListenPairs::startWork() {
    threadState.workReady = true;
    threadState.workFinished = false;
    threadState.condition.notify_one();
}

void BroadCastListenPairs::waitForWorkFinished() const {
    while (!threadState.workFinished.load()) {
        std::this_thread::yield();
    }
}

//------------------------------------------
// Private Methods

void BroadCastListenPairs::process()  {
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
