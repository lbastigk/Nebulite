//------------------------------------------
// Includes

// Standard library
#include <cmath>

// Nebulite
#include "Nebulite.hpp"
#include "Core/RenderObject.hpp"
#include "Data/DocumentCache.hpp"
#include "Data/JSON.hpp"
#include "Interaction/Invoke.hpp"
#include "Interaction/Logic/Assignment.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Interaction {

//------------------------------------------
// Constructor / Destructor

Invoke::Invoke() {
    // Initialize synchronization primitives
    threadState.stopFlag = false;

    // Create and start threads
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w = std::make_unique<Data::BroadCastListenPairs>(threadState.stopFlag);
    }
}

Invoke::~Invoke() {
    // Signal threads to stop and finish
    // Stopping is handled in BroadCastListenPairs destructor
    threadState.stopFlag = true;
}

//------------------------------------------
// Interactions

void Invoke::broadcast(std::shared_ptr<Rules::Ruleset> const& entry) {
    // Get index
    uint32_t const threadIndex = entry->getId() % THREADRUNNER_COUNT;
    worker[threadIndex]->broadcast(entry);
}

void Invoke::listen(Core::RenderObject* obj, std::string const& topic, uint32_t const& listenerId) {
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w->listen(obj, topic, listenerId);
    }
}

//------------------------------------------
// Update

void Invoke::update() {
    // Signal all worker threads to start processing
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w->startWork();
    }

    // Wait for all threads to finish processing
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w->waitForWorkFinished();
    }

    // Prepare work for the next frame
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w->prepare();
    }
}

} // namespace Nebulite::Interaction
