//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

// Nebulite
#include "Constants/Event.hpp"
#include "Data/TaskQueue.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/IO/FileManagement.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Data {

TaskQueue::TaskQueueResult TaskQueue::resolve(Interaction::Context& ctx, Interaction::ContextScope& ctxScope, bool const& recover) {
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
        if (currentResult != Constants::Event::Success) {
            fullResult.events.push_back(currentResult);
        }
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

void TaskQueue::addScript(std::string const& filename, Utility::IO::Capture& capture){
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
        pushFront(taskLine);
    }
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
