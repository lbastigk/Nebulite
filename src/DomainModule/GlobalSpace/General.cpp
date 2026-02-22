#include "Nebulite.hpp"
#include "ScopeAccessor.hpp"
#include "Core/GlobalSpace.hpp"
#include "Core/JsonScope.hpp"
#include "DomainModule/GlobalSpace/General.hpp"
#include "Utility/FileManagement.hpp"

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

// NOLINTNEXTLINE
Constants::Error General::eval(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope){
    // TODO: An idea would be to only eval until the next "eval" keyword, allowing for nested evals within for-loops, ifs, etc.:
    //       Example:
    //       eval for i 1 {global.loopCount} eval process-state {global.currentState} {i}
    //       This way, the first eval will not vanish the information within the for-loop,
    //       allowing us to properly retrieve the current state for each iteration.
    //       Do the same for the RenderObject eval function. Perhaps we should combine them?

    // argc/argv to string for evaluation
    std::string const argStr = Utility::StringHandler::recombineArgs(args);

    // Evaluate expression, empty context for self and other
    std::string const argsEvaluated = Interaction::Logic::Expression::eval(argStr);

    // reparse
    (void)callerScope; // Unused parameter
    return caller.parseStr(argsEvaluated);
}

Constants::Error General::exit() const {
    // Clear all task queues to prevent further execution
    domain.clearAllTaskQueues();

    // Set the renderer to quit
    domain.quitRenderer();
    return Constants::ErrorTable::NONE();
}

Constants::Error General::wait(int const argc, char** argv) const {
    if (argc == 2) {
        // Standard wait acts on taskQueue "script"
        domain.getTaskQueue(Core::GlobalSpace::StandardTasks::script)->incrementWaitCounter(std::stoull(argv[1]));
        return Constants::ErrorTable::NONE();
    }
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
}

Constants::Error General::task(int const argc, char** argv) const {
    Log::println("Loading task list from file: ", argc > 1 ? std::string(argv[1]) : "none");

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
        Error::println("Warning: unexpected file ending for task file '", filename, "'. Expected '.nebs'. Trying to load anyway.");
    }

    // Using FileManagement to load the .nebs file
    std::string file = Utility::FileManagement::LoadFile(filename);
    if (file.empty()) {
        Error::println("Error: ", argv[0], " Could not open file '", filename, "'.");
        return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }

    // Replace all "\n " with "\n" to allow for multi-line commands with leading spaces
    while (file.find("\n ") != std::string::npos) {
        file = Utility::StringHandler::replaceAll(file, "\n ", "\n");
    }

    // Replace all " \\\n" with "\\\n" to allow for multi-line commands with trailing spaces
    while (file.find(" \\\n") != std::string::npos) {
        file = Utility::StringHandler::replaceAll(file, " \\\n", "\\\n");
    }

    // Replace all "\\n" with an empty string to allow for multi-line commands in a single line
    auto constexpr toReplace = "\\\n";
    file = Utility::StringHandler::replaceAll(file, toReplace, "");

    // Split std::string file into lines and remove comments
    std::vector<std::string> lines;
    std::istringstream stream(file);
    std::string line;
    while (std::getline(stream, line)) {
        line = Utility::StringHandler::untilSpecialChar(line, '#'); // Remove comments.
        line = Utility::StringHandler::lStrip(line, ' '); // Remove whitespaces at start
        line = Utility::StringHandler::rStrip(line, ' '); // Remove whitespaces at end
        if (line.empty()) {
            // line is empty
            continue;
        }
        // Insert line backwards, so we can process them in the order they were written later on:
        lines.insert(lines.begin(), line);
    }

    // Now insert all lines into the task queue
    for (auto const& taskLine : lines) {
        domain.getTaskQueue(Core::GlobalSpace::StandardTasks::script)->pushFront(taskLine);
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::echo(std::span<std::string const> const& args) {
    Log::println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error General::func_if(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope) {
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    if (!Interaction::Logic::Expression::evalAsBool(args[1])) {
        // If the condition is false/nan, skip the following commands
        return Constants::ErrorTable::NONE();
    }

    // Build the command string from rest
    std::string commands = Utility::StringHandler::recombineArgs(args.subspan(2));
    commands = __FUNCTION__ + std::string(" ") + commands;
    (void)callerScope; // Unused parameter
    return caller.parseStr(commands);
}

Constants::Error General::func_assert(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string const& condition = args[1];

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

Constants::Error General::func_return(std::span<std::string const> const& args) {
    return Constants::ErrorTable::addError(Utility::StringHandler::recombineArgs(args.subspan(1)), Constants::Error::CRITICAL);
}

Constants::Error General::always(int argc, char** argv) const {
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
                domain.getTaskQueue(Core::GlobalSpace::StandardTasks::always)->pushBack(command);
            }
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::alwaysClear() const {
    domain.getTaskQueue(Core::GlobalSpace::StandardTasks::always)->clear();
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error General::func_for(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope) {
    if (args.size() > 4) {
        std::string const& varName = args[1];

        int const iStart = std::stoi(Interaction::Logic::Expression::eval(args[2]));
        int const iEnd = std::stoi(Interaction::Logic::Expression::eval(args[3]));

        std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(4));
        for (int i = iStart; i <= iEnd; i++) {
            // for + args
            std::string args_replaced = std::string(args[0]) + " " + Utility::StringHandler::replaceAll(argStr, '{' + varName + '}', std::to_string(i));
            (void)callerScope; // Unused parameter
            if (auto const err = caller.parseStr(args_replaced); err.isCritical()) {
                return err;
            } else if (err.isError()) {
                Error::println(err.getDescription());
            }
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::nop(std::span<std::string const> const& /*args*/) {
    // Do nothing
    return Constants::ErrorTable::NONE();
}

Constants::Error General::inScope(std::span<std::string const> const& args) const {
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    // A bit whacky, as we use the global scope for this instead of what is shared with this DomainModule
    // But this is the only way to get a full JsonScope with domain functionality
    std::string const& scope = args[1];
    auto const access = getDomainModuleAccessToken(*this);
    auto& s = Global::shareScope(access).shareScope(scope);
    std::string const& cmd = std::string(__FUNCTION__) + std::string(" ") + Utility::StringHandler::recombineArgs(args.subspan(2));
    return s.parseStr(cmd);
}

} // namespace Nebulite::DomainModule::GlobalSpace
