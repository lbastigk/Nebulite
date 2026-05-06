#ifndef NEBULITE_INTERACTION_EXECUTION_TASKS_HPP
#define NEBULITE_INTERACTION_EXECUTION_TASKS_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

// Nebulite
#include "Data/TaskQueue.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

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


    Tasks() {
        tasks[StandardTasks::always] = std::make_shared<Data::TaskQueue>(StandardTasks::always, false);
        tasks[StandardTasks::internal] = std::make_shared<Data::TaskQueue>(StandardTasks::internal, true);
        tasks[StandardTasks::script] = std::make_shared<Data::TaskQueue>(StandardTasks::script, true);
    }

    bool scriptIsWaiting() {
        return tasks[StandardTasks::script]->isWaiting();
    }

    void incrementScriptWaitCounter(size_t const& count) {
        tasks[StandardTasks::script]->incrementWaitCounter(count);
    }

    /**
     * @brief Gets a specific task queue by name.
     * @param name The name of the task queue.
     * @return Pointer to the TaskQueue instance, or nullptr if not found.
     */
    std::shared_ptr<Data::TaskQueue> getTaskQueue(std::string const& name) {
        if (auto const it = tasks.find(name); it != tasks.end()) {
            return it->second;
        }
        return nullptr;
    }

    void addTask(std::string const& name, std::string const& queueName = StandardTasks::internal) {
        tasks[queueName]->pushBack(name);
    }

    Constants::Event addScript(std::string const& filename, Utility::IO::Capture& capture) {
        if (filename.length() < 6 || !filename.ends_with(".nebs")) {
            capture.error.println("Warning: unexpected file ending for task file '", filename, "'. Expected '.nebs'. Trying to load anyway.");
        }

        std::string fileContent = Utility::IO::FileManagement::LoadFile(filename);

        // Replace all "\n " with "\n" to allow for multi-line commands with leading spaces
        while (fileContent.find("\n ") != std::string::npos) {
            fileContent = Utility::StringHandler::replaceAll(fileContent, "\n ", "\n");
        }

        // Replace all " \\\n" with "\\\n" to allow for multi-line commands with trailing spaces
        while (fileContent.find(" \\\n") != std::string::npos) {
            fileContent = Utility::StringHandler::replaceAll(fileContent, " \\\n", "\\\n");
        }

        // Replace all "\\n" with an empty string to allow for multi-line commands in a single line
        auto constexpr toReplace = "\\\n";
        fileContent = Utility::StringHandler::replaceAll(fileContent, toReplace, "");

        // Split std::string file into lines and remove comments
        std::vector<std::string> lines;
        std::istringstream stream(fileContent);
        std::string line;
        while (std::getline(stream, line)) {
            std::string_view lineView(line);
            Utility::StringHandler::untilSpecialChar(lineView, '#'); // Remove comments.
            Utility::StringHandler::lStrip(lineView, ' '); // Remove whitespaces at start
            Utility::StringHandler::rStrip(lineView, ' '); // Remove whitespaces at end
            if (lineView.empty()) {
                continue;
            }
            // Insert line backwards, so we can process them in the order they were written later on:
            lines.insert(lines.begin(), std::string(lineView));
        }

        // Now insert all lines into the task queue
        for (auto const& taskLine : lines) {
            tasks[StandardTasks::script]->pushFront(taskLine);
        }
        return Constants::Event::Success;
    }

    void clearAllTaskQueues() {
        std::ranges::for_each(tasks | std::views::values, [](auto &tq) {
            if (tq) tq->clear();
        });
    }

    bool clearTaskQueue(std::string const& queueName) {
        if (auto const it = tasks.find(queueName); it != tasks.end()) {
            it->second->clear();
            return true;
        }
        return false;
    }

    void decrementScriptWaitCounter() {
        tasks[StandardTasks::script]->decrementWaitCounter();
    }

    void decrementWaitCounter() {
        for (auto const& tq : std::views::values(tasks)) {
            tq->decrementWaitCounter();
        }
    }

    Constants::Event parse(Domain& domain, Data::JsonScope& scope, bool const& recover) {
        Context ctx{domain, domain, domain};
        ContextScope ctxScope{scope, scope, scope};
        return parse(ctx, ctxScope, recover);
    }

    Constants::Event parse(Context& ctx, ContextScope& ctxScope, bool const& recover) {
        queueResult.clear();
        for (auto const& [name, queue] : tasks) {
            queueResult[name] = queue->resolve(ctx, ctxScope, recover);
            if (queueResult[name].encounteredCriticalResult && !recover) {
                return queueResult[name].events.back();
            }
        }
        return Constants::Event::Success;
    }

private:
    /**
     * @brief Contains task queues for different types of tasks.
     */
    absl::flat_hash_map<std::string,std::shared_ptr<Data::TaskQueue>> tasks;

    /**
     * @brief Contains results of the last task queue resolutions.
     */
    absl::flat_hash_map<std::string, Data::TaskQueueResult> queueResult;
};

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_TASKS_HPP
