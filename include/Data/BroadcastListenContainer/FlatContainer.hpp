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

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Rules {
class Ruleset;
struct Listener;
} // namespace Nebulite::Interaction::Rules

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

class FlatContainer final : public BaseContainer<FlatContainer*> {
public:
    explicit FlatContainer(std::atomic<bool>& stopFlag, uint32_t const& workerIndex, uint32_t const& workerCount)
        : BaseContainer(stopFlag, workerIndex, workerCount, this) {
    }

    ~FlatContainer() override = default;

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

    void prepare() override {}

    void process() override;
    void init() override;
private:
    static auto constexpr broadcasterSpreading = 2*BATCH_WORKER_COUNT;
    static auto constexpr listenerSpreading = 2*BATCH_WORKER_COUNT;

    std::array<HotStringKeyMap<std::vector<std::shared_ptr<Interaction::Rules::Ruleset>>>,broadcasterSpreading> broadcasters;
    std::array<HotStringKeyMap<std::vector<std::shared_ptr<Interaction::Rules::Listener>>>, listenerSpreading> listeners;
    mutable Utility::SharedMutex mutex;
};

} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_FLAT_CONTAINER_HPP
