#include "Data/BroadcastListenContainer/FlatContainer.hpp"
#include "Utility/Threading.hpp"

namespace Nebulite::Data::BroadcastListenContainer {

void FlatContainer::broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
    static auto threadSpreader = Utility::Threading::atomicThreadIncrementGenerator();

    // First half of broadcasterSpreading without locking, second half spreads with locking
    if (thread_local size_t threadId = threadSpreader(); threadId < broadcasterSpreading / 2) {
        // 0 to broadcasterSpreading/2 - 1: No locking, purely thread-local
        broadcasters[threadId][entry->getTopic()].push_back(entry);
    }
    else {
        // broadcasterSpreading/2 to broadcasterSpreading - 1: With locking, shared among threads
        auto const sharedId = threadId % (broadcasterSpreading / 2) + broadcasterSpreading / 2;
        broadcasters[sharedId].lock(entry->getTopic());
        broadcasters[sharedId][entry->getTopic()].push_back(entry);
    }
}

void FlatContainer::listen(std::shared_ptr<Interaction::Rules::Listener> const& listener) {
    // Listening generally happens on multiple threads, so we need to lock the listener map for this topic while modifying it
    auto lock = listeners.lock(listener->topic);
    auto& t = listeners[listener->topic];
    t.push_back(listener);
}

void FlatContainer::process() {
    // Offset for this worker thread
    // Distributes listener access by offsetting the starting index for each worker thread
    // This way, workers are less likely to contend for the same listeners when processing the same topic
    thread_local double const relativeOffset = static_cast<double>(workerInfo.index) / static_cast<double>(workerInfo.count);

    auto const broadcasterOffset = static_cast<size_t>(relativeOffset*static_cast<double>(broadcasterSpreading));
    listeners.forall([&](std::string const& topic, auto& lv) {

        // Derive size and offset for this worker thread
        auto const lvSize = lv.size();
        auto const lvOffset = static_cast<size_t>(relativeOffset*static_cast<double>(lvSize));

        // Process all entries for this topic
        for (size_t i = 0; i < lvSize; ++i) {
            size_t const idx = (i + lvOffset) % lvSize;
            auto& listener = lv.at(idx);

            for (size_t j = 0; j < broadcasterSpreading; ++j) {
                size_t const broadcasterIdx = (j + broadcasterOffset) % broadcasterSpreading;
                for (auto& broadcasterMap = broadcasters[broadcasterIdx]; auto const& ruleset : broadcasterMap[topic]) { // No offsetting here, since all broadcasters are per-worker thread and thus already distributed. Tests show that offsetting here does not improve performance.
                    if (ruleset->getId() == listener->listenerId) {
                        continue; // Skip if the ruleset is from the same render object as the listener
                    }
                    if (ruleset->evaluateCondition(listener->domain)) {
                        ruleset->apply(listener);
                    }
                }
            }
        }

        lv.clear(); // Clear the listener vector for this topic after processing
    });

    // Cleanup: Clear all broadcaster vectors
    for (auto& broadcasterMap : broadcasters) {
        broadcasterMap.forall([&](auto& bv) {
            bv.clear();
        });
    }
}

void FlatContainer::init() {
    // Nothing to initialize for the flat container
}

} // namespace Nebulite::Data::BroadcastListenContainer
