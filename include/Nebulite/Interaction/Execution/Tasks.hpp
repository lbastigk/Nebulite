#ifndef NEBULITE_INTERACTION_EXECUTION_TASKS_HPP
#define NEBULITE_INTERACTION_EXECUTION_TASKS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/TaskQueue.hpp"
#include "Nebulite/Data/TaskQueueResult.hpp"
#include "Nebulite/Interaction/Context.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @brief Collection of different types of TaskQueues
 */
class Tasks {
public:
    /**
     * @struct StandardTasks
     * @brief Contains standard task queue names used in domains.
     */
    struct StandardTasks {
        static auto constexpr always = "tasks::always";
        static auto constexpr internal = "tasks::internal";
        static auto constexpr script = "tasks::script";
    };

    Tasks();

    /**
     * @brief Checks if the wait counter of the script TaskQueue is greater than zero.
     * @return True if the wait counter is greater than zero, false otherwise.
     */
    bool scriptIsWaiting();

    /**
     * @brief Increments the wait counter of the script TaskQueue by a specified count.
     * @param count The number of increments to add to the wait counter.
     */
    void incrementScriptWaitCounter(std::size_t count);

    /**
     * @brief Gets a specific task queue by name.
     * @param name The name of the task queue.
     * @return Pointer to the TaskQueue instance, or nullptr if not found.
     */
    std::shared_ptr<Data::TaskQueue> getTaskQueue(std::string_view name);

    /**
     * @brief Adds a task to the specified task queue.
     * @param name The name of the task to add.
     * @param queueName The name of the task queue to add the task to.
     */
    void addTask(std::string_view name, std::string_view queueName = StandardTasks::internal);

    /**
     * @brief Loads a script into the script task queue.
     * @param filename The name of the script file to add.
     * @param capture The capture instance for handling script output.
     */
    void addScript(std::string_view filename, Utility::IO::Capture& capture);

    /**
     * @brief Clears all tasks from any queues, but does not remove the TaskQueues themselves.
     */
    void clearAllTaskQueues();

    /**
     * @brief Clears a specified TaskQueue, but does not remove it from the list.
     * @param queueName The name of the TaskQueue to clear.
     * @return True if the specified name exists and was cleared, false otherwise.
     */
    bool clearTaskQueue(std::string_view queueName);

    /**
     * @brief Decrement only the wait counter of the script TaskQueue.
     */
    void decrementScriptWaitCounter();

    /**
     * @brief Decrement the wait counter of all TaskQueues.
     */
    void decrementWaitCounter();

    /**
     * @brief Parses the task queues and executes tasks based on the provided domain and scope.
     * @details The provided domain and scope is used for all contexts
     * @param domain The domain to use for parsing tasks.
     * @param scope The JSON scope to use for parsing tasks.
     * @param recover Whether to attempt recovery from errors.
     * @return The result of the parsing operation.
     */
    Constants::Event parse(Domain& domain, Data::JsonScope& scope, bool recover);

    /**
     * @brief Parses the queues and executes tasks based on provided contexts.
     * @param ctx The context to use for parsing tasks.
     * @param ctxScope The context scopes to use for parsing tasks.
     * @param recover Whether to attempt recovery from errors.
     * @return The result of the parsing operation.
     */
    Constants::Event parse(Context& ctx, ContextScope& ctxScope, bool recover);

private:
    /**
     * @brief Contains task queues for different types of tasks.
     */
    absl::flat_hash_map<std::string, std::shared_ptr<Data::TaskQueue>> tasks;

    /**
     * @brief Contains results of the last task queue resolutions.
     */
    absl::flat_hash_map<std::string, Data::TaskQueueResult> queueResult;
};

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_TASKS_HPP
