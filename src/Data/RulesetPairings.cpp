#include "Data/RulesetPairings.hpp"

#include "Core/RenderObject.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Data {

//------------------------------------------
// Container Methods

void BroadCastListenPairs::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    nextFrame.insertBroadcaster(entry);
}

void BroadCastListenPairs::listen(Interaction::Execution::DomainBase* listener, std::string const& topic, uint32_t const& listenerId) {
    thisFrame.insertListener(listener, topic, listenerId);
}

//------------------------------------------
// Worker Thread Methods

void BroadCastListenPairs::prepare() {
    std::scoped_lock lock(thisFrame.mutex, nextFrame.mutex);
    std::swap(thisFrame.data, nextFrame.data);
}

void BroadCastListenPairs::startWork() {
    threadState.workReady = true;
    threadState.workFinished = false;
    threadState.condition.notify_one();
}

void BroadCastListenPairs::waitForWorkFinished() const {
    while (!threadState.workFinished.load()) {
        std::this_thread::yield();
    }
}

//------------------------------------------
// Private Methods

void BroadCastListenPairs::process()  {
    // Thread-local random generator for probabilistic cleanup
    thread_local std::mt19937 cleanup_rng(std::random_device{}());
    thread_local std::uniform_int_distribution<int> cleanup_dist(0, 99); // uniform, avoids modulo bias

    // Set locks etc.
    while (!threadState.stopFlag) {
        // Wait for work to be ready
        std::unique_lock lock(thisFrame.mutex);
        threadState.condition.wait(lock, [this] {
            return threadState.workReady.load() || threadState.stopFlag.load();
        });
        // Process
        if (threadState.stopFlag)
            break;

        // Actual processing of thisFrame
        for (auto& map_other : std::views::values(thisFrame.data)) {
            for (auto& [isActive, rulesets] : std::views::values(map_other)) {
                if (!isActive)
                    continue;
                for (auto& [entry, listeners] : std::ranges::views::values(rulesets)) {
                    // Process active listeners (single pass, no erases here)
                    for (auto & it : listeners) {
                        auto &pair = it.second;
                        if (pair.active) {
                            pair.entry->apply(pair.contextOther);
                            pair.active = false;
                        }
                    }
                    // Probabilistic cleanup performed once per ruleset
                    if (cleanup_dist(cleanup_rng) == 0) {
                        for (auto it = listeners.begin(); it != listeners.end();) {
                            if (!it->second.active) {
                                auto itToErase = it++;
                                listeners.erase(itToErase); // erase returns void in Abseil
                            } else {
                                ++it;
                            }
                        }
                    }
                }
                // Reset activity flag, must be activated on broadcast
                isActive = false;
            }
        }

        // Set work flags
        threadState.workReady = false;
        threadState.workFinished = true;
    }
}

} // namespace Nebulite::Data
