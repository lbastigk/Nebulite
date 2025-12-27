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

void PairingContainer::swap(PairingContainer& other) {
    std::scoped_lock lock(mutex, other.mutex);
    data.swap(other.data);
}

void PairingContainer::process() {
    // Thread-local random generator for probabilistic cleanup
    thread_local std::mt19937 cleanup_rng(std::random_device{}());
    thread_local std::uniform_int_distribution<int> cleanup_dist(0, 99); // uniform, avoids modulo bias

    auto lock = this->lock();

    if (stopFlag)
        return;

    // Actual processing of thisFrame
    for (auto& map_other : std::views::values(data)) {
        for (auto& [isActive, rulesets] : std::views::values(map_other)) {
            if (!isActive)
                continue;
            for (auto& [entry, listeners] : std::ranges::views::values(rulesets)) {
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

                // Process active listeners (single pass, no erases here)
                for (auto & it : listeners) {
                    auto &pair = it.second;
                    if (pair.active) {
                        pair.entry->apply(pair.contextOther);
                        pair.active = false;
                    }
                }
            }
            // Reset activity flag, must be activated on broadcast
            isActive = false;
        }
    }
}


} // namespace Nebulite::Data
