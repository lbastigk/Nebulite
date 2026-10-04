//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Data/TaskQueue.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Execution/Tasks.hpp"
#include "Nebulite/Module/Domain/Common/Tasks.hpp"
#include "Nebulite/Utility/Args/CmdArgs.hpp"
#include "Nebulite/Utility/Convert/Cast.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

//------------------------------------------
// Update

Constants::Event Tasks::updateHook() {
    return Constants::Event::success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event Tasks::wait(Utility::Args::SSV const& args) const {
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

Constants::Event Tasks::task(Utility::Args::SSV const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    auto const fileName = args.recombineSubspan(1);
    domain.capture.log.println("Loading task list from file: ", fileName);
    domain.tasks.addScript(fileName, domain.capture);
    return Constants::Event::success;
}

Constants::Event Tasks::taskExec(Utility::Args::SSV const& args, Interaction::Context ctx, Interaction::ContextScope ctxScope) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    auto const fileName = args.recombineSubspan(1);
    domain.capture.log.println("Loading task list from file and executing immediately: ", fileName);
    Data::TaskQueue tq("LocalTaskQueue", false);
    tq.addScript(fileName, domain.capture);
    return tq.resolve(ctx, ctxScope, true).worstEvent();
}

Constants::Event Tasks::always(Utility::Args::SSV const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    // Split on ';' and push each trimmed command
    std::string const argStr = args.recombineSubspan(1);
    std::stringstream ss(argStr);
    std::string command;
    while (std::getline(ss, command, ';')) {
        // Trim whitespace from each command
        command.erase(0, command.find_first_not_of(" \t"));
        if (command.empty()) {
            continue;
        }
        command.erase(command.find_last_not_of(" \t") + 1);
        if (!command.empty()) {
            domain.tasks.addTask(command, Interaction::Execution::Tasks::StandardTasks::always);
        }
    }
    return Constants::Event::success;
}

Constants::Event Tasks::alwaysClear() const {
    domain.tasks.clearTaskQueue(Interaction::Execution::Tasks::StandardTasks::always);
    return Constants::Event::success;
}

} // namespace Nebulite::Module::Domain::Common

