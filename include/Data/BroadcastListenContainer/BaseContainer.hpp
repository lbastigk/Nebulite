/**
 * @file BaseContainer.hpp
 * @brief Definition of the virtual base class for broadcast-listen containers for rulesets.
 */

#ifndef NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_BASE_CONTAINER_HPP
#define NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_BASE_CONTAINER_HPP

//------------------------------------------
// Includes

// Standard library
#include <atomic>
#include <condition_variable>

// Nebulite
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Data::BroadcastListenContainer {

/**
 * @brief Class to manage broadcast-listen pairs of rulesets.
 */
class BaseContainer {
public:

    // TODO: Add ContainerThreadId to Constructor and ListenerThreadId to listen method, so we can better separate access based on thread id to avoid contention.
    //       This will be heavily used in the FlatContainer.

    explicit BaseContainer(std::atomic<bool>& stopFlag, uint32_t const& workerIndex, uint32_t const& workerCount);

    virtual ~BaseContainer() noexcept ;

    //------------------------------------------
    // Container Methods

    /**
     * @brief Broadcasts a ruleset to all listeners on its topic.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    virtual void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
        (void)entry;
    }

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listening domain.
     * @param topic The topic to listen for.
     * @param listenerId The unique ID of the listener render object.
     */
    virtual void listen(Interaction::Execution::Domain& listener, std::string const& topic, uint32_t const& listenerId) {
        (void)listener;
        (void)topic;
        (void)listenerId;
    }

    //------------------------------------------
    // Worker Thread Methods

    void initializeWorkerThread() {
        workerThread = std::thread([this] {
            ensureEarlyThreadId();
            this->process();
        });
    }

    void stopWorkerThread() {
        // Signal thread to stop
        threadState.stopFlag = true;
        threadState.condition.notify_one();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    /**
     * @brief Prepares for the next frame by swapping the current and next frame containers.
     */
    virtual void prepare(){}

    /**
     * @brief Notifies the worker thread to start processing.
     */
    void startWork() {
        threadState.workReady = true;
        threadState.workFinished = false;
        threadState.condition.notify_one();
    }

    /**
     * @brief Waits for the worker thread to finish processing.
     */
    void waitForWorkFinished() const {
        while (!threadState.workFinished.load()) {
            std::this_thread::yield();
        }
    }

    static void ensureEarlyThreadId();

protected:

    virtual void process() {
        while (!threadState.stopFlag) {
            // Wait for work to be ready
            {
                // This will be implemented in derived classes:
                /*
                std::unique_lock lock = thisFrame->lock();
                threadState.condition.wait(lock, [this] {
                    return threadState.workReady.load() || threadState.stopFlag.load();
                });
                */
            }

            // Process
            // thisFrame->process(); // This will be implemented in derived classes

            // Set work flags
            threadState.workReady = false;
            threadState.workFinished = true;
        }
    }

    struct WorkerInfo {
        uint32_t index; // This worker thread's index (0 to workerCount-1)
        uint32_t count; // Total number of worker threads
    } workerInfo;

    // Threading variables
    struct ThreadState {
        /**
         * @brief Condition variables for thread synchronization.
         */
        std::condition_variable_any condition = {};

        /**
         * @brief Flags to indicate when work is ready for each thread.
         */
        std::atomic<bool> workReady = false;

        /**
         * @brief Flags to indicate when work is finished for each thread.
         */
        std::atomic<bool> workFinished = false;

        /**
         * @brief Flag to signal threads to stop.
         */
        std::atomic<bool>& stopFlag;
    } threadState;

    std::thread workerThread;
};
} // namespace Nebulite::Data::BroadcastListenContainer
#endif // NEBULITE_DATA_BROADCAST_LISTEN_CONTAINER_BASE_CONTAINER_HPP
