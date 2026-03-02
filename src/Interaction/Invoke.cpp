//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "Interaction/Invoke.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Interaction {

//------------------------------------------
// Constructor / Destructor

Invoke::Invoke() {
    // Initialize synchronization primitives
    stopFlag = false;

    // Create and start threads
    size_t workerIndex = 0;
    for (auto& w : worker | std::views::take(activeWorkerCount)) {
        w = std::make_unique<ContainerType>(stopFlag, workerIndex, activeWorkerCount);
        workerIndex++;
    }
}

Invoke::~Invoke() {
    // Signal threads to stop and finish
    // Stopping is handled in BroadCastListenPairs destructor
    stopFlag = true;
}

//------------------------------------------
// Interactions

void Invoke::broadcast(std::shared_ptr<Rules::Ruleset> const& entry) const {
    // Thread assignment based on entry owner ID
    uint32_t const threadIndex = entry->getId() % activeWorkerCount;
    worker[threadIndex]->broadcast(entry);
}

void Invoke::listen(std::shared_ptr<Rules::Listener> const& listener) {
    // Listening happens on all threads
    for (auto const& w : worker | std::views::take(activeWorkerCount)) {
        w->listen(listener);
    }
}

//------------------------------------------
// Update

void Invoke::update() {
    // Signal all worker threads to start processing
    for (auto const& w : worker | std::views::take(activeWorkerCount)) {
        w->startWork();
    }

    // Wait for all threads to finish processing
    for (auto const& w : worker | std::views::take(activeWorkerCount)) {
        w->waitForWorkFinished();
    }

    // Prepare work for the next frame
    for (auto const& w : worker | std::views::take(activeWorkerCount)) {
        w->prepare();
    }
}

} // namespace Nebulite::Interaction
