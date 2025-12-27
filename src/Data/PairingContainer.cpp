#include "Data/PairingContainer.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Data {

void PairingContainer::insertBroadcaster(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    std::scoped_lock lock(mutex);
    auto& [isActive, rulesets] = data[entry->getTopic()][entry->getId()];
    rulesets[entry->getIndex()].entry = entry;
    isActive = true;
}

void PairingContainer::insertListener(Interaction::Execution::DomainBase* listener, std::string const& topic, uint32_t const& listenerId) {
    // Lock to safely read from broadcasted.entriesThisFrame
    std::scoped_lock broadcastLock(mutex);

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
        for (auto& [entry, listeners] : std::ranges::views::values(onTopicFromId.rulesets)) {
            listeners[listenerId] = BroadCastListenPair{
                entry,
                listener,
                entry->evaluateCondition(listener)
            };
        }
    }
}



} // namespace Nebulite::Data
