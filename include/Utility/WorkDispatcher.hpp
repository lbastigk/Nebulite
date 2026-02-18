#ifndef NEBULITE_UTILITY_WORK_DISPATCHER_HPP
#define NEBULITE_UTILITY_WORK_DISPATCHER_HPP


//------------------------------------------
// Includes

// Standard library
#include <condition_variable>
#include <thread>

// Nebulite
#include "Utility/SharedMutex.hpp"

namespace Nebulite::Utility {

// TODO: Use this dispatcher for the BaseContainer class

template<typename Workspace, auto WorkerFunc>
class WorkDispatcher {
public:
    explicit WorkDispatcher(std::atomic<bool>& stop)
        : threadState{ .stopFlag = stop }
    {
        initializeWorkerThread();
    }

    ~WorkDispatcher() {
        threadState.stopFlag = true;
        threadState.condition.notify_one();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void initializeWorkerThread() {
        workerThread = std::thread([this] {
            this->process();
        });
    }

    void waitForWorkFinished() {
        std::unique_lock lock(mutex);
        threadState.condition.wait(lock, [this] {
            return threadState.workFinished.load();
        });
    }

    void startWork() {
        threadState.workReady = true;
        threadState.workFinished = false;
        threadState.condition.notify_one();
    }

    Workspace workspace;

private:
    void process() {
        while (!threadState.stopFlag.load()) {
            std::unique_lock lock(mutex);
            threadState.condition.wait(lock, [this] {
                return threadState.workReady.load() || threadState.stopFlag.load();
            });

            if (threadState.stopFlag.load()) break;

            // Call the template function
            WorkerFunc(workspace);

            // Notify that work is finished
            threadState.workReady = false;
            threadState.workFinished = true;
            threadState.condition.notify_one();
        }
    }

    struct ThreadState {
        std::atomic<bool>& stopFlag;
        std::atomic<bool> workReady{false};
        std::atomic<bool> workFinished{false};
        std::condition_variable_any condition = {};
    } threadState;

    std::thread workerThread;
    mutable SharedMutex mutex;
};

} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_WORK_DISPATCHER_HPP
