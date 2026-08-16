//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <cmath>
#include <cstddef>
#include <memory>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Constants/ThreadSettings.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/BroadcastListenContainer/FlatContainer.hpp"
#include "Nebulite/Interaction/Rules/Listener.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Coordination/IdGenerator.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

FlatContainerBase::FlatContainerBase(Settings const& s) : settings(s) {}

namespace {
class ThreadIdGenerator {

    // Might be helpful for tracking max thread id set, as checking proper thread spreading is difficult otherwise.
#ifdef NDEBUG
    class Maximum {
        std::atomic<size_t> maxThreadIdAtomic{0};

    public:
        std::size_t get() const {
            return maxThreadIdAtomic.load(std::memory_order_acquire);
        }

        void set(std::size_t const threadId) {
            maxThreadIdAtomic.store(threadId, std::memory_order_release);
        }
    };

    static Maximum& maximum() {
        static Maximum maximum;
        return maximum;
    }
#endif

public:
    static std::size_t getThreadId() {
        static auto threadSpreader = Utility::Coordination::IdGenerator::atomicIncrementIdGenerator();
        thread_local std::size_t const threadId = threadSpreader();

        // Sanity check: cannot have more threads than workerCount
        assert(threadId < Constants::ThreadSettings::getInvokeWorkerCount());

#ifdef NDEBUG
        if (threadId > maximum().get()) {
            maximum().set(threadId);
        }
#endif

        return threadId;
    }
};
} // namespace

void FlatContainerBase::broadcast(std::shared_ptr<Interaction::Rules::Ruleset>&& entry) {
    thread_local auto threadId = ThreadIdGenerator::getThreadId();
    assert(threadId < activeWorkerCount); // Too many threads trying to broadcast/listen, increase FlatContainerBase::activeWorkerCount
    broadcasters[threadId][entry->getTopic()].push_back(std::move(entry));
}

void FlatContainerBase::listen(std::shared_ptr<Interaction::Rules::Listener>&& listener) {
    thread_local auto threadId = ThreadIdGenerator::getThreadId();
    assert(threadId < activeWorkerCount); // Too many threads trying to broadcast/listen, increase FlatContainerBase::activeWorkerCount
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

/**
 * @brief Applies rulesets to listeners based on their domains and the global space.
 * @tparam R The type of the rulesets range. Must be a viewable range.
 * @tparam L The type of the listeners range. Must be a viewable range.
 * @param rulesets The range of rulesets to apply.
 * @param listeners The range of listeners to apply the rulesets to.
 * @param globalSpace The global space to use for evaluation.
 */
template <typename R, typename L>
void apply(R&& rulesets, L&& listeners, Core::GlobalSpace& globalSpace) {
    for (auto& listener : std::forward<L>(listeners)) {
        for (auto const& ruleset : std::forward<R>(rulesets)) {
            if (ruleset->getId() == listener->domain.getId()) continue;
            if (ruleset->evaluateConditionGlobally(listener->domain, globalSpace)) {
                globalSpace.applyRulesetToListener(*ruleset, *listener);
            }
        }
    }
}

} // namespace

void FlatContainerBase::processWithOffset() {
    auto& globalSpace = Global::instance();
    for (auto& listenerMap : rotate(listeners, settings.listenerOffset)) {
        listenerMap.forall([&](std::string const& topic, auto& lv) {
            apply(
                rotate(broadcasters, settings.broadcasterOffset)
                    | std::views::transform([&](auto& broadcasterMap) -> auto& {
                          return broadcasterMap[topic];
                      })
                    | std::views::transform([&](auto& bv) {
                          return rotate(bv, settings.bvOffset);
                      })
                    | std::views::join,
                rotate(lv, settings.lvOffset),
                globalSpace
            );
            lv.clear();
        });
    }

    // Cleanup: Clear all broadcasters
    for (auto& broadcasterMap : broadcasters) {
        broadcasterMap.forallValues([&](auto& bv) {
            bv.clear();
        });
    }
}

void FlatContainerBase::processNoOffset(){
    auto& globalSpace = Global::instance();
    for (auto& listenerMap : listeners) {
        listenerMap.forall([&](std::string const& topic, auto& lv) {
            apply(
                broadcasters
                    | std::views::transform(
                        [&](auto& broadcasterMap) -> auto& {
                            return broadcasterMap[topic];
                        })
                    | std::views::join,
                lv,
                globalSpace
            );
            lv.clear();
        });
    }

    // Cleanup: Clear all broadcasters
    for (auto& broadcasterMap : broadcasters) {
        broadcasterMap.forallValues([&](auto& bv) {
            bv.clear();
        });
    }
}

} // namespace Nebulite::Data::BroadcastListenContainer
