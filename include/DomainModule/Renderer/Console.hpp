/**
 * @file Console.hpp
 * @brief This file contains the DomainModule of the GlobalSpace to provide console capabilities.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP
#define NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP

//------------------------------------------
// Includes

// Standard Library
#include <queue>

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Constants/KeyNames.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TextInput.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::Console
 * @brief DomainModule for Console capabilities within the Renderer.
 * @todo: Add functions to open/close the console via commands.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, Console) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error consoleOpen();
    static auto constexpr consoleOpen_name = "console open";
    static auto constexpr consoleOpen_desc = "Opens the console, allowing it to be rendered and receive input.\n"
        "\n"
        "Usage: console open\n";

    Constants::Error consoleClose();
    static auto constexpr consoleClose_name = "console close";
    static auto constexpr consoleClose_desc = "Closes the console, hiding it and preventing it from receiving input.\n"
        "\n"
        "Usage: console close\n";

    // Autotype functions

    Constants::Error consoleAutotypeText(std::span<std::string const> const& args);
    static auto constexpr consoleAutotypeText_name = "console autotype text";
    static auto constexpr consoleAutotypeText_desc = "Adds a text input command into the autotype queue.\n"
        "\n"
        "Usage: console autotype text <text>\n";

    Constants::Error consoleAutotypeEnter();
    static auto constexpr consoleAutotypeEnter_name = "console autotype enter";
    static auto constexpr consoleAutotypeEnter_desc = "Puts an enter command into the autotype queue.\n"
        "\n"
        "Usage: console autotype enter\n";

    Constants::Error consoleAutotypeExecute();
    static auto constexpr consoleAutotypeExecute_name = "console autotype execute";
    static auto constexpr consoleAutotypeExecute_desc = "Executes all autotype commands in the queue.\n"
        "\n"
        "Usage: console autotype execute\n";

    Constants::Error consoleAutotypeWait(std::span<std::string const> const& args);
    static auto constexpr consoleAutotypeWait_name = "console autotype wait";
    static auto constexpr consoleAutotypeWait_desc = "Waits for a specified amount of ms before executing the next autotype command.\n"
        "\n"
        "Usage: console autotype wait <milliseconds>\n";

    Constants::Error consoleAutotypeClose();
    static auto constexpr consoleAutotypeClose_name = "console autotype close";
    static auto constexpr consoleAutotypeClose_desc = "Closes the console if the autotype wait counter reaches zero"
        "\n"
        "Usage: console autotype close\n";

    Constants::Error consoleAutotypeHistoryUp();
    static auto constexpr consoleAutotypeHistoryUp_name = "console autotype up";
    static auto constexpr consoleAutotypeHistoryUp_desc = "Adds a history up command into the autotype queue.\n"
        "\n"
        "Usage: console autotype up\n";

    Constants::Error consoleAutotypeHistoryDown();
    static auto constexpr consoleAutotypeHistoryDown_name = "console autotype down";
    static auto constexpr consoleAutotypeHistoryDown_desc = "Adds a history down command into the autotype queue.\n"
        "\n"
        "Usage: console autotype down\n";

    Constants::Error consoleAutotypeClear();
    static auto constexpr consoleAutotypeClear_name = "console autotype clear";
    static auto constexpr consoleAutotypeClear_desc = "Clears the autotype queue without executing the commands.\n"
        "Clears both the queue pending for execution and the active queue currently being executed.\n"
        "\n"
        "Usage: console autotype clear\n";


    //------------------------------------------
    // Category strings

    static auto constexpr console_name = "console";
    static auto constexpr console_desc = "Console commands and settings.\n"
        "\n"
        "Contains commands to manipulate the in-application console.\n";

    static auto constexpr consoleAutotype_name = "console autotype";
    static auto constexpr consoleAutotype_desc = "Utilities to automate typing in the console";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, Console) {
        // we cannot do much here, since renderer might not be initialized yet
        // so we do the actual initialization in update() when needed
        bindCategory(console_name, console_desc);
        BIND_FUNCTION(&Console::consoleOpen, consoleOpen_name, consoleOpen_desc);
        BIND_FUNCTION(&Console::consoleClose, consoleClose_name, consoleClose_desc);

        bindCategory(consoleAutotype_name, consoleAutotype_desc);
        BIND_FUNCTION(&Console::consoleAutotypeText, consoleAutotypeText_name, consoleAutotypeText_desc);
        BIND_FUNCTION(&Console::consoleAutotypeEnter, consoleAutotypeEnter_name, consoleAutotypeEnter_desc);
        BIND_FUNCTION(&Console::consoleAutotypeExecute, consoleAutotypeExecute_name, consoleAutotypeExecute_desc);
        BIND_FUNCTION(&Console::consoleAutotypeWait, consoleAutotypeWait_name, consoleAutotypeWait_desc);
        BIND_FUNCTION(&Console::consoleAutotypeClose, consoleAutotypeClose_name, consoleAutotypeClose_desc);
        BIND_FUNCTION(&Console::consoleAutotypeHistoryUp, consoleAutotypeHistoryUp_name, consoleAutotypeHistoryUp_desc);
        BIND_FUNCTION(&Console::consoleAutotypeHistoryDown, consoleAutotypeHistoryDown_name, consoleAutotypeHistoryDown_desc);
        BIND_FUNCTION(&Console::consoleAutotypeClear, consoleAutotypeClear_name, consoleAutotypeClear_desc);
    }

    struct Key : Data::KeyGroup<"renderer."> {
        // No keys for now
    };

private:
    // Whether the console is currently active
    bool consoleMode = false;

    //------------------------------------------
    // Autotype handling

    /**
     * @brief Struct to manage the autotype system, which allows queuing up commands to be executed in the console with optional wait times between them.
     */
    struct AutoType {
        /**
         * @struct Command
         * @brief Represents a command for the autotype system
         * @details Commands cannot be executed immediately, so we store their representation in a queue instead.
         */
        struct Command {
            enum class Type {
                TEXT,
                ENTER,
                CLOSE,
                WAIT,
                HISTORY_UP,
                HISTORY_DOWN
            } type;
            std::string text; // Additional data for text or wait commands
        };

        // Stores all autotype commands before "autotype execute" is called
        std::queue<Command> queue;

        // Stores autotype commands that are currently being executed (after "autotype execute" is called, until all commands are executed)
        std::queue<Command> activeQueue;

        // Remaining wait time in ms before the next autotype command from autotypeActiveQueue can be executed.
        size_t waitTimeRemaining = 0;

        // Timer to track dt for autotype wait commands
        Utility::TimeKeeper waitTimer;
    }autoType;

    /**
     * @brief Processes the autotype command queue, executing commands one by one while respecting wait times.
     */
    void processAutotypeQueue();
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP
