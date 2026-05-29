/**
 * @file BaseContainer.hpp
 * @brief Definition of the virtual base class for broadcast-listen containers for rulesets.
 */

#ifndef DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_HPP
#define DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_HPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Utility/Coordination/WorkDispatcher.hpp"

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
 * @tparam DerivedContainer The type of the derived container class, required as argument of the WorkDispatcher to use as workspace reference.
 */
template<typename DerivedContainer>
class BaseContainer {
public:
    explicit BaseContainer(std::atomic<bool>& stopFlag, size_t const& workerIndex, size_t const& workerCount, DerivedContainer container)
        : workerInfo{workerIndex, workerCount}
        , dispatcher(stopFlag, processImpl, initImpl)
    {
        dispatcher.workspace = container;
        ensureEarlyThreadId();
    }

    virtual ~BaseContainer() = default;

    BaseContainer(const BaseContainer&) = delete;
    BaseContainer& operator=(const BaseContainer&) = delete;
    BaseContainer(BaseContainer&&) = delete;
    BaseContainer& operator=(BaseContainer&&) = delete;

    //------------------------------------------
    // Worker thread access

    void startWork() {
        dispatcher.startWork();
    }

    void waitForWorkFinished() {
        dispatcher.waitForWorkFinished();
    }

    //------------------------------------------
    // Container Methods to be implemented by derived classes

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    virtual void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
        (void)entry;
    }

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listener to add.
     */
    virtual void listen(std::shared_ptr<Interaction::Rules::Listener> const& listener) {
        (void)listener;
    }

    /**
     * @brief Prepare container for next processing round.
     */
    virtual void prepare() {}

    // non-static hooks for derived classes to implement
    virtual void init() {}
    virtual void process() {}

protected:
    struct WorkerInfo {
        size_t index;
        size_t count;
    } workerInfo;

private:
    // static wrappers for WorkDispatcher
    static void initImpl(DerivedContainer container) { container->init(); }
    static void processImpl(DerivedContainer container) { container->process(); }

    static void ensureEarlyThreadId() {
        thread_local bool threadIdAssigned = false;
        if (threadIdAssigned) return;
        if (size_t const id = JsonScope::assignThreadIndex(); id >= JsonScope::noLockArraySize) {
            throw std::runtime_error("Thread ID exceeds non-locking array size!");
        }
        threadIdAssigned = true;
    }

    // store WorkDispatcher with container reference
    Utility::Coordination::WorkDispatcher<DerivedContainer> dispatcher;
};
} // namespace Nebulite::Data::BroadcastListenContainer
#endif // DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_HPP
