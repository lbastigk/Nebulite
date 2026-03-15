/**
 * @file WorkDispatcher.hpp
 * @brief Definition of the WorkDispatcher class for managing worker threads that process work using provided functions and workspaces.
 */

#ifndef NEBULITE_UTILITY_COORDINATION_WORK_DISPATCHER_HPP
#define NEBULITE_UTILITY_COORDINATION_WORK_DISPATCHER_HPP


//------------------------------------------
// Includes

// Standard library
#include <condition_variable>
#include <thread>

// Nebulite
#include "SharedMutex.hpp"

namespace Nebulite::Utility::Coordination {
/**
 * @brief A class to manage a worker thread that processes work using a provided function and workspace.
 * @tparam Workspace The workspace of the worker thread
 * @tparam WorkerFunc The function to call in the worker thread, must be a template function that takes the workspace as an argument
 * @tparam InitFunc An optional function to call in the worker thread before the main loop, must be a template function that takes the workspace as an argument
 */
template<typename Workspace, auto WorkerFunc, auto InitFunc = nullptr>
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
        if constexpr (InitFunc != nullptr) {
            InitFunc(workspace);
        }
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
        std::unique_lock lock(mutex);
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

} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_COORDINATION_WORK_DISPATCHER_HPP
