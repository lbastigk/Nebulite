/**
 * @file General.hpp
 * @brief Provides rendering utilities for the Nebulite engine.
 *        This file contains a DomainModule for basic rendering-related functioncalls.
 */

#ifndef NEBULITE_RRDM_GENERAL_HPP
#define NEBULITE_RRDM_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Renderer; // Forward declaration of domain class Renderer
class RenderObject; // Forward declaration of RenderObject
}

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::General
 * @brief Basic Renderer-Related Functions
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, General) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // Spawn a renderobject
    /**
     * @brief Spawns a new render object.
     * @details Memory management ist handled by the Renderer
     *          Implementing `unique_ptr` or `shared_ptr` is a work in progress
     *          as its made difficult by the ability of globalspace to select a RenderObject
     *          and store its pointer.
     *          The Renderer is, besides the selection addition from `selectedObject_get`,
     *          a closed system that handles the pointer and lifetime of RenderObjects.
     *          Thus, the usage of `unique_ptr` or `shared_ptr` is not needed here,
     *          but perhaps helpful if complexity increases.
     *          We might wish to implement this in the future, but for now
     *          we will use a raw pointer and let the Renderer handle the memory management.
     *          Implementing a shared or unique pointer would require a significant rework of:
     *          - Renderer append function
     *          - Environment append function
     *          - RenderObjectContainer append function
     *          - Its batch management
     *          - RenderObject selection mechanism
     *          - RenderObject deletion mechanism in Renderer::update()
     *          As of now, the implementation is fully functional so it's a low priority task.
     */
    Constants::Error spawn(int argc, char** argv);
    static std::string_view constexpr spawn_name = "spawn";
    static std::string_view constexpr spawn_desc = "Spawn a RenderObject from a json/jsonc file.\n"
        "\n"
        "Usage: spawn <path/to/file.jsonc>\n\n"
        "Supports lookups in standard resource directories:\n"
        "- ./Resources/RenderObjects/\n"
        "- ./Resources/Renderobjects/\n\n"
        "Example: 'spawn Planets/sun.jsonc|set text.str This is a sun'\n"
        "Looks for object 'sun.jsonc' in the standard directories\n"
        "- './Planets/sun.jsonc'\n"
        "- './Resources/RenderObjects/Planets/sun.jsonc'\n"
        "- './Resources/Renderobjects/Planets/sun.jsonc'\n"
        "and spawns the first found object.";

    Constants::Error env_load(int argc, char** argv);
    static std::string_view constexpr env_load_name = "env load";
    static std::string_view constexpr env_load_desc = "Load an environment/level from a json/jsonc file.\n"
        "\n"
        "Usage: env load <path/to/file.jsonc>\n\n"
        "If no argument is provided, an empty environment is loaded.";

    Constants::Error env_deload(int argc, char** argv);
    static std::string_view constexpr env_deload_name = "env deload";
    static std::string_view constexpr env_deload_desc = "Deload entire environment, leaving an empty renderer.\n"
        "\n"
        "Usage: env deload";

    Constants::Error setResolution(int argc, char** argv);
    static std::string_view constexpr setResolution_name = "set-res";
    static std::string_view constexpr setResolution_desc = "Set resolution of renderer.\n"
        "\n"
        "Usage: set-res [Width] [Height] [Scale]\n\n"
        "Defaults to 1000  for width if argument count < 1\n"
        "Defaults to 1000  for height if argument count < 2\n"
        "Defaults to 1     for scale if argument count < 3\n";

    Constants::Error setFPS(int argc, char** argv);
    static std::string_view constexpr setFPS_name = "set-fps";
    static std::string_view constexpr setFPS_desc = "Set FPS of renderer.\n"
        "\n"
        "Usage: set-fps [fps]\n\n"
        "Defaults to 60 fps if no argument is provided";

    Constants::Error showFPS(int argc, char** argv);
    static std::string_view constexpr showFPS_name = "show-fps";
    static std::string_view constexpr showFPS_desc = "Show FPS of renderer.\n"
        "\n"
        "Usage: show-fps [on|off]\n\n"
        "Defaults to on if no argument is provided";

    Constants::Error cam_move(int argc, char** argv);
    static std::string_view constexpr cam_move_name = "cam move";
    static std::string_view constexpr cam_move_desc = "Move camera by a given delta.\n"
        "\n"
        "Usage: cam move <dx> <dy>\n\n"
        "<dx> : Delta x to move camera by\n"
        "<dy> : Delta y to move camera by\n";

    Constants::Error cam_set(int argc, char** argv);
    static std::string_view constexpr cam_set_name = "cam set";
    static std::string_view constexpr cam_set_desc = "Set camera to concrete position.\n"
        "\n"
        "Usage: cam set <x> <y> [c]\n\n"
        "<x> : X position to set camera to\n"
        "<y> : Y position to set camera to\n"
        "[c] : Optional. If provided, sets the camera's center to the given position.\n";

    Constants::Error snapshot(int argc, char** argv);
    static std::string_view constexpr snapshot_name = "snapshot";
    static std::string_view constexpr snapshot_desc = "Create a snapshot of the current renderer state.\n"
        "\n"
        "Usage: snapshot [filename]\n\n"
        "Defaults to \"./Resources/Snapshots/snapshot.png\" if no argument is provided";

    Constants::Error beep();
    static std::string_view constexpr beep_name = "beep";
    static std::string_view constexpr beep_desc = "Make a beep noise.\n"
        "\n"
        "Usage: beep";

    /**
     * @todo If an object is deleted, the reference in GlobalSpace::selectedRenderObject is not cleared!
     *       Fix idea: Make Renderer a domain itself, with this function as a domainmodule part
     *       Then, we can have the SelectedRenderObject as a private member of the Renderer domain
     *       and manipulate it directly in Renderer::update()
     *       This would also make sense, as the Renderer is the owner of the RenderObjects
     *       and should thus also be the owner of the selected object reference
     */
    Constants::Error selectedObject_get(int argc, char** argv);
    static std::string_view constexpr selectedObject_get_name = "selected-object get";
    static std::string_view constexpr selectedObject_get_desc = "Get a renderobject by its ID.\n"
        "\n"
        "Usage: selected-object get <id>\n";

    Constants::Error selectedObject_Parse(int argc, char** argv);
    static std::string_view constexpr selectedObject_Parse_name = "selected-object parse";
    static std::string_view constexpr selectedObject_Parse_desc = "Parse a command on the selected RenderObject.\n"
        "\n"
        "Usage: selected-object parse <command>\n"
        "\n"
        "Use 'selected-object get <id>' to select a RenderObject first.\n"
        "Use 'selected-object parse help' to see available commands for the selected object.\n";

    //------------------------------------------
    // Category names
    static std::string_view constexpr cam_name = "cam";
    static std::string_view constexpr cam_desc = "Renderer Camera Functions";

    static std::string_view constexpr selectedObject_name = "selected-object";
    static std::string_view constexpr selectedObject_desc = "Functions to select and interact with a selected RenderObject";

    static std::string_view constexpr env_name = "env";
    static std::string_view constexpr env_desc = "Environment management functions";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     * @todo Add domainModules for camera and renderobject-selection and move respective functions in there.
     * @todo Move functions for Renderer and Environment to domains themselves,
     *       once they are implemented as such.
     *       This will declutter the globalspace, separating its usage from the Renderer and Environment.
     *       The only downside currently is that we have to implement a method to lazy-init the SDL Renderer within the Renderer domain itself.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, General) {
        bindFunction(&General::spawn, spawn_name, spawn_desc);
        bindFunction(&General::setResolution, setResolution_name, setResolution_desc);
        bindFunction(&General::setFPS, setFPS_name, setFPS_desc);
        bindFunction(&General::showFPS, showFPS_name, showFPS_desc);
        bindFunction(&General::snapshot, snapshot_name, snapshot_desc);
        bindFunction(&General::beep, beep_name, beep_desc);

        (void)bindCategory(cam_name, cam_desc);
        bindFunction(&General::cam_move, cam_move_name, cam_move_desc);
        bindFunction(&General::cam_set, cam_set_name, cam_set_desc);

        (void)bindCategory(selectedObject_name, selectedObject_desc);
        bindFunction(&General::selectedObject_get, selectedObject_get_name, selectedObject_get_desc);
        bindFunction(&General::selectedObject_Parse, selectedObject_Parse_name, selectedObject_Parse_desc);

        (void)bindCategory(env_name, env_desc);
        bindFunction(&General::env_load, env_load_name, env_load_desc);
        bindFunction(&General::env_deload, env_deload_name, env_deload_desc);
    }

private:
    /**
     * @brief Pointer to the currently selected RenderObject
     */
    Core::RenderObject* selectedRenderObject = nullptr;
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_RRDM_GENERAL_HPP
