/**
 * @file FlatContainer.hpp
 * @brief Contains the definition of the Flat Broadcast-listen container model.
 * @details Multiple types of flat containers are available, determined by the FlatContainerType template parameter.
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

enum class FlatContainerType {
    WithRotation, // Apply offset based on p = worker index / worker count, parts being p, others p^2
    WithoutRotation // No offset at all
};

template <FlatContainerType Type>
class FlatContainer;

class FlatContainerImpl {
    // All functions private, only accessible by FlatContainer, which is the only friend class

    template <FlatContainerType Type>
    friend class FlatContainer;

    // Add entire workspace and functions from FlatContainer here
    // Then we add FlatContainerImp as member of FlatContainer, and pass the settings to any function.
    // Passing them during construction might work, but then we likely have to make FlatContainerImpl a unique_ptr.

    struct Settings {
        double relativeOffset;
        double listenerOffset;
        double broadcasterOffset;
        double lvOffset;
        double bvOffset;
    };

    Settings const& settings;

    static std::unique_ptr<FlatContainerImpl> create(Settings const& s) {
        return std::unique_ptr<FlatContainerImpl>(new FlatContainerImpl(s));
    }

    explicit FlatContainerImpl(Settings const& s) : settings(s) {}

    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry);
    void listen(std::shared_ptr<Interaction::Rules::Listener> const& listener);
    void process();

    static auto constexpr activeWorkerCount = Constants::ThreadSettings::Maximum::invokeWorkerCount;
    std::array<HotStringKeyMap<std::vector<std::shared_ptr<Interaction::Rules::Ruleset>>>, activeWorkerCount> broadcasters = {};
    std::array<HotStringKeyMap<std::vector<std::shared_ptr<Interaction::Rules::Listener>>>, activeWorkerCount> listeners = {};
};

/**
 * @class FlatContainer
 * @brief A broadcast-listen container that uses a flat structure for storing rulesets and listeners, no direct storage of pairs.
 * @tparam Type The type of the flat container, determining how offsets are applied for worker threads.
 *              WithRotation applies offsets to spread workers across the container, while WithoutRotation has all workers
 *              starting at index 0 for listeners and broadcasters.
 */
template <FlatContainerType Type>
class FlatContainer final : public BaseContainer<FlatContainer<Type>*> {
public:
    explicit FlatContainer(std::atomic<bool>& stopFlag, uint32_t const& workerIndex, uint32_t const& workerCount)
        : BaseContainer<FlatContainer*>(stopFlag, workerIndex, workerCount, this)
    {
        FlatContainerImpl::Settings settings{};

        if constexpr (Type == FlatContainerType::WithRotation) { // Set offsets based on worker index
            settings.relativeOffset = static_cast<double>(workerIndex) / static_cast<double>(workerCount);
            settings.listenerOffset = std::pow(settings.relativeOffset, 1);
            settings.broadcasterOffset = std::pow(settings.relativeOffset, 1);
            settings.lvOffset = std::pow(settings.relativeOffset, 2);
            settings.bvOffset = std::pow(settings.relativeOffset, 2);
        }
        else if constexpr (Type == FlatContainerType::WithoutRotation) { // No offsets, workers start at the same index for listeners and broadcasters
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

        impl = FlatContainerImpl::create(settings);
    }

    ~FlatContainer() override = default;

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) override {
        impl->broadcast(entry);
    }

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listener to add.
     */
    void listen(std::shared_ptr<Interaction::Rules::Listener> const& listener) override {
        impl->listen(listener);
    }

    /**
     * @brief Empty, no preparation needed for this container type
     */
    void prepare() override {}

    /**
     * @brief Empty, no initialization needed for this container type
     */
    void init() override {}

    /**
     * @brief Processes all broadcasted rulesets,
     *        matching them with listeners and executing the appropriate actions.
     */
    void process() override {
        impl->process();
    }

private:
    /**
     * @brief Non-templated implementation class of the FlatContainer
     */
    std::unique_ptr<FlatContainerImpl> impl;
};

} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_FLAT_CONTAINER_HPP
