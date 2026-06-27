//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <cstddef>
#include <memory>
#include <ranges>
#include <string>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Data/BroadcastListenContainer/FlatContainer.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Listener.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/IdGenerator.hpp"

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

namespace {

class ThreadIdGenerator {

    // Might be helpful for tracking max thread id set
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
        thread_local std::size_t threadId = threadSpreader();

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
} // namespace

void FlatContainerBase::processWithRotation() {
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
                    if (ruleset->evaluateConditionGlobally(listener->domain, Global::instance())) {
                        ruleset->apply(listener, Global::instance());
                    }
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

void FlatContainerBase::processWithoutRotation(){
    for (auto& listenerMap : listeners) {
        listenerMap.forall([&](std::string const& topic, auto& lv) {
            // Build a flattened view of all rulesets for this topic
            auto rulesets = broadcasters
                | std::views::transform([&](auto& broadcasterMap) -> auto& {return broadcasterMap[topic];})
                | std::views::join;

            // Apply all valid rulesets and clear listeners
            for (auto& listener : lv) {
                for (auto const& ruleset : rulesets) {
                    if (ruleset->getId() == listener->domain.getId()) continue;
                    if (ruleset->evaluateConditionGlobally(listener->domain, Global::instance())) {
                        ruleset->apply(listener, Global::instance());
                    }
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
