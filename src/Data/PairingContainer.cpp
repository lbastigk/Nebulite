#include "Data/PairingContainer.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Data {

void PairingContainer::insertBroadcaster(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    auto const topic = entry->getTopic();
    auto const id = entry->getId();
    auto const index = entry->getIndex();
    auto& [isActive, rulesets] = data[topic][id]; // creates maps/entries if missing
    rulesets[index].entry = entry;
    isActive = true;
}

void PairingContainer::insertListener(Interaction::Execution::DomainBase* listener, std::string const& topic, uint32_t const& listenerId) {
    // Check if any object has broadcasted on this topic
    auto topicIt = data.find(topic);
    if (topicIt == data.end()) {
        return; // No entries for this topic in this thread
    }

    for (auto& [id_self, onTopicFromId] : topicIt->second) {
        // Skip if broadcaster and listener are the same object
        if (id_self == listenerId)
            continue;

        // Skip if inactive
        if (!onTopicFromId.active)
            continue;

        // For all rulesets under this broadcaster and topic
        for (auto& listenersOnRuleset : std::ranges::views::values(onTopicFromId.rulesets)) {
#if USE_BYTETREE_CONTAINER
            if (listenersOnRuleset.entry->evaluateCondition(listener)) {
                auto blp = BroadCastListenPair{
                    listenersOnRuleset.entry,
                    listener
                };
                listenersOnRuleset.insert(listenerId, blp);
            }
#else
            auto blp = BroadCastListenPair{
                listenersOnRuleset.entry,
                listener,
                listenersOnRuleset.entry->evaluateCondition(listener)
            };
            listenersOnRuleset.insert(listenerId, blp);
#endif // USE_BYTETREE_CONTAINER
        }
    }
}

void PairingContainer::process() {
    for (auto& map_other : std::views::values(data)) {
        for (auto& [isActive, rulesets] : std::views::values(map_other)) {
            if (!isActive)
                continue;
            for (auto& listenersOnRuleset : std::ranges::views::values(rulesets)) {
                // Probabilistic cleanup performed once per ruleset
                listenersOnRuleset.cleanup();

                // Process active listeners (single pass, no erases here)
                listenersOnRuleset.apply();
            }
            // Reset activity flag, must be activated on broadcast
            isActive = false;
        }
    }
}

} // namespace Nebulite::Data
