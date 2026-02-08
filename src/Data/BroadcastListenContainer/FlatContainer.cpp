#include "Data/BroadcastListenContainer/FlatContainer.hpp"

namespace Nebulite::Data::BroadcastListenContainer {
void FlatContainer::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    broadcasters[entry->getTopic()].push_back(entry);
}

void FlatContainer::listen(Interaction::Execution::Domain& listener, std::string const& topic, uint32_t const& listenerId) {
    listeners[topic].push_back({&listener, listenerId});
}

void FlatContainer::prepare() {
    // Nothing to do here
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
            listeners.forall([&](std::string const& topic, auto& v) {
                auto const& bv = broadcasters[topic];
                for (auto& listener : v) {
                    for (auto const& ruleset : bv) {
                        if (ruleset->getId() == listener.id) {
                            continue; // Skip if the ruleset is from the same render object as the listener
                        }
                        if (ruleset->evaluateCondition(listener.domain)) {
                            ruleset->apply(listener.domain);
                        }
                    }
                }

                v.clear(); // Clear the listener vector for this topic after processing
            });

            // Cleanup: Clear all broadcaster vectors
            broadcasters.forall([&](auto& v) {
                v.clear();
            });
        }

        // Set work flags
        threadState.workReady = false;
        threadState.workFinished = true;
    }
}

} // namespace Nebulite::Data::BroadcastListenContainer
