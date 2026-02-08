/**
 * @file FlatContainer.hpp
 * @brief Contains the definition of the Flat Broadcast-listen container model.
 */

#ifndef NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_FLAT_CONTAINER_HPP
#define NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_FLAT_CONTAINER_HPP

#include "Data/BroadcastListenContainer/BaseContainer.hpp"
#include "Data/Map/HotStringKeyMap.hpp"
#include "Interaction/Rules/Ruleset.hpp"

namespace Nebulite::Data::BroadcastListenContainer {

class FlatContainer final : public BaseContainer {
public:
    explicit FlatContainer(std::atomic<bool>& stopFlag) : BaseContainer(stopFlag) {
        initializeWorkerThread();
    }

    ~FlatContainer() override {
        stopWorkerThread();
    }

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) override ;

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listening domain.
     * @param topic The topic to listen for.
     * @param listenerId The unique ID of the listener render object.
     */
    void listen(Interaction::Execution::Domain& listener, std::string const& topic, uint32_t const& listenerId) override ;

    //------------------------------------------
    // Worker Thread Methods

    /**
     * @brief Prepares for the next frame by swapping the current and next frame containers.
     */
    void prepare() override ;

private:
    // TODO: since listeners is accessed by multiple threads, perhaps an array of map-vector pairs is better?
    //       with access based on worker thread ID?

    // TODO: Find some way to introduce randomness in order of listeners, as this container is the same for all threads
    //       meaning we may get some unwanted clinches, where all threads try to access the same listener
    //       Perhaps some percentual offset in where each thread starts iterating through the vector of listeners?
    //       offset = workerIndex * (vector.size() / THREADRUNNER_COUNT)
    //       idx = (i + workerIndex) % vector.size()
    //       Add workedIndex to BaseContainer class so this is possible

    struct ListenerEntry {
        Interaction::Execution::Domain* domain;
        uint32_t id;
    };

    HotStringKeyMap<std::vector<std::shared_ptr<Interaction::Rules::Ruleset>>> broadcasters;
    HotStringKeyMap<std::vector<ListenerEntry>> listeners;

    /**
     * @brief Processes all broadcast-listen pairs.
     */
    void process() override ;

    mutable Utility::SharedMutex mutex;


};

} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_FLAT_CONTAINER_HPP
