//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <cstddef>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Data/BroadcastListenContainer/FlatContainer.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Listener.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Utility/Coordination/IdGenerator.hpp"

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

namespace {
size_t getThreadId() {
    static auto workerCount = Constants::ThreadSettings::getInvokeWorkerCount();
    static auto threadSpreader = Utility::Coordination::IdGenerator::atomicThreadIncrementGenerator();
    thread_local size_t threadId = threadSpreader();
    if (threadId >= workerCount) {
        throw std::runtime_error("Too many threads trying to broadcast, increase broadcasterSpreading or reduce thread count");
    }
    return threadId;
}
} // namespace

void FlatContainerImpl::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> entry) {
    auto const threadId = getThreadId();
    //auto lock = broadcasters[threadId].lock(entry->getTopic());
    broadcasters[threadId][entry->getTopic()].push_back(std::move(entry));
}

void FlatContainerImpl::listen(std::shared_ptr<Interaction::Rules::Listener> listener) {
    auto const threadId = getThreadId();
    // Lock is, for some reason, necessary ...
    auto lock = listeners[threadId].lock(listener->topic);
    listeners[threadId][listener->topic].push_back(std::move(listener));
}

namespace {
/**
 * @brief Rotates a range by a given percentage.
 * @tparam R The type of the range to rotate. Must be a viewable range.
 * @param r The range to rotate.
 * @param percent The percentage to rotate the range by. Should be in the range [0, 1), but can be any real number.
 * @return A new range that is the result of rotating the input range by the specified percentage.
 */
template <std::ranges::viewable_range R>
auto rotate(R&& r, double percent) {
    auto view = std::views::all(std::forward<R>(r));

    auto size = std::ranges::size(view);

    if (size == 0) {
        percent = 0.0;
    }

    double normalized = std::fmod(percent, 1.0);
    if (normalized < 0)
        normalized += 1.0;

    std::size_t const offset =
        size == 0 ? 0 :
        static_cast<std::size_t>(
            std::floor(normalized * static_cast<double>(size))
        ) % size;

    return std::views::concat(
        view | std::views::drop(offset),
        view | std::views::take(offset)
    );
}
} // namespace

void FlatContainerImpl::process() {
    for (auto& listenerMap : rotate(listeners, settings.listenerOffset)) {
        listenerMap.forall([&](std::string const& topic, auto& lv) {
            // Build a flattened view of all rulesets for this topic
            auto rulesets = rotate(broadcasters, settings.broadcasterOffset)
                | std::views::transform([&](auto& broadcasterMap) -> auto& {
                      return broadcasterMap[topic];
                  })
                | std::views::transform([&](auto& bv) {
                      return rotate(bv, settings.bvOffset);
                  })
                | std::views::join;

            // Apply all valid rulesets and clear listeners
            for (auto& listener : rotate(lv, settings.lvOffset)) {
                for (auto const& ruleset : rulesets) {
                    if (ruleset->getId() == listener->domain.getId()) continue;
                    if (ruleset->evaluateCondition(listener->domain)) ruleset->apply(listener);
                }
            }
            lv.clear();
        });
    }

    // Cleanup: Clear all broadcasters
    for (auto& broadcasterMap : broadcasters) {
        broadcasterMap.forall([&](auto& bv) {
            bv.clear();
        });
    }
}

} // namespace Nebulite::Data::BroadcastListenContainer
