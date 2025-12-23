/**
 * @file TaskQueue.hpp
 * @brief Definition of TaskQueue and TaskQueueResult structures for managing task queues in Nebulite.
 */

#ifndef NEBULITE_DATA_TASKQUEUE_HPP
#define NEBULITE_DATA_TASKQUEUE_HPP

//------------------------------------------
// Includes

// Standard library
#include <deque>
#include <string>
#include <vector>
#include <mutex>
#include <cstdint>
#include <cstddef>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Context.hpp"

//------------------------------------------

namespace Nebulite::Data {
/**
 * @brief Represents the result of resolving a task queue.
 *        This structure holds the outcome of processing a task queue, including any errors
 *        encountered during resolution and whether the process was halted due to a critical error.
 */
struct TaskQueueResult {
    bool encounteredCriticalResult = false; // Indicates if a critical error was encountered during task resolution
    std::vector<Constants::Error> errors;   // List of errors encountered during task resolution
};

/**
 * @struct Nebulite::Data::TaskQueue
 * @brief Represents a queue of tasks to be processed by the engine, including metadata.
 * @todo Would it make sense for this to be a class derived from Domain?
 *       That way, each taskQueue would have functions such as clear, wait, always etc.?
 *       Perhaps an idea for the future, for now we simply use separate taskQueueWrappers for each type:
 *       - script
 *       - always
 *       - internal
 */
class TaskQueue {
public:

    /**
     * @brief Constructs a TaskQueue with specified settings.
     * @param clearAfterResolving If true, the task queue is cleared after resolving tasks.
     * @param callbackName The name used as arg[0] when parsing tasks from this queue.
     */
    explicit TaskQueue(std::string const& callbackName, bool const& clearAfterResolving = true)
        : settings{callbackName, clearAfterResolving} {}

    /**
     * @brief Resolves the task queue by parsing and executing each task in the context of the provided domain.
     * @details skips tasks if the internal wait counter is greater than zero.
     * @param context The domain context in which to resolve the tasks.
     * @return The result of the task queue resolution.
     */
    TaskQueueResult resolve(Interaction::Execution::DomainBase& context, bool const& recover);

    /**
     * @brief Appends a task to the task queue.
     * @param task The task string to append.
     */
    void pushBack(std::string const& task);

    /**
     * @brief Pushes a task to the front of the task queue.
     * @param task The task string to push.
     */
    void pushFront(std::string const& task);

    /**
     * @brief Increases the internal wait counter by the specified number of frames.
     * @param frames The number of frames to wait.
     */
    void wait(uint64_t const& frames);

    /**
     * @brief Clears all tasks from the task queue.
     */
    void clear();

    /**
     * @brief Increments the internal wait counter by a specified value.
     * @param increment The value to increment the wait counter by. Default is 1.
     */
    void incrementWaitCounter(uint64_t const& increment = 1);

    /**
     * @brief Decrements the internal wait counter by 1, ensuring it does not go below zero.
     * @param decrement The value to decrement the wait counter by. Default is 1.
     */
    void decrementWaitCounter(uint64_t const& decrement = 1);

    /**
     * @brief Checks if the task queue is currently waiting.
     * @return True if the wait counter is greater than zero, false otherwise.
     */
    [[nodiscard]] bool isWaiting() const ;

private:
    struct threadsafeTasks {
        std::deque<std::string> list; // List of tasks.
        std::mutex mutex;  // Mutex for thread-safe access to the task queue
    };

    threadsafeTasks tasks; // Thread-safe task queue

    struct Settings {
        std::string callbackName; // Name used as arg[0] when parsing tasks from this queue
        bool clearAfterResolving = true;
    } settings;

    struct State {
        uint64_t waitCounter = 0; // Internal wait counter
    } state;
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_TASKQUEUE_HPP
