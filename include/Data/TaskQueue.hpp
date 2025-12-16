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

//------------------------------------------

namespace Nebulite::Data {
/**
 * @brief Represents the result of resolving a task queue.
 *        This structure holds the outcome of processing a task queue, including any errors
 *        encountered during resolution and whether the process was halted due to a critical error.
 */
struct TaskQueueResult {
    bool encounteredCriticalResult = false;
    std::vector<Constants::Error> errors;
};

/**
 * @struct taskQueueWrapper
 * @brief Represents a queue of tasks to be processed by the engine, including metadata.
 * @todo Rename to TaskQueue, deque should simply be called 'tasks'
 * @todo Move to Nebulite::Data::TaskQueue, as it is not specific to GlobalSpace
 * @todo Would it make sense for this to be a class derived from Domain?
 *       That way, each taskQueue would have functions such as clear, wait, always etc.?
 *       Perhaps an idea for the future, for now we simply use separate taskQueueWrappers for each type:
 *       - script
 *       - always
 *       - internal
 */
struct TaskQueue {
public:
    std::deque<std::string> tasks; // List of tasks
    bool clearAfterResolving = true; // Whether to clear the task list after resolving
    /**
     * @note Add more metadata as needed, for resolveTaskQueue() to use
     *       in case new task types are added in the future.
     *       Perhaps even a hashmap of string to variant around this wrapper for
     *       maximum flexibility.
     *       map string -> taskQueueWrapper{taskQueue, <metadata>}
     *       This way, each task could be sorted into different queues based on type,
     *       so we can simply call <task> for normal tasks,
     *       and specify "on-queue <type> <task>" for specific task types
     *       that we wish to execute in a different manner.
     *       This could allow us to auto-sort tasks into e.g. always-tasks etc.
     *       or even manage tasks with calls such as "modify-task <identifier> <modification>"
     * @todo Implement waitCounter into each taskQueueWrapper, so each queue can have its own wait counter
     *       Then, have function such as wait, task, etc. modify a specify taskQueue.
     *       calls with "on-queue", e.g. "on-queue <always/wait/etc.> <args>" can modify specific queues.
     * @todo Add mutex for thread-safe append/clear of tasks
     * @todo Add its own resolve function, with param for domain.
     * @todo Add own wait counter, being lowered on each frame update.
     */
    void append(std::string const& task);
    void wait(uint64_t const& frames);
    TaskQueueResult resolve();
    void clear(); // Should also clear buffer?

//private:  // Make private later on
    /**
     * @todo An option to avoid race-conditions when multiple threads append tasks.
     *       Idea: Store tasks in temporary buffer, insert into main queue alphanumerically sorted on resolve.
     *       This way, multiple threads can append tasks without locking the main queue.
     *       On resolve, we lock the main queue, merge the temporary buffer into the main queue in sorted order, and then clear the temporary buffer.
     *       This ensures that tasks are always executed in a consistent order, while minimizing locking overhead during appends.
     */
    std::mutex bufferMutex; // Mutex for thread-safe access to the temporary buffer
    std::mutex queueMutex;  // Mutex for thread-safe access to the task queue
    uint64_t waitCounter = 0; // Frames to wait before processing tasks
    std::vector<std::string> tempBuffer; // Temporary buffer for tasks added by multiple threads

    struct Settings {
        bool clearAfterResolving = true;
    } settings;
};
} // namespace Nebulite::Data