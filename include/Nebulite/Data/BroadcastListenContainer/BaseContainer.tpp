#ifndef NEBULITE_DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_TPP
#define NEBULITE_DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_TPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_HPP
#include "Nebulite/Data/BroadcastListenContainer/BaseContainer.hpp"
#endif // NEBULITE_DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_HPP

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

template<typename DerivedContainer>
BaseContainer<DerivedContainer>::BaseContainer(std::atomic<bool>& stopFlag, std::size_t const workerIndex, std::size_t const workerCount, DerivedContainer container)
        : workerInfo{workerIndex, workerCount}
, dispatcher(stopFlag, processImpl, initImpl)
{
    dispatcher.workspace = container;
    verifyCacheLookupIndex();
}

template<typename DerivedContainer>
BaseContainer<DerivedContainer>::~BaseContainer() = default;

//------------------------------------------
// Worker thread access

template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::startWork() {
    dispatcher.startWork();
}

template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::waitForWorkFinished() {
    dispatcher.waitForWorkFinished();
}

//------------------------------------------
// Container Methods to be implemented by derived classes

/**
 * @brief Broadcasts a ruleset to all listeners on its topic.
 * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
 */
template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::broadcast(std::shared_ptr<Interaction::Rules::Ruleset>&& entry) {
    // explicit consumption
    auto local = std::move(entry); // NOLINT
}

/**
 * @brief Listens for rulesets on a specific topic.
 * @param listener The listener to add.
 */
template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::listen(std::shared_ptr<Interaction::Rules::Listener>&& listener) {
    // explicit consumption
    auto local = std::move(listener); // NOLINT
}

/**
 * @brief Prepare container for next processing round.
 */
template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::prepare() {}

template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::init() {}

template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::process() {}

template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::initImpl(DerivedContainer container) { container->init(); }

template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::processImpl(DerivedContainer container) { container->process(); }

template<typename DerivedContainer>
void BaseContainer<DerivedContainer>::verifyCacheLookupIndex() {
    thread_local bool threadIdAssigned = false;
    if (threadIdAssigned) return;
    if (std::size_t const id = JsonScope::assignCacheLookupIndex(); id >= JsonScope::cacheLookupThreadCount) {
        throw std::runtime_error("Thread ID exceeds non-locking array size!");
    }
    threadIdAssigned = true;
}

} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCASTLISTENCONTAINER_BASECONTAINER_TPP
