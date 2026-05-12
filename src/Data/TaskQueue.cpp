//------------------------------------------
// Includes

// Standard library
// NOLINTNEXTLINE
#include <cstdint>
#include <mutex>
#include <string>

// Nebulite
#include "Constants/Event.hpp"
#include "Data/TaskQueue.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
namespace Nebulite::Data {

TaskQueueResult TaskQueue::resolve(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, bool const& recover) {
    Constants::Event currentResult{};
    TaskQueueResult fullResult;

    auto resolve = [&](std::string argStr) {
        // Add binary name if missing
        if (!argStr.starts_with(settings.callbackName + " ")) {
            argStr.insert(0, settings.callbackName + " ");
        }

        // Parse
        currentResult = ctx.self.parseStr(argStr, ctx, ctxScope);

        // Check result
        if (currentResult == Constants::Event::Error) {
            fullResult.encounteredCriticalResult = true;
        }
        fullResult.events.push_back(currentResult);
    };

    // 1.) Process and pop tasks
    if (settings.clearAfterResolving) {
        while (!tasks.list.empty()) {
            // Check stop conditions on each iteration,
            // as they might have changed during parsing
            if (fullResult.encounteredCriticalResult && !recover)
                break;
            if (state.waitCounter > 0)
                return fullResult;

            // Pop front
            std::string const argStr = tasks.list.front();
            tasks.list.pop_front();

            resolve(argStr);
        }
    }
    // 2.) Process without popping tasks
    else {
        for (auto const& argStr : tasks.list) {
            // Check stop conditions on each iteration,
            // as they might have changed during parsing
            if (fullResult.encounteredCriticalResult && !recover)
                break;
            if (state.waitCounter > 0)
                return fullResult;

            resolve(argStr);
        }
    }
    return fullResult;
}

void TaskQueue::pushBack(std::string const& task) {
    std::scoped_lock const lock(tasks.mutex);
    tasks.list.push_back(task);
}

void TaskQueue::pushFront(std::string const& task) {
    std::scoped_lock const lock(tasks.mutex);
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
