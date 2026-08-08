//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/TaskQueue.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/General.hpp"
#include "Nebulite/Utility/Convert/Cast.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

//------------------------------------------
// Update
Constants::Event General::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event General::exit() const {
    // Clear all task queues to prevent further execution
    domain.tasks.clearAllTaskQueues();

    // Set the renderer to quit
    domain.quitRenderer();
    return Constants::Event::success;
}

Constants::Event General::wait(std::span<std::string_view const> args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    auto const count = Utility::Convert::Cast::String::to<std::size_t>(args[1]);
    if (!count.has_value()) {
        return Constants::StandardCapture::Warning::Functional::invalidArgument(domain.capture);
    }
    domain.tasks.incrementScriptWaitCounter(count.value());
    return Constants::Event::success;
}

Constants::Event General::task(std::span<std::string_view const> args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    auto const fileName = Utility::StringHandler::recombineArgs(args.subspan(1));
    domain.capture.log.println("Loading task list from file: ", fileName);
    domain.tasks.addScript(fileName, domain.capture);
    return Constants::Event::success;
}

Constants::Event General::taskExec(std::span<std::string_view const> const args, Interaction::Context ctx, Interaction::ContextScope ctxScope) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    auto const fileName = Utility::StringHandler::recombineArgs(args.subspan(1));
    domain.capture.log.println("Loading task list from file and executing immediately: ", fileName);
    Data::TaskQueue tq("LocalTaskQueue", false);
    tq.addScript(fileName, domain.capture);
    return tq.resolve(ctx, ctxScope, true).worstEvent();
}

Constants::Event General::always(std::span<std::string_view const> const args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    // Split on ';' and push each trimmed command
    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    std::stringstream ss(argStr);
    std::string command;
    while (std::getline(ss, command, ';')) {
        // Trim whitespace from each command
        command.erase(0, command.find_first_not_of(" \t"));
        command.erase(command.find_last_not_of(" \t") + 1);
        if (!command.empty()) {
            domain.tasks.addTask(command, Interaction::Execution::Tasks::StandardTasks::always);
        }
    }
    return Constants::Event::success;
}

Constants::Event General::alwaysClear() const {
    domain.tasks.clearTaskQueue(Interaction::Execution::Tasks::StandardTasks::always);
    return Constants::Event::success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
