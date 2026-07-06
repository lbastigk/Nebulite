#ifndef UTILITY_COORDINATION_WORKDISPATCHER_HPP
#define UTILITY_COORDINATION_WORKDISPATCHER_HPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>

// Nebulite
#include "Utility/Coordination/SharedMutex.hpp"

//------------------------------------------
namespace Nebulite::Utility::Coordination {
/**
 * @brief A class to manage a worker thread that processes work using a provided function and workspace.
 * @tparam Workspace The workspace of the worker thread
 */
template<typename Workspace>
class WorkDispatcher {
public:
    /**
     * @brief Construct a new Work Dispatcher object
     * @param stop A reference to an atomic boolean that indicates when to stop the worker thread
     * @param wF The worker function to execute. Parameter: Reference to the Workspace.
     * @param iF The initializer function to execute. Parameter: Reference to the Workspace.
     */
    explicit WorkDispatcher(std::atomic<bool>& stop, std::function<void(Workspace&)> const& wF, std::function<void(Workspace&)> const& iF = nullptr);

    ~WorkDispatcher();

    WorkDispatcher(WorkDispatcher const&) = delete;
    WorkDispatcher& operator=(WorkDispatcher const&) = delete;
    WorkDispatcher(WorkDispatcher&&) = delete;
    WorkDispatcher& operator=(WorkDispatcher&&) = delete;

    /**
     * @brief Initializes the worker thread and executes the initializer function if provided.
     */
    void initializeWorkerThread();

    /**
     * @brief Waits for the worker thread to finish processing the current work.
     *        This function blocks until the worker thread signals that it has finished processing.
     */
    void waitForWorkFinished();

    /**
     * @brief Signals the worker thread to start processing work.
     */
    void startWork();

    /**
     * @brief The workspace of the worker thread.
     */
    Workspace workspace;

private:
    /**
     * @brief Core processing function of the worker thread.
     */
    void process();

    /**
     * @brief State of the worker thread.
     */
    struct ThreadState {
        std::atomic<bool>& stopFlag; // External stop signal
        std::atomic<bool> workReady{false}; // Internal signal to start processing
        std::atomic<bool> workFinished{false}; // Internal signal to indicate processing is finished
        std::condition_variable_any condition;
    } threadState;

    std::function<void(Workspace&)> workerFunction;
    std::function<void(Workspace&)> initFunction;

    std::thread workerThread;
    mutable SharedMutex mutex;
};
} // namespace Nebulite::Utility::Coordination
#include "Utility/Coordination/WorkDispatcher.tpp" // NOLINT
#endif // UTILITY_COORDINATION_WORKDISPATCHER_HPP
