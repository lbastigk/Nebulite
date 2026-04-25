/**
 * @file Console.hpp
 * @brief This file contains the DomainModule of the GlobalSpace to provide console capabilities.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP
#define NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Constants/KeyNames.hpp"
#include "Module/Domain/Renderer/Input.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {
/**
 * @class Nebulite::Module::Domain::Renderer::Console
 * @brief DomainModule for Console capabilities within the Renderer.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, Console) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event consoleOpen();
    static auto constexpr consoleOpen_name = "console open";
    static auto constexpr consoleOpen_desc = "Opens the console\n"
        "Pauses the application by sending a skip update signal to the renderer\n"
        "\n"
        "Usage: console open\n";

    [[nodiscard]] Constants::Event consoleClose();
    static auto constexpr consoleClose_name = "console close";
    static auto constexpr consoleClose_desc = "Closes the console\n"
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
        bindCategory(console_name, console_desc);
        bindFunction(&Console::consoleOpen, consoleOpen_name, consoleOpen_desc);
        bindFunction(&Console::consoleClose, consoleClose_name, consoleClose_desc);
    }

    struct Key : Data::KeyGroup<Input::Key::getPrefix()> { // Same scope as input domainmodule, so we can access input states for toggling the console
        // No keys for now
    };

private:
    // Whether the console is currently active
    bool consoleMode = false;
};
} // namespace Nebulite::Module::Domain::Renderer
#endif // NEBULITE_DOMAINMODULE_RENDERER_CONSOLE_HPP
