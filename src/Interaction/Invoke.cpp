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
        w.pairContainer = std::make_unique<Data::BroadCastListenPairs>(threadState.stopFlag);
        w.workerThread = std::thread([&pairContainer = w.pairContainer] {
            pairContainer->process();
        });
    }
}

Invoke::~Invoke() {
    // Signal threads to stop and finish
    threadState.stopFlag = true;
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w.pairContainer->join();
        if (w.workerThread.joinable()) {
            w.workerThread.join();
        }
    }
}

//------------------------------------------
// Interactions

void Invoke::broadcast(std::shared_ptr<Rules::Ruleset> const& entry) {
    // Get index
    uint32_t const threadIndex = entry->getId() % THREADRUNNER_COUNT;
    worker[threadIndex].pairContainer->broadcast(entry);
}

void Invoke::listen(Core::RenderObject* obj, std::string const& topic, uint32_t const& listenerId) {
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w.pairContainer->listen(obj, topic, listenerId);
    }
}

//------------------------------------------
// Update

void Invoke::update() {
    // Signal all worker threads to start processing
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w.pairContainer->startWork();
    }

    // Wait for all threads to finish processing
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w.pairContainer->waitForWorkFinished();
    }

    // Swap the containers, preparing for the next frame
    for (auto& w : std::span(worker, THREADRUNNER_COUNT)) {
        w.pairContainer->swap();
    }
}

} // namespace Nebulite::Interaction