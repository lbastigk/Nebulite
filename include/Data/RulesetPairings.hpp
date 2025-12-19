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
#include <atomic>
#include <condition_variable>

// External
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Data{

// TODO: Break into a chain of nibble-trees for faster access?
//       uint32_t -> 8 nibble traversal. vector<vector<vector<vector<...<OnTopicFromId>...>>>?
//       Should be WAY faster for lookups, easy cleanup as well.
//       Byte-Tree should be fine as well, just 4 levels instead of 8.
//       Another option would be to insert all pairs into a flat vector and iterate through that.
//       Meaning we store generated pairs, and insert into a vector if listening was successful.
class BroadCastListenPairs {
public:

    BroadCastListenPairs(std::atomic<bool>& stopFlag) : threadState{ .stopFlag = stopFlag } {
        // Start worker thread
        workerThread = std::thread([this] {
            this->process();
        });
    }

    ~BroadCastListenPairs() {
        // Signal thread to stop
        threadState.stopFlag = true;
        threadState.condition.notify_one();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param obj The render object to check.
     * @param topic The topic to listen for.
     * @param listenerId The unique ID of the listener render object.
     */
    void listen(Core::RenderObject* obj, std::string const& topic, uint32_t const& listenerId);

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry);

    /**
     * @brief Prepares for the next frame by swapping the current and next frame containers.
     */
    void prepare() {
        std::scoped_lock lock(mutexThisFrame, mutexNextFrame);
        std::swap(thisFrame, nextFrame);
    }

    /**
     * @brief Notifies the worker thread to start processing.
     */
    void startWork() {
        threadState.workReady = true;
        threadState.workFinished = false;
        threadState.condition.notify_one();
    }

    /**
     * @brief Waits for the worker thread to finish processing.
     */
    void waitForWorkFinished() {
        while (!threadState.workFinished.load()) {
            std::this_thread::yield();
        }
    }

private:
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

    using PairingContainer = absl::flat_hash_map<
        std::string,            // The topic of the broadcasted entry
        absl::flat_hash_map<
            uint32_t,           // The ID of self.
            OnTopicFromId       // The struct containing active flag and rulesets
        >
    >;

    PairingContainer thisFrame;
    PairingContainer nextFrame;

    mutable std::mutex mutexThisFrame; // for read/write access to the container
    mutable std::mutex mutexNextFrame; // for read/write access to the container

    // Threading variables
    struct ThreadState {
        /**
         * @brief Condition variables for thread synchronization.
         */
        std::condition_variable condition = {};

        /**
         * @brief Flags to indicate when work is ready for each thread.
         */
        std::atomic<bool> workReady = false;

        /**
         * @brief Flags to indicate when work is finished for each thread.
         */
        std::atomic<bool> workFinished = false;

        /**
         * @brief Flag to signal threads to stop.
         */
        std::atomic<bool>& stopFlag;
    } threadState;

    std::thread workerThread;

    /**
     * @brief Processes all broadcast-listen pairs.
     */
    void process();
};

} // namespace Nebulite::Data
#endif // NEBULITE_INTERACTION_RULES_RULESET_PAIRINGS_HPP