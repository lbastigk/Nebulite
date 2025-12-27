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
#include "Data/PairingContainer.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Data{

/**
 * @brief Class to manage broadcast-listen pairs of rulesets. With this and next frame management.
 */
class BroadCastListenPairs {
public:
    BroadCastListenPairs(std::atomic<bool>& stopFlag) : threadState{ .stopFlag = stopFlag }{
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

    //------------------------------------------
    // Container Methods

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry);

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listening domain.
     * @param topic The topic to listen for.
     * @param listenerId The unique ID of the listener render object.
     */
    void listen(Interaction::Execution::DomainBase* listener, std::string const& topic, uint32_t const& listenerId);

    //------------------------------------------
    // Worker Thread Methods

    /**
     * @brief Prepares for the next frame by swapping the current and next frame containers.
     */
    void prepare();

    /**
     * @brief Notifies the worker thread to start processing.
     */
    void startWork();

    /**
     * @brief Waits for the worker thread to finish processing.
     */
    void waitForWorkFinished() const ;

private:
    Data::PairingContainer thisFrame;
    Data::PairingContainer nextFrame;

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
