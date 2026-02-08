/**
 * @file MapContainer.hpp
 * @brief Map-Based Broadcast-listening container for rulesets.
 */

#ifndef NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_MAP_CONTAINER_HPP
#define NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_MAP_CONTAINER_HPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <random>

// Nebulite
#include "Data/BroadcastListenContainer/BaseContainer.hpp"
#include "Data/Map/HotStringKeyMap.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

/**
 * @class MapContainer
 * @brief Map-based implementation of the broadcast-listen container for rulesets.
 *        Uses maps to store listeners and broadcasters, allowing for efficient lookups and insertions.
 */
class MapContainer final : public BaseContainer {
public:
    explicit MapContainer(std::atomic<bool>& stopFlag)
        : BaseContainer(stopFlag),
          thisFrame(std::make_unique<MapPairingContainer>()),
          nextFrame(std::make_unique<MapPairingContainer>())
    {
        initializeWorkerThread();
    }

    ~MapContainer() override {
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
    struct BroadCastListenPair {
        std::shared_ptr<Interaction::Rules::Ruleset> entry; // The Ruleset that was broadcasted
        Interaction::Execution::Domain* contextOther = nullptr; // The domain that listened to the Broadcast
        // TODO: Possible to use std::future<bool> here to check only if needed?
        bool active = false; // If false, this pair is skipped during update

        // Apply function: Hash map version needs to check active flag
        void apply() {
            if (active) {
                entry->apply(contextOther);
                active = false;
            }
        }
    };

    class ListenersOnRuleset {
        mutable Utility::SharedMutex mutex;

        absl::node_hash_map<uint32_t, BroadCastListenPair> listeners; // id_other -> BroadCastListenPair

    public:
        ListenersOnRuleset() = default;
        ~ListenersOnRuleset() = default;

        ListenersOnRuleset(ListenersOnRuleset const& other) {
            Utility::ReadLock slock(other.mutex); // lock other for reading
            listeners = other.listeners;
            entry = other.entry;
        }

        ListenersOnRuleset& operator=(ListenersOnRuleset const& other) {
            if (this == &other) return *this;
            if (&mutex < &other.mutex) {
                Utility::WriteLock writeLock(mutex);
                Utility::ReadLock readLock(other.mutex);
                listeners = other.listeners;
                entry = other.entry;
            } else if (&mutex > &other.mutex) {
                Utility::ReadLock readLock(other.mutex);
                Utility::WriteLock writeLock(mutex);
                listeners = other.listeners;
                entry = other.entry;
            } else {
                // fallback (shouldn't happen unless aliases point to same underlying mutex)
                Utility::WriteLock writeLock(mutex);
                listeners = other.listeners;
                entry = other.entry;
            }
            return *this;
        }

        std::shared_ptr<Interaction::Rules::Ruleset> entry;

        void cleanup() {
            // Thread-local random generator for probabilistic cleanup
            thread_local std::mt19937 cleanup_rng(std::random_device{}());
            thread_local std::uniform_int_distribution cleanup_dist(0, 99); // uniform, avoids modulo bias
            Utility::WriteLock writeLock(mutex);
            if (cleanup_dist(cleanup_rng) == 0) {
                for (auto it = listeners.begin(); it != listeners.end();) {
                    if (!it->second.active) {
                        auto const itToErase = it++;
                        listeners.erase(itToErase); // erase returns void in Abseil
                    } else {
                        ++it;
                    }
                }
            }
        }

        void apply() {
            Utility::ReadLock readLock(mutex);
            for (auto& pair : std::views::values(listeners)) {
                pair.apply();
            }
        }

        void insert(uint32_t const& id, BroadCastListenPair const& pair) {
            Utility::WriteLock writeLock(mutex);
            listeners[id] = pair;
        }
    };

    struct OnTopicFromId {
        bool active = false; // If false, this is skipped during update
        absl::flat_hash_map<uint32_t, ListenersOnRuleset> rulesets; // idx_ruleset -> ListenersOnRuleset
    };

    class MapPairingContainer {
    public:
        MapPairingContainer() = default;
        ~MapPairingContainer() = default;

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

    std::unique_ptr<MapPairingContainer> thisFrame;
    std::unique_ptr<MapPairingContainer> nextFrame;

    /**
     * @brief Processes all broadcast-listen pairs.
     */
    void process() override ;
};
} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_MAP_CONTAINER_HPP
