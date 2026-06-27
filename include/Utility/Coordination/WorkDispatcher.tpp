#ifndef UTILITY_COORDINATION_WORKDISPATCHER_TPP
#define UTILITY_COORDINATION_WORKDISPATCHER_TPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <functional>
#include <mutex>

//------------------------------------------
// Conditional includes

#ifndef UTILITY_COORDINATION_WORKDISPATCHER_HPP
    #include "Utility/Coordination/WorkDispatcher.hpp"
#endif // UTILITY_COORDINATION_WORKDISPATCHER_HPP

//------------------------------------------
namespace Nebulite::Utility::Coordination {

template<typename Workspace>
WorkDispatcher<Workspace>::WorkDispatcher(std::atomic<bool>& stop, std::function<void(Workspace&)> const& wF, std::function<void(Workspace&)> const& iF)
    : threadState{ .stopFlag = stop, .condition = {} }
    , workerFunction(wF), initFunction(iF)
{
    initializeWorkerThread();
}

template<typename Workspace>
WorkDispatcher<Workspace>::~WorkDispatcher() {
    threadState.stopFlag = true;
    threadState.condition.notify_one();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

template<typename Workspace>
void WorkDispatcher<Workspace>::initializeWorkerThread() {
    if (initFunction != nullptr) {
        initFunction(workspace);
    }
    workerThread = std::thread([this] {
        this->process();
    });
}

template<typename Workspace>
void WorkDispatcher<Workspace>::waitForWorkFinished() {
    std::unique_lock lock(mutex);
    threadState.condition.wait(lock, [this] {
        return threadState.workFinished.load();
    });
}

template<typename Workspace>
void WorkDispatcher<Workspace>::startWork() {
    std::unique_lock const lock(mutex);
    threadState.workReady = true;
    threadState.workFinished = false;
    threadState.condition.notify_one();
}

template<typename Workspace>
void WorkDispatcher<Workspace>::process() {
    while (!threadState.stopFlag.load()) {
        std::unique_lock lock(mutex);
        threadState.condition.wait(lock, [this] {
            return threadState.workReady.load() || threadState.stopFlag.load();
        });

        if (threadState.stopFlag.load()) break;

        // Call the template function
        workerFunction(workspace);

        // Notify that work is finished
        threadState.workReady = false;
        threadState.workFinished = true;
        threadState.condition.notify_one();
    }
}

} // namespace Nebulite::Utility::Coordination
#endif // UTILITY_COORDINATION_WORKDISPATCHER_TPP
