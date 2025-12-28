#ifndef NEBULITE_DATA_PAIRING_CONTAINER_HPP
#define NEBULITE_DATA_PAIRING_CONTAINER_HPP

/**
 * @define USE_BYTETREE_CONTAINER
 * @brief Define to use ByteTree as the internal container for listeners on rulesets.
 *        Set to 0 to use Abseil's node_hash_map instead.
 * @details Using ByteTree is a work in progress, and currently about half fast as the hash map.
 */
#define USE_BYTETREE_CONTAINER 0

//------------------------------------------
// Includes

// Standard library
#include <mutex>

// External
#include "absl/container/node_hash_map.h"

// Nebulite
#include "Data/ByteTree.hpp"
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
    Interaction::Execution::DomainBase* contextOther = nullptr; // The domain that listened to the Broadcast
#if USE_BYTETREE_CONTAINER
    // Apply function
    void apply() {
        entry->apply(contextOther);
    }
#else
    bool active = false; // If false, this pair is skipped during update

    // Apply function
    void apply() {
        if (active) {
            entry->apply(contextOther);
            active = false;
        }
    }
#endif // USE_BYTETREE_CONTAINER
};

class ListenersOnRuleset {
#if USE_BYTETREE_CONTAINER
private:
    Data::ByteTree<BroadCastListenPair> listeners;

public:
    std::shared_ptr<Interaction::Rules::Ruleset> entry;

    void cleanup() {
        listeners.cleanup();
    }

    void apply() {
        listeners.apply();
    }

    void insert(uint32_t const& id, BroadCastListenPair const& pair) {
        auto pairPtr = listeners.at(id);

        // We only need to copy if the existing entry is invalid
        if (!pairPtr->entry) {
            pairPtr->entry = pair.entry;
            pairPtr->contextOther = pair.contextOther;
        }
    }

#else
private:
    mutable std::shared_mutex mutex;

    absl::node_hash_map<uint32_t, BroadCastListenPair> listeners; // id_other -> BroadCastListenPair

public:
    ListenersOnRuleset() = default;
    ~ListenersOnRuleset() = default;

    ListenersOnRuleset(ListenersOnRuleset const& other) {
        std::shared_lock<std::shared_mutex> slock(other.mutex);
        listeners = other.listeners;
        entry = other.entry;
    }

    ListenersOnRuleset& operator=(ListenersOnRuleset const& other) {
        if (this == &other) return *this;
        std::scoped_lock lock(mutex, other.mutex);
        listeners = other.listeners;
        entry = other.entry;
        return *this;
    }

    std::shared_ptr<Interaction::Rules::Ruleset> entry;

    void cleanup() {
        // Thread-local random generator for probabilistic cleanup
        thread_local std::mt19937 cleanup_rng(std::random_device{}());
        thread_local std::uniform_int_distribution<int> cleanup_dist(0, 99); // uniform, avoids modulo bias
        std::unique_lock<std::shared_mutex> ulock(mutex);
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

    void apply() {
        std::shared_lock<std::shared_mutex> slock(mutex);
        for (auto & it : listeners) {
            it.second.apply();
        }
    }

    void insert(uint32_t const& id, BroadCastListenPair const& pair) {
        std::unique_lock<std::shared_mutex> ulock(mutex);
        listeners[id] = pair;
    }
#endif // USE_BYTETREE_CONTAINER
};

struct OnTopicFromId {
    bool active = false; // If false, this is skipped during update
    absl::flat_hash_map<uint32_t, ListenersOnRuleset> rulesets; // idx_ruleset -> ListenersOnRuleset
};

class PairingContainer {
public:
    PairingContainer() = default;
    ~PairingContainer() = default;

    void insertBroadcaster(std::shared_ptr<Interaction::Rules::Ruleset> const& entry);
    void insertListener(Interaction::Execution::DomainBase* listener, std::string const& topic, uint32_t const& listenerId);
    void process(); // Worker thread processing function
    std::unique_lock<std::shared_mutex> lock() {return std::unique_lock<std::shared_mutex>(mutex);}

private:
    // Using a node hash map to keep iterators stable during insertions
    absl::node_hash_map<
        std::string,            // The topic of the broadcasted entry. TODO: Consider a rolling id system so we can simplify this key
        absl::node_hash_map<
            uint32_t,           // The ID of self.
            OnTopicFromId       // The struct containing active flag and rulesets
        >
    > data;
    mutable std::shared_mutex mutex; // Mutex for thread-safe access
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_PAIRING_CONTAINER_HPP
