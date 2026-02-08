/**
 * @file TreeContainer.hpp
 * @brief Tree-Based Broadcast-listening container for rulesets.
 */

#ifndef NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_TREE_CONTAINER_HPP
#define NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_TREE_CONTAINER_HPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <memory>

// External
#include "absl/container/node_hash_map.h"

// Nebulite
#include "Data/BroadcastListenContainer/BaseContainer.hpp"
#include "Data/Tree/ByteTree.hpp"
#include "Data/Map/HotStringKeyMap.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/SharedMutex.hpp"

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

struct TreeBroadCastListenPair {
    std::shared_ptr<Interaction::Rules::Ruleset> entry; // The Ruleset that was broadcasted
    Interaction::Execution::Domain* contextOther = nullptr; // The domain that listened to the Broadcast
    // Apply function: ByteTree keeps track of active entries internally
    void apply() const {
        entry->apply(contextOther);
    }
};

/**
 * @class TreeContainer
 * @brief Tree-based implementation of the broadcast-listen container for rulesets.
 *        Uses Trees to store listeners and broadcasters, allowing for efficient lookups and insertions.
 */
class TreeContainer final : public BaseContainer {
public:
    explicit TreeContainer(std::atomic<bool>& stopFlag)
        : BaseContainer(stopFlag),
          thisFrame(std::make_unique<TreePairingContainer>()),
          nextFrame(std::make_unique<TreePairingContainer>())
    {
        initializeWorkerThread();
    }

    ~TreeContainer() override {
        stopWorkerThread();
    }

    //------------------------------------------
    // Container Methods

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) const override ;

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listening domain.
     * @param topic The topic to listen for.
     * @param listenerId The unique ID of the listener render object.
     */
    void listen(Interaction::Execution::Domain& listener, std::string const& topic, uint32_t const& listenerId) const override ;

    //------------------------------------------
    // Worker Thread Methods

    /**
     * @brief Prepares for the next frame by swapping the current and next frame containers.
     */
    void prepare() override ;

private:

    class ListenersOnRuleset{
        ByteTree<TreeBroadCastListenPair> listeners;

    public:
        std::shared_ptr<Interaction::Rules::Ruleset> entry;

        void cleanup() {
            listeners.cleanup();
        }

        void apply() {
            listeners.apply();
        }

        void insert(uint32_t const& id, TreeBroadCastListenPair const& pair) {
            // We only need to copy if the existing entry is invalid
            if (auto const& pairPtr = listeners.at(id); !pairPtr->entry) {
                pairPtr->entry = pair.entry;
                pairPtr->contextOther = pair.contextOther;
            }
        }
    };

    struct OnTopicFromId {
        bool active = false; // If false, this is skipped during update
        absl::flat_hash_map<uint32_t, ListenersOnRuleset> rulesets; // idx_ruleset -> ListenersOnRuleset
    };

    class TreePairingContainer {
    public:
        TreePairingContainer() = default;
        ~TreePairingContainer() = default;

        void insertBroadcaster(std::shared_ptr<Interaction::Rules::Ruleset> const& entry);
        void insertListener(Interaction::Execution::Domain& listener, std::string const& topic, uint32_t const& listenerId);
        void process(); // Worker thread processing function

        /**
         * @brief Required for processing to start working
         * @return A unique lock on the internal mutex
         */
        Utility::SharedLock lock() const {
            return Utility::SharedLock(mutex);
        }

    private:
        struct idToMap {
            absl::node_hash_map<
                uint32_t,           // The ID of self.
                OnTopicFromId       // The struct containing active flag and rulesets
            > map;
            mutable Utility::SharedMutex mutex; // Mutex for thread-safe access
        };

        // HotStringKeyMap seems to be minimally faster than a standard hash map for multiple topics
        HotStringKeyMap<idToMap> data;

        // Mutex for thread-safe access
        mutable Utility::SharedMutex mutex;
    };

    std::unique_ptr<TreePairingContainer> thisFrame;
    std::unique_ptr<TreePairingContainer> nextFrame;

    /**
     * @brief Processes all broadcast-listen pairs.
     */
    void process() override ;
};
} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_TREE_CONTAINER_HPP
