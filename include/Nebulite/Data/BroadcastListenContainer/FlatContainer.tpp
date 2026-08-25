#ifndef NEBULITE_DATA_BROADCASTLISTENCONTAINER_FLATCONTAINER_TPP
#define NEBULITE_DATA_BROADCASTLISTENCONTAINER_FLATCONTAINER_TPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

// Nebulite
#include "Nebulite/Data/BroadcastListenContainer/BaseContainer.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_BROADCASTLISTENCONTAINER_FLATCONTAINER_HPP
#include "Nebulite/Data/BroadcastListenContainer/FlatContainer.hpp"
#endif // NEBULITE_DATA_BROADCASTLISTENCONTAINER_FLATCONTAINER_HPP

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

template <FlatContainerType Type>
FlatContainer<Type>::FlatContainer(std::atomic<bool>& stopFlag, std::size_t workerIndex, std::size_t workerCount)
        : BaseContainer<FlatContainer*>(stopFlag, workerIndex, workerCount, this) {
    FlatContainerBase::Settings settings{};

    if constexpr (Type == FlatContainerType::applyOffset) { // Set offsets based on worker index
        settings.relativeOffset = static_cast<double>(workerIndex) / static_cast<double>(workerCount);
        settings.listenerOffset = std::pow(settings.relativeOffset, 1);
        settings.broadcasterOffset = std::pow(settings.relativeOffset, 1);
        settings.lvOffset = std::pow(settings.relativeOffset, 2);
        settings.bvOffset = std::pow(settings.relativeOffset, 2);
    }
    else if constexpr (Type == FlatContainerType::noOffset) { // No offsets, workers start at the same index for listeners and broadcasters
        settings.relativeOffset = 0;
        settings.listenerOffset = std::pow(settings.relativeOffset, 1);
        settings.broadcasterOffset = std::pow(settings.relativeOffset, 1);
        settings.lvOffset = std::pow(settings.relativeOffset, 2);
        settings.bvOffset = std::pow(settings.relativeOffset, 2);
    }
    else {
        // More types may be added in the future
        std::unreachable();
    }

    base.emplace(settings);
}

template <FlatContainerType Type>
FlatContainer<Type>::~FlatContainer() = default;

template <FlatContainerType Type>
void FlatContainer<Type>::broadcast(std::shared_ptr<Interaction::Rules::Ruleset>&& entry) {
    base->broadcast(std::move(entry)); // NOLINT
}

template <FlatContainerType Type>
void FlatContainer<Type>::listen(std::shared_ptr<Interaction::Rules::Listener>&& listener) {
    base->listen(std::move(listener)); // NOLINT
}

template <FlatContainerType Type>
void FlatContainer<Type>::process() {
    if constexpr (Type == FlatContainerType::noOffset) {
        base->processNoOffset(); // NOLINT
    }
    else {
        base->processWithOffset(); // NOLINT
    }
}

} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCASTLISTENCONTAINER_FLATCONTAINER_TPP
