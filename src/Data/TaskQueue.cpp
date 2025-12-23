#include "Data/TaskQueue.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Data {

TaskQueueResult TaskQueue::resolve(Interaction::Execution::DomainBase& context, bool const& recover) {
    Constants::Error currentResult;
    Data::TaskQueueResult fullResult;

    if (state.waitCounter > 0)
        return fullResult;

    // 1.) Process and pop tasks
    if (settings.clearAfterResolving) {
        while (!tasks.list.empty()) {
            // Check stop conditions
            if (fullResult.encounteredCriticalResult && !recover)
                break;

            // Pop front
            std::string argStr = tasks.list.front();
            tasks.list.pop_front();

            // Add binary name if missing
            if (!argStr.starts_with(settings.callbackName + " ")) {
                argStr.insert(0, settings.callbackName + " ");
            }

            // Parse
            currentResult = context.parseStr(argStr);

            // Check result
            if (currentResult.isCritical()) {
                fullResult.encounteredCriticalResult = true;
            }
            fullResult.errors.push_back(currentResult);
        }
    }
    // 2.) Process without popping tasks
    else {
        for (auto const& argStrOrig : tasks.list) {
            // Check stop conditions
            if (fullResult.encounteredCriticalResult && !recover)
                break;

            // Add binary name if missing
            std::string argStr = argStrOrig;
            if (!argStr.starts_with(settings.callbackName + " ")) {
                argStr.insert(0, settings.callbackName + " ");
            }

            // Parse
            currentResult = context.parseStr(argStr);

            // Check result
            if (currentResult.isCritical()) {
                fullResult.encounteredCriticalResult = true;
            }
            fullResult.errors.push_back(currentResult);
        }
    }
    return fullResult;
}

void TaskQueue::pushBack(std::string const& task) {
    std::lock_guard<std::mutex> lock(tasks.mutex);
    tasks.list.push_back(task);
}

void TaskQueue::pushFront(std::string const& task) {
    std::lock_guard<std::mutex> lock(tasks.mutex);
    tasks.list.push_front(task);
}

void TaskQueue::wait(uint64_t const& frames) {
    state.waitCounter += frames;
}

void TaskQueue::clear() {
    tasks.list.clear();
    state.waitCounter = 0;
}

void TaskQueue::incrementWaitCounter(uint64_t const& increment) {
    state.waitCounter += increment;
}

void TaskQueue::decrementWaitCounter(uint64_t const& decrement) {
    if (state.waitCounter >= decrement) {
        state.waitCounter -= decrement;
    } else {
        state.waitCounter = 0;
    }
}

bool TaskQueue::isWaiting() const {
    return state.waitCounter > 0;
}

} // namespace Nebulite::Data