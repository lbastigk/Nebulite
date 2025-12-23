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

// NOLINTNEXTLINE
Constants::Error General::eval(int argc, char** argv) {
    // argc/argv to string for evaluation
    std::string const args = Utility::StringHandler::recombineArgs(argc, argv);

    // Evaluate expression
    Data::JSON emptyDoc;
    Interaction::ContextBase context{emptyDoc,emptyDoc,Nebulite::global()};
    std::string const argsEvaluated = Interaction::Logic::Expression::eval(args,context);

    // reparse
    return domain->parseStr(argsEvaluated);
}

std::string const General::eval_name = "eval";
std::string const General::eval_desc = R"(Evaluates an expression string and executes it.
Every argument after eval is concatenated with a whitespace to form the expression to be evaluated and then reparsed.

Usage: eval <expression>

Examples:

eval echo $(1+1)
outputs: 2.000000
First, eval evaluates every argument, then concatenates them with a whitespace,
and finally executes the resulting string as a command.
The string 'echo $(1+1)' is evaluated to "echo 2.000000", which is then executed.

eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
This evaluates to 'spawn ./Resources/RenderObjects/NAME.json',
where NAME is the current value of the global variable ToSpawn
)";

// NOLINTNEXTLINE
Constants::Error General::exit(int argc, char** argv) {
    // Clear all task queues to prevent further execution
    domain->clearAllTaskQueues();

    // Set the renderer to quit
    domain->quitRenderer();
    return Constants::ErrorTable::NONE();
}

std::string const General::exit_name = "exit";
std::string const General::exit_desc = R"(Exits the entire program.

Usage: exit

Closes the program with exit code 0 (no error)
Any queued tasks will be discarded.
)";

// NOLINTNEXTLINE
Constants::Error General::wait(int argc, char** argv) {
    if (argc == 2) {
        // Standard wait acts on taskQueue "script"
        domain->getTaskQueue(Nebulite::Core::GlobalSpace::StandardTasks::script)->incrementWaitCounter(std::stoull(argv[1]));
        return Constants::ErrorTable::NONE();
    }
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
}

std::string const General::wait_name = "wait";
std::string const General::wait_desc = R"(Sets the waitCounter to the given value to halt all script tasks for a given amount of frames.

Usage: wait <frames>

This command pauses the execution of all script tasks for the specified number of frames.
This does not halt any tasks coming from objects within the environment and cannot be used by them.

The wait-command is intended for scripts only, allowing for timed delays between commands.

This is useful for:
- Creating pauses in scripts to wait for certain conditions to be met.
- Timing events in a sequence.
- Tool assisted speedruns (TAS)
)";

// NOLINTNEXTLINE
Constants::Error General::task(int argc, char** argv) {
    std::string const message = "Loading task list from file: " + (argc > 1 ? std::string(argv[1]) : "none");
    Nebulite::cout() << message << Nebulite::endl;

    // Rollback RNG, loading a task file should not change the RNG state
    domain->rngRollback();

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
        domain->getTaskQueue(Nebulite::Core::GlobalSpace::StandardTasks::script)->pushFront(taskLine);
    }
    return Constants::ErrorTable::NONE();
}

std::string const General::task_name = "task";
std::string const General::task_desc = R"(Loads tasks from a file into the taskQueue.

Usage: task <filename>

This command loads a list of tasks from the specified file into the task queue.
Each line in the file is treated as a separate task.

Task files are not appended at the end, but right after the current task.
This ensures that tasks can be loaded within task files themselves and being executed immediately.

This example shows how tasks are loaded and executed:

Main task:
    mainCommand1
    mainCommand2
    task subTaskFile.txt:
        subCommand1
        subCommand2
    mainCommand4
)";

// NOLINTNEXTLINE
Constants::Error General::echo(int argc, char** argv) {
    std::string const args = Utility::StringHandler::recombineArgs(argc - 1, argv + 1);
    Nebulite::cout() << args << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

std::string const General::echo_name = "echo";
std::string const General::echo_desc = R"(Echoes all arguments as string to the standard output.

Usage: echo <string>

This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).
Example:
./bin/Nebulite echo Hello World!
Outputs:
Hello World!
)";

// NOLINTNEXTLINE
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
    return domain->parseStr(commands);
}

std::string const General::func_if_name = "if";
std::string const General::func_if_desc = R"(Executes a block of code if a condition is true.

Usage: if <condition> <functioncall>

It is recommended to wrap the condition in quotes to prevent parsing issues.

However, This is not supported for in-console usage.
This is because the console itself removes quotes before passing the arguments to the FuncTree,
rendering them useless.

Example:
if '$(eq(1+1,2))' echo Condition is true!
)";

// NOLINTNEXTLINE
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

std::string const General::assert_name = "assert";
std::string const General::assert_desc = R"(Asserts a condition and throws a custom error if false.

Usage: assert <condition>

It is recommended to wrap the condition in quotes to prevent parsing issues.

Example:
assert '$(eq(1+1,2))'    // No error
assert '$(eq(1+1,3))'    // Critical Error: A custom assertion failed.
Assertion failed: $(eq(1+1,3)) is not true.
)";

// NOLINTNEXTLINE
Constants::Error General::func_return(int argc, char** argv) {
    return Constants::ErrorTable::addError(Utility::StringHandler::recombineArgs(argc - 1, argv + 1), Constants::Error::CRITICAL);
}

std::string const General::func_return_name = "return";
std::string const General::func_return_desc = R"(Returns a custom value as a Critical Error.

Usage: return <string>

This command creates a custom critical error with the given string as description.
This can be used to exit from a task queue with a custom message.

Example:

./bin/Nebulite return We did not anticipate this happening, weird.
Outputs:
We did not anticipate this happening, weird.
Critical Error: We did not anticipate this happening, weird.
)";

// NOLINTNEXTLINE
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
                domain->getTaskQueue(Nebulite::Core::GlobalSpace::StandardTasks::always)->pushBack(command);
            }
        }
    }
    return Constants::ErrorTable::NONE();
}

std::string const General::always_name = "always";
std::string const General::always_desc = R"(Attach a command to the always-taskqueue that is executed on each tick.

Usage: always <command>

Example:
always echo This command runs every frame!
This will output "This command runs every frame!" on every frame.
)";

// NOLINTNEXTLINE
Constants::Error General::alwaysClear(int argc, char** argv) {
    domain->getTaskQueue(Nebulite::Core::GlobalSpace::StandardTasks::always)->clear();
    return Constants::ErrorTable::NONE();
}

std::string const General::alwaysClear_name = "always-clear";
std::string const General::alwaysClear_desc = R"(Clears the entire always-taskqueue.

Usage: always-clear

Example:
always-clear
This will remove all commands from the always-taskqueue.
)";

// NOLINTNEXTLINE
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
            if (auto const err = domain->parseStr(args_replaced); err.isCritical()) {
                return err;
            } else if (err.isError()) {
                Nebulite::cout() << err.getDescription() << Nebulite::endl;
            }
        }
    }
    return Constants::ErrorTable::NONE();
}

std::string const General::func_for_name = "for";
std::string const General::func_for_desc = R"(Executes a for-loop with a function call.

Usage: for <var> <start> <end> <functioncall>

Example:
for i 1 5 echo Iteration {i}
This will output:
    Iteration 1
    Iteration 2
    Iteration 3
    Iteration 4
    Iteration 5

This is useful for:
- Repeating actions a specific number of times.
- Iterating over a range of values.
- Creating complex control flows in scripts.
)";

// NOLINTNEXTLINE
Constants::Error General::nop(std::span<std::string const> const& args) {
    // Do nothing
    return Constants::ErrorTable::NONE();
}

std::string const General::nop_name = "nop";
std::string const General::nop_desc = R"(No operation. Does nothing.
Usage: nop <blind arguments>

Useful for testing or as a placeholder in scripts where no action is required,
but a command is syntactically necessary.)";

} // namespace Nebulite::DomainModule::GlobalSpace
