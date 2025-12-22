#include "Data/TaskQueue.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Data {

TaskQueueResult TaskQueue::resolve(Interaction::Execution::DomainBase& context, bool const& recover) {
    Constants::Error currentResult;
    Data::TaskQueueResult fullResult;

    // 1.) Process and pop tasks
    if (settings.clearAfterResolving) {
        while (!tasks.list.empty()) {
            // Check stop conditions
            if (fullResult.encounteredCriticalResult && !recover)
                break;
            if (state.waitCounter > 0)
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
            if (state.waitCounter > 0)
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

} // namespace Nebulite::Data