/**
 * @file FlatContainer.hpp
 * @brief Contains the definition of the Flat Broadcast-listen container model.
 */

#ifndef NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_FLAT_CONTAINER_HPP
#define NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_FLAT_CONTAINER_HPP


//------------------------------------------
// Includes

// Nebulite
#include "Data/BroadcastListenContainer/BaseContainer.hpp"
#include "Data/Map/HotStringKeyMap.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

class FlatContainer final : public BaseContainer {
public:
    explicit FlatContainer(std::atomic<bool>& stopFlag, uint32_t const& workerIndex, uint32_t const& workerCount) : BaseContainer(stopFlag, workerIndex, workerCount) {
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
     * @param listener The listener to add.
     */
    void listen(std::shared_ptr<Interaction::Rules::Listener> const& listener) override ;

    //------------------------------------------
    // Worker Thread Methods

    /**
     * @brief Prepares for the next frame by swapping the current and next frame containers.
     */
    void prepare() override ;

private:
    static auto constexpr broadcasterSpreading = 16;
    std::array<HotStringKeyMap<std::vector<std::shared_ptr<Interaction::Rules::Ruleset>>>,broadcasterSpreading> broadcasters;
    HotStringKeyMap<std::vector<std::shared_ptr<Interaction::Rules::Listener>>> listeners;

    /**
     * @brief Processes all broadcast-listen pairs.
     */
    void process() override ;

    mutable Utility::SharedMutex mutex;
};

} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_FLAT_CONTAINER_HPP
