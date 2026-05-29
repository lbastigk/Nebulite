//------------------------------------------
// Includes

// Standard library
// NOLINTNEXTLINE
#include <cstddef>
#include <memory>
#include <ranges>

// Nebulite
#include "Interaction/Invoke.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Nebulite.hpp"
#include "Utility/Generate.hpp"

//------------------------------------------
namespace Nebulite::Interaction {

//------------------------------------------
// Constructor / Destructor

Invoke::Invoke()
    : worker(Utility::Generate::array<ContainerType, Constants::ThreadSettings::Maximum::invokeWorkerCount>([&](std::size_t const threadIndex) {
        return ContainerType(stopFlag, threadIndex, activeWorkerCount);
    }))
    , activeWorkers(worker | std::views::take(activeWorkerCount))
    , stopFlag(false)
{}

Invoke::~Invoke() {
    // Signal threads to stop and finish
    // Stopping is handled in BroadCastListenPairs destructor
    stopFlag = true;
}

//------------------------------------------
// Interactions

void Invoke::broadcast(std::shared_ptr<Rules::Ruleset> entry) {
    // Thread assignment based on entry owner ID.
    // We used a hash of the id as domain ids may not be equally distributed
    // E.g. the Broadcasting RenderObjects may be distributed every n IDs, which would lead to all work being done on one thread and the others idle.
    size_t const threadIndex = entry->getIdHashed() % activeWorkerCount;
    worker[threadIndex].broadcast(std::move(entry));
}

void Invoke::listen(std::shared_ptr<Rules::Listener> const& listener) {
    // Listening happens on all threads
    for (auto& w : activeWorkers) {
        w.listen(listener);
    }
}

//------------------------------------------
// Update

void Invoke::update() {
    activeWorkers = worker | std::views::take(activeWorkerCount);

    // Signal all worker threads to start processing
    for (auto& w : activeWorkers) {
        w.startWork();
    }

    // Wait for all threads to finish processing
    for (auto& w : activeWorkers) {
        w.waitForWorkFinished();
    }

    // Prepare work for the next frame
    for (auto& w : activeWorkers) {
        w.prepare();
    }
}

} // namespace Nebulite::Interaction
