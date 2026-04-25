/**
 * @file General.hpp
 * @brief General-purpose DomainModule for the GlobalSpace.
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBALSPACE_GENERAL_HPP
#define NEBULITE_DOMAINMODULE_GLOBALSPACE_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
}

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::General
 * @brief DomainModule for general-purpose functions within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, General) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event exit() const ;
    static auto constexpr exit_name = "exit";
    static auto constexpr exit_desc = "Exits the entire program.\n"
        "\n"
        "Usage: exit\n"
        "\n"
        "Closes the program\n"
        "Any queued tasks will be discarded.\n";

    [[nodiscard]] Constants::Event wait(int argc, char** argv) const ;
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

    [[nodiscard]] Constants::Event task(int argc, char** argv) const ;
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

    [[nodiscard]] Constants::Event always(int argc, char** argv) const ;
    static auto constexpr always_name = "always";
    static auto constexpr always_desc = "Attach a command to the always-taskqueue that is executed on each tick.\n"
        "\n"
        "Usage: always <command>\n"
        "\n"
        "Example:\n"
        "always echo This command runs every frame!\n"
        "This will output \"This command runs every frame!\" on every frame.\n";

    [[nodiscard]] Constants::Event alwaysClear() const ;
    static auto constexpr alwaysClear_name = "always-clear";
    static auto constexpr alwaysClear_desc = "Clears the entire always-taskqueue.\n"
        "\n"
        "Usage: always-clear\n"
        "\n"
        "Example:\n"
        "always-clear\n"
        "This will remove all commands from the always-taskqueue.\n";

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
        bindFunction(&General::exit, exit_name, exit_desc);
        bindFunction(&General::wait, wait_name, wait_desc);
        bindFunction(&General::task, task_name, task_desc);
        bindFunction(&General::always, always_name, always_desc);
        bindFunction(&General::alwaysClear, alwaysClear_name, alwaysClear_desc);
    }
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_GENERAL_HPP
