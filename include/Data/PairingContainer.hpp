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

    void cleanup() {
        // Thread-local random generator for probabilistic cleanup
        thread_local std::mt19937 cleanup_rng(std::random_device{}());
        thread_local std::uniform_int_distribution<int> cleanup_dist(0, 99); // uniform, avoids modulo bias
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
};

struct OnTopicFromId {
    bool active = false; // If false, this is skipped during update
    absl::flat_hash_map<uint32_t, ListenersOnRuleset> rulesets; // idx_ruleset -> ListenersOnRuleset

    [[nodiscard]] bool isActive() const {
        return active;
    }
};

class PairingContainer {
public:
    void insertBroadcaster(std::shared_ptr<Interaction::Rules::Ruleset> const& entry);
    void insertListener(Interaction::Execution::DomainBase* listener, std::string const& topic, uint32_t const& listenerId);

    void swap(PairingContainer& other);

    void process(); // Worker thread processing function

    PairingContainer() = default;
    ~PairingContainer() = default;

    std::unique_lock<std::mutex> lock() {
        return std::unique_lock<std::mutex>(mutex);
    }

private:
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

