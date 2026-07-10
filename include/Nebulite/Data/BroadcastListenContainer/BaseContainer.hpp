#ifndef NEBULITE_DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_HPP
#define NEBULITE_DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_HPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <cstddef>
#include <memory>

// Nebulite
#include "Nebulite/Utility/Coordination/WorkDispatcher.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Rules {
class Ruleset;
struct Listener;
} // namespace Nebulite::Interaction::Rules

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {
/**
 * @brief Interface Class to manage broadcast-listen pairs of rulesets.
 * @details If a hashmap is used in the implementation, using Data::MapType is recommended.
 * @tparam DerivedContainer The type of the derived container class, required as argument of the WorkDispatcher to use as workspace reference.
 */
template<typename DerivedContainer>
class BaseContainer {
public:
    explicit BaseContainer(std::atomic<bool>& stopFlag, std::size_t workerIndex, std::size_t workerCount, DerivedContainer container);

    virtual ~BaseContainer();

    BaseContainer(BaseContainer const&) = delete;
    BaseContainer& operator=(BaseContainer const&) = delete;
    BaseContainer(BaseContainer&&) = delete;
    BaseContainer& operator=(BaseContainer&&) = delete;

    //------------------------------------------
    // Worker thread access

    void startWork();

    void waitForWorkFinished();

    //------------------------------------------
    // Container Methods to be implemented by derived classes

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    virtual void broadcast(std::shared_ptr<Interaction::Rules::Ruleset>&& entry);

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listener to add.
     */
    virtual void listen(std::shared_ptr<Interaction::Rules::Listener>&& listener);

    /**
     * @brief Prepare container for next processing round.
     */
    virtual void prepare();

    // non-static hooks for derived classes to implement
    virtual void init();
    virtual void process();

protected:
    struct WorkerInfo {
        std::size_t index;
        std::size_t count;
    } workerInfo;

private:
    // static wrappers for WorkDispatcher
    static void initImpl(DerivedContainer container);
    static void processImpl(DerivedContainer container);

    static void verifyCacheLookupIndex();

    // store WorkDispatcher with container reference
    Utility::Coordination::WorkDispatcher<DerivedContainer> dispatcher;
};
} // namespace Nebulite::Data::BroadcastListenContainer
#include "Nebulite/Data/BroadcastListenContainer/BaseContainer.tpp" // NOLINT
#endif // NEBULITE_DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_HPP
