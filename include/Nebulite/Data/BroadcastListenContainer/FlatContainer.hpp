#ifndef NEBULITE_DATA_BROADCASTLISTENCONTAINER_FLATCONTAINER_HPP
#define NEBULITE_DATA_BROADCASTLISTENCONTAINER_FLATCONTAINER_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <optional>

// Nebulite
#include "Nebulite/Constants/ThreadSettings.hpp"
#include "Nebulite/Data/BroadcastListenContainer/BaseContainer.hpp"
#include "Nebulite/Data/BroadcastListenContainer/MapType.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

namespace Nebulite::Interaction::Rules {
class Ruleset;
struct Listener;
} // namespace Nebulite::Interaction::Rules

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

enum class FlatContainerType : bool {
    applyOffset, // Apply offset based on p = worker index / worker count, parts being p, others p^2
    noOffset, // No offset at all
};

template <FlatContainerType Type>
class FlatContainer;

class FlatContainerBase {
    // rendererWorkerCount should be enough, but if we decide that rulesets are able to directly broadcast/listen, we need one slot for each thread!
    static auto constexpr activeWorkerCount = Constants::ThreadSettings::Maximum::rendererWorkerCount;

    std::array<MapType<Interaction::Rules::Ruleset>, activeWorkerCount> broadcasters = {};
    std::array<MapType<Interaction::Rules::Listener>, activeWorkerCount> listeners = {};

public:
    struct Settings {
        double relativeOffset;
        double listenerOffset;
        double broadcasterOffset;
        double lvOffset;
        double bvOffset;
    };

    Settings const& settings;

    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset>&& entry);
    void listen(std::shared_ptr<Interaction::Rules::Listener>&& listener);

    /**
     * @brief Uses the provided offsets to process all broadcasted rulesets.
     */
    void processWithOffset();

    /**
     * @brief Ignores settings and processes all broadcasted rulesets without any rotation or offset.
     */
    void processNoOffset();

    explicit FlatContainerBase([[clang::lifetimebound]] Settings const& s);
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
    explicit FlatContainer(std::atomic<bool>& stopFlag, std::size_t workerIndex, std::size_t workerCount);

    ~FlatContainer() override ;

    FlatContainer(FlatContainer const&) = delete;
    FlatContainer& operator=(FlatContainer const&) = delete;
    FlatContainer(FlatContainer&&) = delete;
    FlatContainer& operator=(FlatContainer&&) = delete;

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset>&& entry) override ;

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listener to add.
     */
    void listen(std::shared_ptr<Interaction::Rules::Listener>&& listener) override ;

    /**
     * @brief Processes all broadcasted rulesets,
     *        matching them with listeners and executing the appropriate actions.
     */
    void process() override ;

private:
    /**
     * @brief Non-templated base class of the FlatContainer
     */
    std::optional<FlatContainerBase> base;
};

} // namespace Nebulite::Data::BroadcastListenContainer
#include "Nebulite/Data/BroadcastListenContainer/FlatContainer.tpp" // NOLINT(misc-include-cleaner)
#endif // NEBULITE_DATA_BROADCASTLISTENCONTAINER_FLATCONTAINER_HPP
