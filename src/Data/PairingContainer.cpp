#include "Data/PairingContainer.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Data {

void PairingContainer::insertBroadcaster(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    auto& topic = entry->getTopic();
    auto& id = entry->getId();
    auto& index = entry->getIndex();
    auto& [map, mtx] = data[topic]; // creates map if missing
    auto const lock = std::unique_lock<std::shared_mutex>(mtx);
    auto& [isActive, rulesets] = map[id]; // creates maps/entries if missing
    rulesets[index].entry = entry;
    isActive = true;
}

void PairingContainer::insertListener(Interaction::Execution::DomainBase& listener, std::string const& topic, uint32_t const& listenerId) {
    // Check if any object has broadcasted on this topic
    auto const [it, isValid] = data.find(topic);
    if (!isValid) {
        return; // No entries for this topic in this thread
    }

    auto const lock = std::shared_lock<std::shared_mutex>(it->second.mutex);
    for (auto& [id_self, onTopicFromId] : it->second.map) {
        // Skip if broadcaster and listener are the same object, or if the broadcaster is inactive
        if (id_self == listenerId || !onTopicFromId.active)
            continue;

        // For all rulesets under this broadcaster and topic
        for (auto& listenersOnRuleset : std::ranges::views::values(onTopicFromId.rulesets)) {
#if USE_BYTETREE_CONTAINER
            // Insert in this frame
            if (listenersOnRuleset.entry->evaluateCondition(&listener)) {
                auto blp = BroadCastListenPair{
                    listenersOnRuleset.entry,
                    &listener
                };
                listenersOnRuleset.insert(listenerId, blp);
            }
#else
            auto blp = BroadCastListenPair{
                listenersOnRuleset.entry,
                &listener,
                listenersOnRuleset.entry->evaluateCondition(&listener)
            };
            listenersOnRuleset.insert(listenerId, blp);
#endif // USE_BYTETREE_CONTAINER
        }
    }
}

void PairingContainer::process() {
    for (const auto& map : data.getMaps()) {
        for (auto& map_other : std::views::values(map)) {
            auto const& lock = std::shared_lock<std::shared_mutex>(map_other.mutex);
            for (auto& [isActive, rulesets] : std::views::values(map_other.map)) {
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
}

} // namespace Nebulite::Data
