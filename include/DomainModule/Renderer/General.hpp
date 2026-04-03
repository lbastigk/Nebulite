/**
 * @file General.hpp
 * @brief Provides rendering utilities for the Nebulite engine.
 *        This file contains a DomainModule for basic rendering-related functioncalls.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDERER_GENERAL_HPP
#define NEBULITE_DOMAINMODULE_RENDERER_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
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
    [[nodiscard]] Constants::Event updateHook() override;
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
    [[nodiscard]] Constants::Event spawn(int argc, char** argv) const ;
    static auto constexpr spawn_name = "spawn";
    static auto constexpr spawn_desc = "Spawn a RenderObject from a json/jsonc file.\n"
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
        "and spawns the first found object.\n";

    [[nodiscard]] Constants::Event envLoad(int argc, char** argv) const ;
    static auto constexpr envLoad_name = "env load";
    static auto constexpr envLoad_desc = "Load an environment/level from a json/jsonc file.\n"
        "\n"
        "Usage: env load <path/to/file.jsonc>\n\n"
        "If no argument is provided, an empty environment is loaded.\n";

    [[nodiscard]] Constants::Event envDeload() const ;
    static auto constexpr envDeload_name = "env deload";
    static auto constexpr envDeload_desc = "Deload entire environment, leaving an empty renderer.\n"
        "\n"
        "Usage: env deload\n";

    [[nodiscard]] Constants::Event setResolution(int argc, char** argv) const ;
    static auto constexpr setResolution_name = "set-res";
    static auto constexpr setResolution_desc = "Set resolution of renderer.\n"
        "\n"
        "Usage: set-res [Width] [Height] [Scale]\n\n"
        "Defaults to 1000  for width if argument count < 1\n"
        "Defaults to 1000  for height if argument count < 2\n"
        "Defaults to 1     for scale if argument count < 3\n";

    [[nodiscard]] Constants::Event setFPS(int argc, char** argv) const ;
    static auto constexpr setFPS_name = "set-fps";
    static auto constexpr setFPS_desc = "Set FPS of renderer.\n"
        "\n"
        "Usage: set-fps [fps]\n\n"
        "Defaults to 60 fps if no argument is provided\n";

    [[nodiscard]] Constants::Event showFPS(int argc, char** argv) const ;
    static auto constexpr showFPS_name = "show-fps";
    static auto constexpr showFPS_desc = "Show FPS of renderer.\n"
        "\n"
        "Usage: show-fps [on|off]\n\n"
        "Defaults to on if no argument is provided\n";

    [[nodiscard]] Constants::Event cam_move(int argc, char** argv) const ;
    static auto constexpr cam_move_name = "cam move";
    static auto constexpr cam_move_desc = "Move camera by a given delta.\n"
        "\n"
        "Usage: cam move <dx> <dy>\n\n"
        "<dx> : Delta x to move camera by\n"
        "<dy> : Delta y to move camera by\n";

    [[nodiscard]] Constants::Event cam_set(int argc, char** argv) const ;
    static auto constexpr cam_set_name = "cam set";
    static auto constexpr cam_set_desc = "Set camera to concrete position.\n"
        "\n"
        "Usage: cam set <x> <y> [c]\n\n"
        "<x> : X position to set camera to\n"
        "<y> : Y position to set camera to\n"
        "[c] : Optional. If provided, sets the camera's center to the given position.\n";

    [[nodiscard]] Constants::Event snapshot(int argc, char** argv) const ;
    static auto constexpr snapshot_name = "snapshot";
    static auto constexpr snapshot_desc = "Create a snapshot of the current renderer state.\n"
        "\n"
        "Usage: snapshot [filename]\n\n"
        "Defaults to \"./Resources/Snapshots/snapshot.png\" if no argument is provided\n";

    [[nodiscard]] Constants::Event dumpView() const;
    static auto constexpr dumpView_name = "dump-view";
    static auto constexpr dumpView_desc = "Dump the current view of the renderer to the console, as JSON.\n"
        "The dump is not synchronous with the call, but is executed after the next render pass.\n"
        "\n"
        "Usage: dump-view\n";

    [[nodiscard]] Constants::Event selectedObject_get(int argc, char** argv);
    static auto constexpr selectedObject_get_name = "selected-object get";
    static auto constexpr selectedObject_get_desc = "Get a renderobject by its index in the Renderer.\n"
        "The index is converted to its corresponding Domain ID and selected as the current RenderObject to interact with for other selected-object commands.\n"
        "\n"
        "Usage: selected-object get <idx>\n";

    [[nodiscard]] Constants::Event selectedObject_Parse(std::span<std::string const> const& args) const ;
    static auto constexpr selectedObject_Parse_name = "selected-object parse";
    static auto constexpr selectedObject_Parse_desc = "Parse a command on the selected RenderObject.\n"
        "\n"
        "Usage: selected-object parse <command>\n"
        "\n"
        "Use 'selected-object get <id>' to select a RenderObject first.\n"
        "Use 'selected-object parse help' to see available commands for the selected object.\n";

    //------------------------------------------
    // Category names
    static auto constexpr cam_name = "cam";
    static auto constexpr cam_desc = "Renderer Camera Functions";

    static auto constexpr selectedObject_name = "selected-object";
    static auto constexpr selectedObject_desc = "Functions to select and interact with a selected RenderObject";

    static auto constexpr env_name = "env";
    static auto constexpr env_desc = "Environment management functions";

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
        bindFunction(&General::dumpView, dumpView_name, dumpView_desc);

        bindCategory(cam_name, cam_desc);
        bindFunction(&General::cam_move, cam_move_name, cam_move_desc);
        bindFunction(&General::cam_set, cam_set_name, cam_set_desc);

        bindCategory(selectedObject_name, selectedObject_desc);
        bindFunction(&General::selectedObject_get, selectedObject_get_name, selectedObject_get_desc);
        bindFunction(&General::selectedObject_Parse, selectedObject_Parse_name, selectedObject_Parse_desc);

        bindCategory(env_name, env_desc);
        bindFunction(&General::envLoad, envLoad_name, envLoad_desc);
        bindFunction(&General::envDeload, envDeload_name, envDeload_desc);
    }

private:
    /**
     * @brief Pointer to the currently selected RenderObject
     * @details Marked mutable to allow modification in const functions
     *          We consider the act of selecting a RenderObject
     *          as a non-logical change to the state of the DomainModule,
     *          to signal that it not act on the moduleScope's data itself.
     */
    Core::RenderObject* selectedRenderObject = nullptr;
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_DOMAINMODULE_RENDERER_GENERAL_HPP
