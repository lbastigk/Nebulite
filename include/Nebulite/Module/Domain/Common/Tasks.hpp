#ifndef NEBULITE_MODULE_DOMAIN_COMMON_TASKS_HPP
#define NEBULITE_MODULE_DOMAIN_COMMON_TASKS_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
/**
 * @class Nebulite::Module::Domain::Common::Tasks
 * @brief DomainModule for task management.
 */
class Tasks final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event wait(std::span<std::string_view const> args) const ;
    static auto constexpr waitName = "wait";
    static auto constexpr waitDesc = "Sets the waitCounter to the given value to halt all script tasks for a given amount of frames.\n"
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

    [[nodiscard]] Constants::Event task(std::span<std::string_view const> args) const ;
    static auto constexpr taskName = "task";
    static auto constexpr taskDesc = "Loads tasks from a file into the taskQueue, but does not execute them immediately.\n"
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

    [[nodiscard]] Constants::Event taskExec(std::span<std::string_view const> args, Interaction::Context ctx, Interaction::ContextScope ctxScope) const ;
    static auto constexpr taskExecName = "task-exec";
    static auto constexpr taskExecDesc = "Same as 'task', but with instant execution.";

    [[nodiscard]] Constants::Event always(std::span<std::string_view const> args) const ;
    static auto constexpr alwaysName = "always";
    static auto constexpr alwaysDesc = "Attach a command to the always-taskqueue that is executed on each tick.\n"
        "\n"
        "Usage: always <command>\n"
        "\n"
        "Example:\n"
        "always echo This command runs every frame!\n"
        "This will output \"This command runs every frame!\" on every frame.\n";

    [[nodiscard]] Constants::Event alwaysClear() const ;
    static auto constexpr alwaysClearName = "always-clear";
    static auto constexpr alwaysClearDesc = "Clears the entire always-taskqueue.\n"
        "\n"
        "Usage: always-clear\n"
        "\n"
        "Example:\n"
        "always-clear\n"
        "This will remove all commands from the always-taskqueue.\n";

    //------------------------------------------
    // Other public functions

    /**
     * @brief Rolls back all RNGs to their previous state.
     *        Can be called by any domainModule function
     *        if you don't want this functioncall to modify RNG state.
     *        Example: calling a script should not modify RNG, so that we can
     *                 always load scripts for TAS without RNG state changes.
     */
    void rngRollback();

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit Tasks(ConstructorParams const& params) : DomainModule(params) {
        bindFunction(&Tasks::wait, waitName, waitDesc);
        bindFunction(&Tasks::task, taskName, taskDesc);
        bindFunction(&Tasks::taskExec, taskExecName, taskExecDesc);
        bindFunction(&Tasks::always, alwaysName, alwaysDesc);
        bindFunction(&Tasks::alwaysClear, alwaysClearName, alwaysClearDesc);
    }
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_MODULE_DOMAIN_COMMON_TASKS_HPP
