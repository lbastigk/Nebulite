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
    // Linking an empty doc is needed for some functions
    emptyDoc = new Data::JSON("Empty JSON document for Invoke");

    // Initialize synchronization primitives
    threadState.stopFlag = false;
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++) {
        threadState.individualState[i].workReady = false;
        threadState.individualState[i].workFinished = false;

        // Start worker threads
        threadrunners[i] = std::thread([this, i] {
            while (!threadState.stopFlag) {
                // Wait for work to be ready
                std::unique_lock lock(broadcasted.entriesThisFrame[i].mutex);
                threadState.individualState[i].condition.wait(lock, [this, i] {
                    return threadState.individualState[i].workReady.load() || threadState.stopFlag.load();
                });
                // Process
                if (threadState.stopFlag)
                    break;
                processWork(broadcasted.entriesThisFrame[i].Container);
                threadState.individualState[i].workReady = false;
                threadState.individualState[i].workFinished = true;
            }
        });
    }
}

Invoke::~Invoke() {
    // Signal threads to stop and finish
    threadState.stopFlag = true;
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++) {
        threadState.individualState[i].condition.notify_one();
        if (threadrunners[i].joinable()) {
            threadrunners[i].join();
        }
    }
}

//------------------------------------------
// Interactions

void Invoke::broadcast(std::shared_ptr<Rules::Ruleset> const& entry) {
    // Get index
    uint32_t const id_self = entry->getId();
    uint32_t const threadIndex = id_self % THREADRUNNER_COUNT;

    // Insert into next frame's entries
    std::scoped_lock lock(broadcasted.entriesNextFrame[threadIndex].mutex);
    auto& [isActive, rulesets] = broadcasted.entriesNextFrame[threadIndex].Container[entry->getTopic()][id_self];
    rulesets[entry->getIndex()].entry = entry;
    isActive = true;
}

void Invoke::listen(Core::RenderObject* obj, std::string const& topic, uint32_t const& listenerId) {
    for (auto& [container, mutex] : std::span(broadcasted.entriesThisFrame, THREADRUNNER_COUNT)) {
        // Lock to safely read from broadcasted.entriesThisFrame
        std::scoped_lock broadcastLock(mutex);

        // Check if any object has broadcasted on this topic
        auto topicIt = container.find(topic);
        if (topicIt == container.end()) {
            continue; // No entries for this topic in this thread
        }

        for (auto& [id_self, onTopicFromId] : topicIt->second) {
            // Skip if broadcaster and listener are the same object
            if (id_self == listenerId)
                continue;

            // Skip if inactive
            if (!onTopicFromId.active)
                continue;

            // For all rulesets under this broadcaster and topic
            for (auto& [entry, listeners] : std::ranges::views::values(onTopicFromId.rulesets)) {
                listeners[listenerId] = BroadCastListenPair{entry, obj, entry->evaluateCondition(obj)};
            }
        }
    }
}

//------------------------------------------
// Update

void Invoke::update() {
    // Signal all worker threads to start processing
    for (auto& [condition, workReady, workFinished] : std::span(threadState.individualState, THREADRUNNER_COUNT)) {
        workReady = true;
        workFinished = false;
        condition.notify_one();
    }

    // Wait for all threads to finish processing
    for (auto& [condition, workReady, workFinished] : std::span(threadState.individualState, THREADRUNNER_COUNT)) {
        while (!workFinished.load()) {
            std::this_thread::yield(); // Yield to avoid busy waiting
        }
    }

    // Swap the containers, preparing for the next frame
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++) {
        // No workers active -> no mutex lock needed
        std::swap(broadcasted.entriesThisFrame[i].Container, broadcasted.entriesNextFrame[i].Container);
    }
}

//------------------------------------------
// Work processing

void Invoke::processWork(BroadCastListenContainer& container) {
    thread_local std::mt19937 cleanup_rng(std::random_device{}());
    thread_local std::uniform_int_distribution<int> cleanup_dist(0, 99); // uniform, avoids modulo bias
    for (auto& map_other : std::views::values(container)) {
        for (auto& [isActive, rulesets] : std::views::values(map_other)) {
            if (!isActive)
                continue;
            for (auto& [entry, listeners] : std::ranges::views::values(rulesets)) {
                // Process active listeners (single pass, no erases here)
                for (auto & it : listeners) {
                    auto &pair = it.second;
                    if (pair.active) {
                        pair.entry->apply(pair.contextOther);
                        pair.active = false;
                    }
                }
                // Probabilistic cleanup performed once per ruleset
                if (cleanup_dist(cleanup_rng) == 0) {
                    for (auto it = listeners.begin(); it != listeners.end();) {
                        if (!it->second.active) {
                            auto itToErase = it++;
                            listeners.erase(itToErase); // erase returns void in Abseil
                        } else {
                            ++it;
                        }
                    }
                }
            }
            // Reset activity flag, must be activated on broadcast
            isActive = false;
        }
    }
}

} // namespace Nebulite::Interaction