//------------------------------------------
// Includes

// Standard library
#include <sstream>
#include <string>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Core/GlobalSpace.hpp"
#include "Module/Domain/GlobalSpace/General.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

//------------------------------------------
// Update
Constants::Event General::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event General::exit() const {
    // Clear all task queues to prevent further execution
    domain.tasks.clearAllTaskQueues();

    // Set the renderer to quit
    domain.quitRenderer();
    return Constants::Event::Success;
}

Constants::Event General::wait(int const argc, char** argv) const {
    if (argc == 2) {
        domain.tasks.incrementScriptWaitCounter(std::stoull(argv[1]));
        return Constants::Event::Success;
    }
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
}

Constants::Event General::task(int const argc, char** argv) const {
    domain.capture.log.println("Loading task list from file: ", argc > 1 ? std::string(argv[1]) : "none");

    // Rollback RNG, loading a task file should not change the RNG state
    domain.rngRollback();

    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (argc > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    // Warn if file ending is not .nebs
    std::string const& filename = argv[1];
    return domain.tasks.addScript(filename, domain.capture);
}

Constants::Event General::always(int argc, char** argv) const {
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1)
                oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string const argStr = oss.str();
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
    }
    return Constants::Event::Success;
}

Constants::Event General::alwaysClear() const {
    domain.tasks.clearTaskQueue(Interaction::Execution::Tasks::StandardTasks::always);
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
