/**
 * @file General.hpp
 * @brief General-purpose DomainModule for the GlobalSpace.
 */

#ifndef NEBULITE_GSDM_GENERAL_HPP
#define NEBULITE_GSDM_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
}

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::General
 * @brief DomainModule for general-purpose functions within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, General) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error eval(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope);
    static auto constexpr eval_name = "eval";
    static auto constexpr eval_desc = "Evaluates an expression string and executes it.\n"
        "Every argument after eval is concatenated with a whitespace to form the expression to be evaluated and then reparsed.\n"
        "\n"
        "Usage: eval <expression>\n"
        "\n"
        "Examples:\n"
        "\n"
        "eval echo $(1+1)\n"
        "outputs: 2.000000\n"
        "First, eval evaluates every argument, then concatenates them with a whitespace,\n"
        "and finally executes the resulting string as a command.\n"
        "The string 'echo $(1+1)' is evaluated to \"echo 2.000000\", which is then executed.\n"
        "\n"
        "eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json\n"
        "This evaluates to 'spawn ./Resources/RenderObjects/NAME.json',\n"
        "where NAME is the current value of the global variable ToSpawn\n";

    Constants::Error exit();
    static auto constexpr exit_name = "exit";
    static auto constexpr exit_desc = "Exits the entire program.\n"
        "\n"
        "Usage: exit\n"
        "\n"
        "Closes the program\n"
        "Any queued tasks will be discarded.\n";

    Constants::Error wait(int argc, char** argv);
    static auto constexpr wait_name = "wait";
    static auto constexpr wait_desc = "Sets the waitCounter to the given value to halt all script tasks for a given amount of frames.\n"
        "\n"
        "Usage: wait <frames>\n"
        "\n"
        "This command pauses the execution of all script tasks for the specified number of frames.\n"
        "This does not halt any tasks coming from objects within the environment and cannot be used by them.\n"
        "\n"
        "This is useful for:"
        "- Creating pauses in scripts to wait for certain conditions to be met.\n"
        "- Timing events in a sequence.\n"
        "- Tool assisted speedruns (TAS)\n";

    Constants::Error task(int argc, char** argv);
    static auto constexpr task_name = "task";
    static auto constexpr task_desc = "Loads tasks from a file into the taskQueue.\n"
        "\n"
        "Usage: task <filename>\n"
        "\n"
        "This command loads a list of tasks from the specified file into the task queue.\n"
        "Each line in the file is treated as a separate task.\n"
        "\n"
        "Task files are not appended at the end, but right after the current task.\n"
        "This ensures that tasks can be loaded within task files themselves and being executed immediately.\n"
        "\n"
        "This example shows how tasks are loaded and executed:\n"
        "\n"
        "Main task:\n"
        "    mainCommand1\n"
        "    mainCommand2\n"
        "    task subTaskFile.txt:\n"
        "        subCommand1\n"
        "        subCommand2\n"
        "    mainCommand4\n";

    Constants::Error func_for(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope);
    static auto constexpr func_for_name = "for";
    static auto constexpr func_for_desc = "Executes a for-loop with a function call.\n"
        "\n"
        "Usage: for <var> <start> <end> <functioncall>\n"
        "\n"
        "Example:\n"
        "for i 1 5 echo Iteration {i}\n"
        "This will output:\n"
        "    Iteration 1\n"
        "    Iteration 2\n"
        "    Iteration 3\n"
        "    Iteration 4\n"
        "    Iteration 5\n"
        "\n"
        "This is useful for:\n"
        "- Repeating actions a specific number of times.\n"
        "- Iterating over a range of values.\n"
        "- Creating complex control flows in scripts.\n";

    Constants::Error func_if(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope);
    static auto constexpr func_if_name = "if";
    static auto constexpr func_if_desc = "Executes a block of code if a condition is true.\n"
        "\n"
        "Usage: if <condition> <functioncall>\n"
        "\n"
        "It is recommended to wrap the condition in quotes to prevent parsing issues.\n"
        "\n"
        "Example:\n"
        "if '$(eq(1+1,2))' echo Condition is true!\n";

    Constants::Error func_return(int argc, char** argv);
    static auto constexpr func_return_name = "return";
    static auto constexpr func_return_desc = "Returns a custom value as a Critical Error.\n"
        "\n"
        "Usage: return <string>\n"
        "\n"
        "This command creates a custom critical error with the given string as description.\n"
        "This can be used to exit from a task queue with a custom message.\n"
        "\n"
        "Example:\n"
        "./bin/Nebulite return We did not anticipate this happening, weird.\n"
        "Outputs:\n"
        "We did not anticipate this happening, weird.\n"
        "Critical Error: We did not anticipate this happening, weird.\n";

    Constants::Error echo(int argc, char** argv);
    static auto constexpr echo_name = "echo";
    static auto constexpr echo_desc = "Echoes all arguments as string to the standard output.\n"
        "\n"
        "Usage: echo <string>\n"
        "\n"
        "This command concatenates all arguments with a whitespace and outputs them to the standard output (cout).\n"
        "Example:\n"
        "./bin/Nebulite echo Hello World!\n"
        "Outputs:\n"
        "Hello World!\n";

    Constants::Error func_assert(int argc, char** argv);
    static auto constexpr assert_name = "assert";
    static auto constexpr assert_desc = "Asserts a condition and throws a custom error if false.\n"
        "\n"
        "Usage: assert <condition>\n"
        "\n"
        "It is recommended to wrap the condition in quotes to prevent parsing issues.\n"
        "\n"
        "Example:\n"
        "assert '$(eq(1+1,2))'    // No error\n"
        "assert '$(eq(1+1,3))'    // Critical Error: A custom assertion failed.\n"
        "Assertion failed: $(eq(1+1,3)) is not true.\n";

    Constants::Error always(int argc, char** argv);
    static auto constexpr always_name = "always";
    static auto constexpr always_desc = "Attach a command to the always-taskqueue that is executed on each tick.\n"
        "\n"
        "Usage: always <command>\n"
        "\n"
        "Example:\n"
        "always echo This command runs every frame!\n"
        "This will output \"This command runs every frame!\" on every frame.\n";

    Constants::Error alwaysClear();
    static auto constexpr alwaysClear_name = "always-clear";
    static auto constexpr alwaysClear_desc = "Clears the entire always-taskqueue.\n"
        "\n"
        "Usage: always-clear\n"
        "\n"
        "Example:\n"
        "always-clear\n"
        "This will remove all commands from the always-taskqueue.\n";

    Constants::Error nop();
    static auto constexpr nop_name = "nop";
    static auto constexpr nop_desc = "No operation. Does nothing.\n"
        "\n"
        "Usage: nop <blind arguments>\n"
        "\n"
        "Useful for testing or as a placeholder in scripts where no action is required,\n"
        "but a command is syntactically necessary.\n";

    Constants::Error inScope(std::span<std::string const> const& args);
    static auto constexpr inScope_name = "in-scope";
    static auto constexpr inScope_desc = "Parses a command within a specific scope of the Global Space.\n"
        "\n"
        "Usage: in-scope <scope> <command>\n"
        "\n"
        "Example:\n"
        "in-scope global echo This is in the global scope!\n"
        "This will output: This is in the global scope!\n";

    //------------------------------------------
    // Category names

    // None, general functions should be in the root of the GlobalSpace
    // Otherwise they should not be considered general-purpose

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, General) {
        BINDFUNCTION(&General::eval, eval_name, eval_desc);
        BINDFUNCTION(&General::exit, exit_name, exit_desc);
        BINDFUNCTION(&General::wait, wait_name, wait_desc);
        BINDFUNCTION(&General::task, task_name, task_desc);
        BINDFUNCTION(&General::func_for, func_for_name, func_for_desc);
        BINDFUNCTION(&General::func_if, func_if_name, func_if_desc);
        BINDFUNCTION(&General::func_return, func_return_name, func_return_desc);
        BINDFUNCTION(&General::echo, echo_name, echo_desc);
        BINDFUNCTION(&General::func_assert, assert_name, assert_desc);
        BINDFUNCTION(&General::always, always_name, always_desc);
        BINDFUNCTION(&General::alwaysClear, alwaysClear_name, alwaysClear_desc);
        BINDFUNCTION(&General::nop, nop_name, nop_desc);
        BINDFUNCTION(&General::inScope, inScope_name, inScope_desc);
    }
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_GENERAL_HPP
