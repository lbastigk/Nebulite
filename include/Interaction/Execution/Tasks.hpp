#ifndef INTERACTION_EXECUTION_TASKS_HPP
#define INTERACTION_EXECUTION_TASKS_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Data/TaskQueue.hpp"
#include "Interaction/Context.hpp"

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
     * @brief Contains standard task queue names used in the GlobalSpace.
     */
    struct StandardTasks {
        static auto constexpr always = "tasks::always";
        static auto constexpr internal = "tasks::internal";
        static auto constexpr script = "tasks::script";
    };

    Tasks();

    bool scriptIsWaiting();

    void incrementScriptWaitCounter(size_t const& count);

    /**
     * @brief Gets a specific task queue by name.
     * @param name The name of the task queue.
     * @return Pointer to the TaskQueue instance, or nullptr if not found.
     */
    std::shared_ptr<Data::TaskQueue> getTaskQueue(std::string const& name);

    void addTask(std::string const& name, std::string const& queueName = StandardTasks::internal);

    void addScript(std::string const& filename, Utility::IO::Capture& capture);

    void clearAllTaskQueues();

    bool clearTaskQueue(std::string const& queueName);

    void decrementScriptWaitCounter();

    void decrementWaitCounter();

    Constants::Event parse(Domain& domain, Data::JsonScope& scope, bool recover);

    Constants::Event parse(Context& ctx, ContextScope& ctxScope, bool recover);

private:
    /**
     * @brief Contains task queues for different types of tasks.
     */
    absl::flat_hash_map<std::string, std::shared_ptr<Data::TaskQueue>> tasks;

    /**
     * @brief Contains results of the last task queue resolutions.
     */
    absl::flat_hash_map<std::string, Data::TaskQueue::TaskQueueResult> queueResult;
};

} // namespace Nebulite::Interaction::Execution
#endif // INTERACTION_EXECUTION_TASKS_HPP
