#include "DomainModule/GlobalSpace/General.hpp"
#include "Core/GlobalSpace.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

//------------------------------------------
// Update
Constants::Error General::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Constants::Error General::eval(int argc, char** argv) {
    // argc/argv to string for evaluation
    std::string const args = Utility::StringHandler::recombineArgs(argc, argv);

    // Evaluate expression
    Core::JsonScope emptyDoc;
    Interaction::ContextBase context{emptyDoc,emptyDoc,Nebulite::global()};
    std::string const argsEvaluated = Interaction::Logic::Expression::eval(args,context);

    // reparse
    return domain.parseStr(argsEvaluated);
}

Constants::Error General::exit() {
    // Clear all task queues to prevent further execution
    domain.clearAllTaskQueues();

    // Set the renderer to quit
    domain.quitRenderer();
    return Constants::ErrorTable::NONE();
}

Constants::Error General::wait(int argc, char** argv) {
    if (argc == 2) {
        // Standard wait acts on taskQueue "script"
        domain.getTaskQueue(Nebulite::Core::GlobalSpace::StandardTasks::script)->incrementWaitCounter(std::stoull(argv[1]));
        return Constants::ErrorTable::NONE();
    }
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
}

Constants::Error General::task(int argc, char** argv) {
    std::string const message = "Loading task list from file: " + (argc > 1 ? std::string(argv[1]) : "none");
    Nebulite::cout() << message << Nebulite::endl;

    // Rollback RNG, loading a task file should not change the RNG state
    domain.rngRollback();

    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    // Warn if file ending is not .nebs
    std::string const filename = argv[1];
    if (filename.length() < 6 || !filename.ends_with(".nebs")) {
        Nebulite::cerr() << "Warning: unexpected file ending for task file '" << filename << "'. Expected '.nebs'. Trying to load anyway." << Nebulite::endl;
    }

    // Using FileManagement to load the .nebs file
    std::string const file = Utility::FileManagement::LoadFile(filename);
    if (file.empty()) {
        Nebulite::cerr() << "Error: " << argv[0] << " Could not open file '" << filename << "'" << Nebulite::endl;
        return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }

    std::vector<std::string> lines;

    // Split std::string file into lines and remove comments
    std::istringstream stream(file);
    std::string line;
    while (std::getline(stream, line)) {
        line = Utility::StringHandler::untilSpecialChar(line, '#'); // Remove comments.
        line = Utility::StringHandler::lStrip(line, ' '); // Remove whitespaces at start
        if (line.empty()) {
            // line is empty
            continue;
        }
        // Insert line backwards, so we can process them in the order they were written later on:
        lines.insert(lines.begin(), line);
    }

    // Now insert all lines into the task queue
    for (auto const& taskLine : lines) {
        domain.getTaskQueue(Nebulite::Core::GlobalSpace::StandardTasks::script)->pushFront(taskLine);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::echo(int argc, char** argv) {
    std::string const args = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    Nebulite::cout() << args << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

Constants::Error General::func_if(int argc, char** argv) {
    if (argc < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    if (!Interaction::Logic::Expression::evalAsBool(argv[1])) {
        // If the condition is false/nan, skip the following commands
        return Constants::ErrorTable::NONE();
    }

    // Build the command string from rest
    std::string commands = Utility::StringHandler::recombineArgs(argc - 2, argv + 2);
    commands = __FUNCTION__ + std::string(" ") + commands;
    return domain.parseStr(commands);
}

Constants::Error General::func_assert(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    if (argc > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string const condition = argv[1];

    // condition must start with $( and end with )
    if (condition.front() != '$' || condition[1] != '(' || condition.back() != ')') {
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }

    // Evaluate condition
    if (!Interaction::Logic::Expression::evalAsBool(condition)) {
        return Constants::ErrorTable::addError("Critical Error: A custom assertion failed.\nAssertion failed: " + condition + " is not true.", Constants::Error::CRITICAL);
    }

    // All good
    return Constants::ErrorTable::NONE();
}

Constants::Error General::func_return(int argc, char** argv) {
    return Constants::ErrorTable::addError(Utility::StringHandler::recombineArgs(argc - 1, argv + 1), Constants::Error::CRITICAL);
}

Constants::Error General::always(int argc, char** argv) {
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1)
                oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string argStr = oss.str();
        std::stringstream ss(argStr);
        std::string command;

        while (std::getline(ss, command, ';')) {
            // Trim whitespace from each command
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);
            if (!command.empty()) {
                domain.getTaskQueue(Nebulite::Core::GlobalSpace::StandardTasks::always)->pushBack(command);
            }
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::alwaysClear() {
    domain.getTaskQueue(Nebulite::Core::GlobalSpace::StandardTasks::always)->clear();
    return Constants::ErrorTable::NONE();
}

Constants::Error General::func_for(int argc, char** argv) {
    if (argc > 4) {
        std::string const varName = argv[1];

        int const iStart = std::stoi(Interaction::Logic::Expression::eval(argv[2]));
        int const iEnd = std::stoi(Interaction::Logic::Expression::eval(argv[3]));

        std::string args;
        for (int i = 4; i < argc; ++i) {
            args += argv[i];
            if (i < argc - 1) {
                args += " ";
            }
        }
        for (int i = iStart; i <= iEnd; i++) {
            // for + args
            std::string args_replaced = std::string(argv[0]) + " " + Utility::StringHandler::replaceAll(args, '{' + varName + '}', std::to_string(i));
            if (auto const err = domain.parseStr(args_replaced); err.isCritical()) {
                return err;
            } else if (err.isError()) {
                Nebulite::cout() << err.getDescription() << Nebulite::endl;
            }
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::nop() {
    // Do nothing
    return Constants::ErrorTable::NONE();
}

Constants::Error General::inScope(std::span<std::string const> const& args) {
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    // A bit whacky, as we use the global scope for this instead of what is shared with this DomainModule
    // But this is the only way to get a full JsonScope with domain functionality
    std::string const scope = args[1];
    auto const& s = Nebulite::globalDoc().shareScope(*this);
    std::string const& cmd = std::string(__FUNCTION__) + std::string(" ") + Utility::StringHandler::recombineArgs(args.subspan(2));
    return s.parseStr(cmd);
}

} // namespace Nebulite::DomainModule::GlobalSpace
