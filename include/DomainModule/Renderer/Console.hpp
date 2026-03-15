/**
 * @file Console.hpp
 * @brief This file contains the DomainModule of the GlobalSpace to provide console capabilities.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP
#define NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Constants/KeyNames.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TextInput.hpp"

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

    //------------------------------------------
    // Category strings

    static auto constexpr console_name = "console";
    static auto constexpr console_desc = "Console commands and settings.\n"
        "\n"
        "Contains commands to manipulate the in-application console.\n";

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
    }

    struct Key : Data::KeyGroup<"renderer."> {
        // No keys for now
    };

private:
    // Whether the console is currently active
    bool consoleMode = false;
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP
