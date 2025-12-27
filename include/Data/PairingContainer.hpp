#ifndef NEBULITE_DATA_PAIRING_CONTAINER_HPP
#define NEBULITE_DATA_PAIRING_CONTAINER_HPP

//------------------------------------------
// Includes

// Standard library
#include <mutex>

// External
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Data {

/**
 * @struct BroadCastListenPair
 * @brief Structure to hold a broadcast-listen pair.
 */
struct BroadCastListenPair {
    std::shared_ptr<Interaction::Rules::Ruleset> entry; // The Ruleset that was broadcasted
    Interaction::Execution::DomainBase* contextOther; // The domain that listened to the Broadcast
    bool active = true; // If false, this pair is skipped during update

    [[nodiscard]] bool isActive() const {
        return active;
    }
};

struct ListenersOnRuleset {
    std::shared_ptr<Interaction::Rules::Ruleset> entry;
    absl::flat_hash_map<uint32_t, BroadCastListenPair> listeners; // id_other -> BroadCastListenPair
};

struct OnTopicFromId {
    bool active = false; // If false, this is skipped during update
    absl::flat_hash_map<uint32_t, ListenersOnRuleset> rulesets; // idx_ruleset -> ListenersOnRuleset

    [[nodiscard]] bool isActive() const {
        return active;
    }
};

struct PairingContainer {
    void insertBroadcaster(std::shared_ptr<Interaction::Rules::Ruleset> const& entry);
    void insertListener(Interaction::Execution::DomainBase* listener, std::string const& topic, uint32_t const& listenerId);

    PairingContainer() = default;

    absl::flat_hash_map<
        std::string,            // The topic of the broadcasted entry
        absl::flat_hash_map<
            uint32_t,           // The ID of self.
            OnTopicFromId       // The struct containing active flag and rulesets
        >
    > data;

    std::mutex mutex;
};

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_PAIRING_CONTAINER_HPP

