//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <memory>
#include <ranges>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/TaskQueue.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Execution/Tasks.hpp"

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

void Tasks::incrementScriptWaitCounter(std::size_t const count) {
    tasks[StandardTasks::script]->incrementWaitCounter(count);
}

/**
 * @brief Gets a specific task queue by name.
 * @param name The name of the task queue.
 * @return Pointer to the TaskQueue instance, or nullptr if not found.
 */
std::shared_ptr<Data::TaskQueue> Tasks::getTaskQueue(std::string_view const name) {
    if (auto const it = tasks.find(name); it != tasks.end()) {
        return it->second;
    }
    return nullptr;
}

void Tasks::addTask(std::string_view const name, std::string_view const queueName) {
    tasks[queueName]->pushBack(name);
}

void Tasks::addScript(std::string_view const filename, Utility::IO::Capture& capture) {
    tasks[StandardTasks::script]->addScript(filename, capture);
}

void Tasks::clearAllTaskQueues() {
    std::ranges::for_each(tasks | std::views::values, [](auto &tq) {
        if (tq) tq->clear();
    });
}

bool Tasks::clearTaskQueue(std::string_view const queueName) {
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

Constants::Event Tasks::parse(Domain& domain, Data::JsonScope& scope, bool const recover) {
    Context ctx{domain, domain, domain};
    ContextScope ctxScope{scope, scope, scope};
    return parse(ctx, ctxScope, recover);
}

Constants::Event Tasks::parse(Context& ctx, ContextScope& ctxScope, bool const recover) {
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
