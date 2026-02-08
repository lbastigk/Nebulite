#include "Data/BroadcastListenContainer/FlatContainer.hpp"

namespace Nebulite::Data::BroadcastListenContainer {
void FlatContainer::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    broadcasters[entry->getTopic()].push_back(entry);
}

void FlatContainer::listen(Interaction::Execution::Domain& listener, std::string const& topic, uint32_t const&) {
    listeners[topic].push_back(&listener);
}

void FlatContainer::prepare() {
    // Nothing to do here
}

void FlatContainer::process() {
    listeners.forall([&](std::string const& topic, auto& v) {
        auto& bv = broadcasters[topic];
        for (auto* listener : v) {
            for (auto& ruleset : bv) {
                ruleset->apply(listener);
            }
        }

        v.clear(); // Clear the listener vector for this topic after processing
    });

    // Cleanup: Clear all broadcaster vectors
    broadcasters.forall([&](auto& v) {
        v.clear();
    });
}

} // namespace Nebulite::Data::BroadcastListenContainer
