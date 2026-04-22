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
 */
template<typename Workspace>
class WorkDispatcher {
public:
    explicit WorkDispatcher(std::atomic<bool>& stop, std::function<void(Workspace&)> const& wF, std::function<void(Workspace&)> const& iF = nullptr);

    ~WorkDispatcher();

    void initializeWorkerThread();

    void waitForWorkFinished();

    void startWork();

    Workspace workspace;

private:
    void process();

    struct ThreadState {
        std::atomic<bool>& stopFlag;
        std::atomic<bool> workReady{false};
        std::atomic<bool> workFinished{false};
        std::condition_variable_any condition = {};
    } threadState;

    std::function<void(Workspace&)> workerFunction;
    std::function<void(Workspace&)> initFunction;

    std::thread workerThread;
    mutable SharedMutex mutex;
};

} // namespace Nebulite::Utility::Coordination
#include "Utility/Coordination/WorkDispatcher.tpp"
#endif // NEBULITE_UTILITY_COORDINATION_WORK_DISPATCHER_HPP
