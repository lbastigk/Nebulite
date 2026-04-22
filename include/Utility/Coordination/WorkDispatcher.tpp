#ifndef NEBULITE_UTILITY_COORDINATION_WORK_DISPATCHER_TPP
#define NEBULITE_UTILITY_COORDINATION_WORK_DISPATCHER_TPP

#include "Utility/Coordination/WorkDispatcher.hpp"

namespace Nebulite::Utility::Coordination {

template<typename Workspace>
WorkDispatcher<Workspace>::WorkDispatcher(std::atomic<bool>& stop, std::function<void(Workspace&)> const& wF, std::function<void(Workspace&)> const& iF)
        : threadState{ .stopFlag = stop }, workerFunction(wF), initFunction(iF)
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
    /*
    if constexpr (InitFunc != nullptr) {
        InitFunc(workspace);
    }
    */
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
    std::unique_lock lock(mutex);
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
        //WorkerFunc(workspace);
        workerFunction(workspace);

        // Notify that work is finished
        threadState.workReady = false;
        threadState.workFinished = true;
        threadState.condition.notify_one();
    }
}

} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_COORDINATION_WORK_DISPATCHER_TPP
