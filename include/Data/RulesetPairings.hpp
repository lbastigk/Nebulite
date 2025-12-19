/**
 * @file RulesetPairings.hpp
 * @brief This file contains the declaration of the RulesetPairings class, which is responsible for managing
 *        the pairing of broadcasted and listened rulesets in the Nebulite engine.
 */

#ifndef NEBULITE_INTERACTION_RULES_RULESET_PAIRINGS_HPP
#define NEBULITE_INTERACTION_RULES_RULESET_PAIRINGS_HPP

//------------------------------------------
// Includes

// Standard library

// Nebulite
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Data{

/**
 * @struct BroadCastListenPair
 * @brief Structure to hold a broadcast-listen pair.
 */
struct BroadCastListenPair {
    std::shared_ptr<Interaction::Rules::Ruleset> entry; // The Ruleset that was broadcasted
    Core::RenderObject* contextOther; // The object that listened to the Broadcast
    bool active = true; // If false, this pair is skipped during update
};

struct ListenersOnRuleset {
    std::shared_ptr<Interaction::Rules::Ruleset> entry;
    absl::flat_hash_map<uint32_t, BroadCastListenPair> listeners; // id_other -> BroadCastListenPair
};

struct OnTopicFromId {
    bool active = false; // If false, this is skipped during update
    absl::flat_hash_map<uint32_t, ListenersOnRuleset> rulesets; // idx_ruleset -> ListenersOnRuleset
};

// TODO: Break into a chain of nibble-trees for faster access?
//       uint32_t -> 8 nibble traversal. vector<vector<vector<vector<...<OnTopicFromId>...>>>?
//       Should be WAY faster for lookups, easy cleanup as well.
//       Byte-Tree should be fine as well, just 4 levels instead of 8.
//       Make both versions with the same interface and benchmark them!
//       Should only need an operator[] and an erase() method,
//       as well as an iterator?
class BroadCastListenPairs {
public:
    // TODO: Move thread states into here for better encapsulation?
    //       - explicit constructor with stopFlag reference
    //       - workRead and workFinished per thread, inside class
    //       - condition variable inside class
    //       - proper destructor so we dont forget to join threads
    //       Make members private, only expose:
    //       - process() method
    //       - broadcast()/listen() methods that lock the mutex internally
    //       Modify process so it includes all necessary locking and condition variable handling
    //       Essentially moving all code from Invoke into here

    void process() {
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

    absl::flat_hash_map<
        std::string,            // The topic of the broadcasted entry
        absl::flat_hash_map<
            uint32_t,           // The ID of self.
            OnTopicFromId       // The struct containing active flag and rulesets
        >
    > container;
    mutable std::mutex mutex; // for read/write access to the container
};

} // namespace Nebulite::Data
#endif // NEBULITE_INTERACTION_RULES_RULESET_PAIRINGS_HPP