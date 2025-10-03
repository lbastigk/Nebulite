/**
 * @file GDM_General.hpp
 * 
 * This file contains the DomainModule of the GlobalSpace for general-purpose functions.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::General
 * @brief DomainModule for general-purpose functions within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, General) {
public:
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Evaluates an expression string and executes it
     * 
     * @param argc The argument count
     * @param argv The argument vector: the string to evaluate
     * @return Potential errors that occured on command execution
     * 
     * Examples:
     * 
     * eval echo $(1+1)    outputs:    2.000000
     * eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
     */
    Nebulite::Constants::Error eval(int argc, char* argv[]);
    std::string eval_desc = R"(Evaluates an expression string and executes it.
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

    /**
     * @brief Exits the entire program
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error exit(int argc, char* argv[]);
    std::string exit_desc = R"(Exits the entire program.

    Usage: exit

    Closes the program with exit code 0 (no error)
    Any queued tasks will be discarded.
    )";

    /**
     * @brief Sets the waitCounter to the given value to halt all script tasks for a given amount of frames
     * 
     * @param argc The argument count
     * @param argv The argument vector: frame count to wait
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error wait(int argc, char* argv[]);
    std::string wait_desc = R"(Sets the waitCounter to the given value to halt all script tasks for a given amount of frames.

    Usage: wait <frames>

    This command pauses the execution of all script tasks for the specified number of frames.
    This does not halt any tasks comming from objects within the environment and cannot be used by them.

    The wait-command is intended for scripts only, allowing for timed delays between commands.

    This is useful for:
    - Creating pauses in scripts to wait for certain conditions to be met.
    - Timing events in a sequence.
    - Tool assisted speedruns (TAS)
    )";

    /**
     * @brief Loads tasks from a file into the taskQueue
     * 
     * @param argc The argument count
     * @param argv The argument vector: the filename to load
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error task(int argc, char* argv[]);
    std::string task_desc = R"(Loads tasks from a file into the taskQueue.

    Usage: task <filename>

    This command loads a list of tasks from the specified file into the task queue.
    Each line in the file is treated as a separate task.

    Task files are not appended at the end, but right after the current task. 
    This ensures that tasks can be loaded within task files themselves and being executed immediately.

    This pseudo-example shows how tasks are loaded and executed:

    MAIN_TASK{
        maincommand1
        maincommand2
        task subtaskfile.txt{
            subcommand1
            subcommand2
        }
        maincommand4
    }
    )";

    /**
     * @brief Executes a for-loop with a function call
     * 
     * @param argc The argument count
     * @param argv The argument vector: <var> <start> <end> <functioncall>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error func_for(int argc, char* argv[]);
    std::string func_for_desc = R"(Executes a for-loop with a function call.

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

    /**
     * @brief Executes a block of code if a condition is true
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition> <functioncall>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error func_if(int argc, char* argv[]);
    std::string func_if_desc = R"(Executes a block of code if a condition is true.

    Usage: if <condition> <functioncall>

    It is recommended to wrap the condition in quotes to prevent parsing issues.

    However, This is not supported for in-console usage. 
    This is because the console itself removes quotes before passing the arguments to the FuncTree,
    rendering them useless.

    Example:
    if '$(eq(1+1,2))' echo Condition is true!
    )";

    /**
     * @brief Returns a custom value of type Critical Error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error. 
     */
    Nebulite::Constants::Error func_return(int argc, char* argv[]);
    std::string func_return_desc = R"(Returns a custom value as a Critical Error.

    Usage: return <string>

    This command creates a custom critical error with the given string as description.
    This can be used to exit from a task queue with a custom message.

    Example:

    ./bin/Nebulite return We did not anticipate this happening, weird.
    Outputs:
    We did not anticipate this happening, weird.
    Critical Error: We did not anticipate this happening, weird.
    )";

    /**
     * @brief Echoes all arguments as string to the standard output
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error echo(int argc, char* argv[]);
    std::string echo_desc = R"(Echoes all arguments as string to the standard output.

    Usage: echo <string>

    This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).
    Example:
    ./bin/Nebulite echo Hello World!
    Outputs:
    Hello World!
    )";

    /**
     * @brief Asserts a condition and throws a custom error if false
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition>
     * @return Potential errors that occured on command execution
     * 
     * @todo Return a custom error string with the failed condition
     */
    Nebulite::Constants::Error func_assert(int argc, char* argv[]);
    std::string assert_desc = R"(Asserts a condition and throws a custom error if false.

    Usage: assert <condition>

    It is recommended to wrap the condition in quotes to prevent parsing issues.
    
    Example:
    assert '$(eq(1+1,2))'    // No error
    assert '$(eq(1+1,3))'    // Critical Error: A custom assertion failed.
    Assertion failed: $(eq(1+1,3)) is not true.
    )";
    /**
     * @brief Attach a command to the always-taskqueue that is executed on each tick.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <command>. The command to attach.
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error always(int argc, char* argv[]);
    std::string always_desc = R"(Attach a command to the always-taskqueue that is executed on each tick.

    Usage: always <command>

    Example:
    always echo This command runs every frame!
    This will output "This command runs every frame!" on every frame.
    )";

    /**
     * @brief Clears the entire always-taskqueue.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error alwaysClear(int argc, char* argv[]);
    std::string alwaysClear_desc = R"(Clears the entire always-taskqueue.

    Usage: always-clear

    Example:
    always-clear
    This will remove all commands from the always-taskqueue.
    )";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, General){
        bindFunction(&General::eval,        "eval",         eval_desc);
        bindFunction(&General::exit,        "exit",         exit_desc);
        bindFunction(&General::wait,        "wait",         wait_desc);
        bindFunction(&General::task,        "task",         task_desc);
        bindFunction(&General::func_for,    "for",          func_for_desc);
        bindFunction(&General::func_if,     "if",           func_if_desc);
        bindFunction(&General::func_return, "return",       func_return_desc);
        bindFunction(&General::echo,        "echo",         echo_desc);
        bindFunction(&General::func_assert, "assert",       assert_desc);
        bindFunction(&General::always,      "always",       always_desc);
        bindFunction(&General::alwaysClear, "always-clear", alwaysClear_desc);
    }
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite