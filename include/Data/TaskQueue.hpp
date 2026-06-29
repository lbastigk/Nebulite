/**
 * @file TaskQueue.hpp
 * @brief Definition of TaskQueue and TaskQueueResult structures for managing task queues in Nebulite.
 */

#ifndef DATA_TASKQUEUE_HPP
#define DATA_TASKQUEUE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <deque>
#include <mutex>
#include <string>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

//------------------------------------------
namespace Nebulite::Data {
/**
 * @struct Nebulite::Data::TaskQueue
 * @brief Represents a queue of tasks to be processed by the engine, including metadata.
 */
class TaskQueue {
public:
    /**
     * @brief Constructs a TaskQueue with specified settings.
     * @param callbackName The name used as arg[0] when parsing tasks from this queue.
     * @param clearAfterResolving If true, the task queue is cleared after resolving tasks.
     */
    explicit TaskQueue(std::string const& callbackName, bool const clearAfterResolving = true)
        : settings{.callbackName=callbackName, .clearAfterResolving=clearAfterResolving} {}

    /**
     * @brief Represents the result of resolving a task queue.
     *        This structure holds the outcome of processing a task queue, including any errors
     *        encountered during resolution and whether the process was halted due to a critical error.
     *        Does not include successful results
     * @note Since Events just hold the type (Warning, Error), returning a vector of results might be unnecessary.
     *       Although it's sort of useful, as we can count the amount of events and inform: "Task encountered X warnings and Y errors"
     *       Later on we might change that to 2 std::size_t that count the amount of warnings and errors, instead of returning a vector of events.
     */
    struct TaskQueueResult {
        bool encounteredCriticalResult = false; // Indicates if a critical error was encountered during task resolution
        std::vector<Constants::Event> events;   // List of events encountered during task resolution
    };

    /**
     * @brief Resolves the task queue by parsing and executing each task in the context of the provided domain.
     * @details skips tasks if the internal wait counter is greater than zero.
     * @param ctx The context of the interaction. Commands are parsed into 'self'
     * @param ctxScope The context scope of the interaction.
     * @param recover If true, continues processing tasks even after encountering a critical error.
     * @return The result of the task queue resolution.
     */
    TaskQueueResult resolve(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, bool recover);

    void addScript(std::string const& filename, Utility::IO::Capture& capture);

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
    void wait(std::uint64_t frames);

    /**
     * @brief Clears all tasks from the task queue.
     */
    void clear();

    /**
     * @brief Increments the internal wait counter by a specified value.
     * @param increment The value to increment the wait counter by. Default is 1.
     */
    void incrementWaitCounter(std::uint64_t increment = 1);

    /**
     * @brief Decrements the internal wait counter by 1, ensuring it does not go below zero.
     * @param decrement The value to decrement the wait counter by. Default is 1.
     */
    void decrementWaitCounter(std::uint64_t decrement = 1);

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
        std::string const callbackName; // Name used as arg[0] when parsing tasks from this queue
        bool clearAfterResolving = true;
    } settings;

    struct State {
        std::uint64_t waitCounter = 0; // Internal wait counter
    } state;
};
} // namespace Nebulite::Data
#endif // DATA_TASKQUEUE_HPP
