#include "Data/BroadcastListenContainer/FlatContainer.hpp"
#include "Utility/SharedMutex.hpp"

namespace Nebulite::Data::BroadcastListenContainer {

void FlatContainer::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    // Multiple threads may broadcast on the same topic, so we need to lock the broadcaster map for this topic while modifying it
    auto lock = broadcasters.lock(entry->getTopic());
    broadcasters[entry->getTopic()].push_back(entry);
}

void FlatContainer::listen(std::shared_ptr<Interaction::Rules::Listener> const& listener) {
    // Listening generally happens on multiple threads, so we need to lock the listener map for this topic while modifying it
    auto lock = listeners.lock(listener->topic);
    auto& t = listeners[listener->topic];
    t.push_back({listener});
}

void FlatContainer::prepare() {
    // Since we never build any pairs during broadcast/listen, there is no container swapping to be done.
    // instead, after broadcast/listen, we have a list of broadcasters and listeners for the current frame for each topic, which we can process in the worker thread.
}

void FlatContainer::process() {
    while (!threadState.stopFlag) {
        // Wait for work to be ready
        {
            auto lock = Utility::SharedLock(mutex);
            threadState.condition.wait(lock, [this] {
                return threadState.workReady.load() || threadState.stopFlag.load();
            });
        }

        // Process
        {
            listeners.forall([&](std::string const& topic, auto& lv) {
                auto const& bv = broadcasters[topic];

                size_t const lvSize = lv.size();
                for (size_t i = 0; i < lvSize; ++i) {
                    // Distribute listener access by offsetting the starting index for each worker thread
                    // This way, workers are less likely to contend for the same listeners when processing the same topic
                    size_t const idx = (i + workerInfo.index * (lvSize / workerInfo.count) ) % lvSize;
                    auto& listener = lv.at(idx);
                    for (auto const& ruleset : bv) { // No offsetting here, since all broadcasters are per-worker thread and thus already distributed. Tests show that offsetting here does not improve performance.
                        if (ruleset->getId() == listener->listenerId) {
                            continue; // Skip if the ruleset is from the same render object as the listener
                        }
                        if (ruleset->evaluateCondition(listener->domain)) {
                            ruleset->apply(listener);
                        }
                    }
                }

                lv.clear(); // Clear the listener vector for this topic after processing
            });

            // Cleanup: Clear all broadcaster vectors
            broadcasters.forall([&](auto& bv) {
                bv.clear();
            });
        }

        // Set work flags
        threadState.workReady = false;
        threadState.workFinished = true;
    }
}

} // namespace Nebulite::Data::BroadcastListenContainer
