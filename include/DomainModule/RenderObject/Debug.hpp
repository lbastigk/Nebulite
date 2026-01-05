/**
 * @file Debug.hpp
 * @brief Debug functions for the domain RenderObject.
 */

#ifndef NEBULITE_RODM_DEBUG_HPP
#define NEBULITE_RODM_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
}

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::Debug
 * @brief Debug management for the RenderObject tree DomainModule.
 * @todo Move texture debugging function to Texture Domain later on, once src/dst rect is properly handled there too.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Debug) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error eval(int argc, char** argv);
    static auto constexpr eval_name = "eval";
    static auto constexpr eval_desc = "Evaluate an expression and execute the result.\n"
        "Example: eval echo $(1+1)\n"
        "\n"
        "Examples:\n\n"
        "eval echo $(1+1)    outputs:    2.000000\n"
        "eval echo {self.id} outputs this objects id\n";

    Constants::Error printSrcRect();
    static auto constexpr printSrcRect_name = "debug print-src-rect";
    static auto constexpr printSrcRect_desc = "Prints debug information about the source rectangle to console\n"
        "\n"
        "Usage: debug print-src-rect\n"
        "\n"
        "Outputs:\n"
        "Source Rectangle: { x: ..., y: ..., w: ..., h: ... }\n"
        "If the RenderObject is not a spritesheet, indicates that instead:\n"
        "This RenderObject is not a spritesheet.\n";

    Constants::Error printDstRect();
    static auto constexpr printDstRect_name = "debug print-dst-rect";
    static auto constexpr printDstRect_desc = "Prints debug information about the destination rectangle to console\n"
        "\n"
        "Usage: debug print-dst-rect\n"
        "\n"
        "Outputs:\n"
        "Destination Rectangle: { x: ..., y: ..., w: ..., h: ... }\n"
        "If the RenderObject is not a spritesheet, indicates that instead:\n"
        "Destination rectangle is not set.\n";

    Constants::Error textureStatus();
    static auto constexpr textureStatus_name = "debug texture-status";
    static auto constexpr textureStatus_desc = "Prints debug information about the texture to console\n"
        "\n"
        "Usage: debug texture-status\n"
        "\n"
        "Outputs various details about the texture, including:\n"
        " - Texture Key\n"
        " - Valid Texture\n"
        " - Local Texture\n"
        " - SDL Texture Info (Width, Height, Access, Format)\n";

    //------------------------------------------
    // Category names
    static auto constexpr debug_name = "debug";
    static auto constexpr debug_desc = "Debugging functions for RenderObject domains";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Debug) {
        // Some functions like selected-object need eval to resolve variables
        BINDFUNCTION(&Debug::eval, eval_name, eval_desc);

        (void)bindCategory(debug_name, debug_desc);
        BINDFUNCTION(&Debug::printSrcRect, printSrcRect_name, printSrcRect_desc);
        BINDFUNCTION(&Debug::printDstRect, printDstRect_name, printDstRect_desc);
        BINDFUNCTION(&Debug::textureStatus, textureStatus_name, textureStatus_desc);
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_DEBUG_HPP
