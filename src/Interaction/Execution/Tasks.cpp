//------------------------------------------
// Includes

// Standard library
#include <memory>
// NOLINTNEXTLINE
#include <cstddef>
#include <sstream>
#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"
#include "Data/TaskQueue.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Tasks.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {

Tasks::Tasks() {
    tasks[StandardTasks::always] = std::make_shared<Data::TaskQueue>(StandardTasks::always, false);
    tasks[StandardTasks::internal] = std::make_shared<Data::TaskQueue>(StandardTasks::internal, true);
    tasks[StandardTasks::script] = std::make_shared<Data::TaskQueue>(StandardTasks::script, true);
}

bool Tasks::scriptIsWaiting() {
    return tasks[StandardTasks::script]->isWaiting();
}

void Tasks::incrementScriptWaitCounter(size_t const& count) {
    tasks[StandardTasks::script]->incrementWaitCounter(count);
}

/**
 * @brief Gets a specific task queue by name.
 * @param name The name of the task queue.
 * @return Pointer to the TaskQueue instance, or nullptr if not found.
 */
std::shared_ptr<Data::TaskQueue> Tasks::getTaskQueue(std::string const& name) {
    if (auto const it = tasks.find(name); it != tasks.end()) {
        return it->second;
    }
    return nullptr;
}

void Tasks::addTask(std::string const& name, std::string const& queueName) {
    tasks[queueName]->pushBack(name);
}

Constants::Event Tasks::addScript(std::string const& filename, Utility::IO::Capture& capture) {
    if (filename.length() < 6 || !filename.ends_with(".nebs")) {
        capture.error.println("Warning: unexpected file ending for task file '", filename, "'. Expected '.nebs'. Trying to load anyway.");
    }

    std::string fileContent = Utility::IO::FileManagement::LoadFile(filename);

    // Replace all "\n " with "\n" to allow for multi-line commands with leading spaces
    while (fileContent.contains("\n ")) {
        fileContent = Utility::StringHandler::replaceAll(fileContent, "\n ", "\n");
    }

    // Replace all " \\\n" with "\\\n" to allow for multi-line commands with trailing spaces
    while (fileContent.contains(" \\\n")) {
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

void Tasks::clearAllTaskQueues() {
    std::ranges::for_each(tasks | std::views::values, [](auto &tq) {
        if (tq) tq->clear();
    });
}

bool Tasks::clearTaskQueue(std::string const& queueName) {
    if (auto const it = tasks.find(queueName); it != tasks.end()) {
        it->second->clear();
        return true;
    }
    return false;
}

void Tasks::decrementScriptWaitCounter() {
    tasks[StandardTasks::script]->decrementWaitCounter();
}

void Tasks::decrementWaitCounter() {
    for (auto const& tq : std::views::values(tasks)) {
        tq->decrementWaitCounter();
    }
}

Constants::Event Tasks::parse(Domain& domain, Data::JsonScope& scope, bool const& recover) {
    Context ctx{domain, domain, domain};
    ContextScope ctxScope{scope, scope, scope};
    return parse(ctx, ctxScope, recover);
}

Constants::Event Tasks::parse(Context& ctx, ContextScope& ctxScope, bool const& recover) {
    queueResult.clear();
    for (auto const& [name, queue] : tasks) {
        queueResult[name] = queue->resolve(ctx, ctxScope, recover);
        if (queueResult[name].encounteredCriticalResult && !recover) {
            return queueResult[name].events.back();
        }
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::Interaction::Execution
