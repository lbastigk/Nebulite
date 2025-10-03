/**
 * @file GDM_Renderer.hpp
 * @brief Provides rendering utilities for the Nebulite engine.
 *
 * This file contains a GlobalTree DomainModule for basic rendering-related functioncalls.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
        class RenderObject; // Forward declaration of RenderObject
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Renderer
 * @brief Basic Renderer-Related Functions
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Renderer) {
public:
    /**
     * @brief The Renderer DomainModule does not make use of any Render-Updates yet. This function is empty.
     * 
     * If we ever wish to implement special rendering features, we can do so here.
     * However, the Nebulite::Core::Renderer class takes care of the core rendering functionality.
     * 
     * Why not implement here? Because all DomainModules are called, then we call the Renderer update function.
     * Implementing all renderer updates here breaks this separation, as we would then have to specify the update order
     * of the GlobalSpace DomainModules.
     * 
     * Current implementation is, simplified:
     * 
     * ```cpp
     * while(true){
     *      globalSpace.update();               // Update with all its DomainModules
     *      globalSpace.getRenderer()->tick();  // Update Renderer
     * }
     * ```
     */
    void update();

    //------------------------------------------
    // Available Functions

    // Spawn a renderobject
    /**
     * @brief Spawns a new render object.
     * 
     * Memory management ist handled by the Renderer
     * 
     * Implementing `unique_ptr` or `shared_ptr` is a work in progress
     * as its made difficult by the ability of globalspace to select a RenderObject
     * and store its pointer.
     * The Renderer is, besides the selection addition from `selectedObject_get`,
     * a closed system that handles the pointer and lifetime of RenderObjects.
     * Thus, the usage of `unique_ptr` or `shared_ptr` is not needed here, 
     * but perhaps helpful if complexity increases.
     * 
     * We might wish to implement this in the future, but for now
     * we will use a raw pointer and let the Renderer handle the memory management.
     * 
     * Implementing a shared or unique pointer would require a significant rework of:
     * 
     * - Renderer append function
     * 
     * - Environment append function
     * 
     * - RenderObjectContainer append function
     * 
     * - Its batch management
     * 
     * - RenderObject selection mechanism
     * 
     * - RenderObject deletion mechanism in Renderer::update()
     * 
     * As of now, the implementation is fully functional so its a low priority task.
     * 
     * @param argc The argument count
     * @param argv The argument vector: RenderObject as link to json/jsonc file
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error spawn(int argc, char* argv[]);
    std::string spawn_desc = R"(Spawn a RenderObject from a json/jsonc file.

    Usage: spawn <path/to/file.jsonc>

    Supports lookups in standard resource directories:
    - ./Resources/RenderObjects/
    - ./Resources/Renderobjects/

    Example: 'spawn Planets/sun.jsonc|set text.str This is a sun'
    Looks for object 'sun.jsonc' in the standard directories
    - './Planets/sun.jsonc'
    - './Resources/RenderObjects/Planets/sun.jsonc'
    - './Resources/Renderobjects/Planets/sun.jsonc'
    and spawns the first found object.
    )"; 

    /**
     * @brief Loads an environment.
     * 
     * @param argc The argument count
     * @param argv The argument vector: Environment as link to json/jsonc file
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error env_load(int argc, char* argv[]);
    std::string env_load_desc = R"(Load an environment/level from a json/jsonc file.

    Usage: env load <path/to/file.jsonc>

    If no argument is provided, an empty environment is loaded.
    )";

    /**
     * @brief Deload entire environment, leaving an empty renderer
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error env_deload(int argc, char* argv[]);
    std::string env_deload_desc = R"(Deload entire environment, leaving an empty renderer.

    Usage: env deload

    )";

    /**
     * @brief Sets resolution of renderer
     * 
     * @param argc The argument count
     * @param argv The argument vector: [Width] [Height] [Scale]. 
     * 
     * Defaults to 1    for scale if argument count < 3
     * 
     * Defaults to 1000 for height if argument count < 2
     * 
     * Defaults to 1000 for width if argument count < 1
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error setResolution(int argc, char* argv[]);
    std::string setResolution_desc = R"(Set resolution of renderer.

    Usage: set-res [Width] [Height] [Scale]

    Defaults to 1000  for width if argument count < 1
    Defaults to 1000  for height if argument count < 2
    Defaults to 1     for scale if argument count < 3
    )";

    /**
     * @brief Sets fps of renderer
     * 
     * @param argc The argument count
     * @param argv The argument vector: [fps]
     * 
     * Defaults to 60 fps if no argument is provided
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error setFPS(int argc, char* argv[]);
    std::string setFPS_desc = R"(Set FPS of renderer.

    Usage: set-fps [fps]

    Defaults to 60 fps if no argument is provided
    )";

    /**
     * @brief Toggle fps on/off
     * 
     * @param argc The argument count
     * @param argv The argument vector: [on|off]
     * 
     * Defaults to on if no argument is provided
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error showFPS(int argc, char* argv[]);
    std::string showFPS_desc = R"(Show FPS of renderer.

    Usage: show-fps [on|off]

    Defaults to on if no argument is provided
    )";

    /**
     * @brief Move cam by a given delta
     * 
     * @param argc The argument count
     * @param argv The argument vector: <dx> <dy>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error cam_move(int argc, char* argv[]);
    std::string cam_move_desc = R"(Move camera by a given delta.

    Usage: cam move <dx> <dy>

    )";

    /**
     * @brief Set cam to concrete position
     * 
     * @param argc The argument count
     * @param argv The argument vector: <x> <y> [c]
     * 
     * Where [c] determines if the given position is the camera's center
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error cam_set(int argc, char* argv[]);
    std::string cam_set_desc = R"(Set camera to concrete position.

    Usage: cam set <x> <y> [c]

    Where [c] determines if the given position is the camera's center

    )";

    /**
     * @brief Create a snapshot of the current renderer screen output
     * 
     * @param argc The argument count
     * @param argv The argument vector: <filename>
     * 
     * Defaults to "./Resources/Snapshots/snapshot.png" if no argument is provided
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error snapshot(int argc, char* argv[]);
    std::string snapshot_desc = R"(Create a snapshot of the current renderer state.

    Usage: snapshot [filename]

    Defaults to "./Resources/Snapshots/snapshot.png" if no argument is provided

    )";

    /**
     * @brief Makes a beep noise
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error beep(int argc, char* argv[]);
    std::string beep_desc = R"(Make a beep noise.

    Usage: beep

    )";
    
    /**
     * @brief Inserts a reference to the selected renderobject by ID to the GlobalSpace
     * 
     * If the user ever needs to get an object from the renderer by its ID
     * If an object is found with the given ID, its pointer is stored in GlobalSpace::selectedRenderObject
     * If no object is found, GlobalSpace::selectedRenderObject is set to nullptr
     * 
     * @param argc The argument count
     * @param argv The argument vector: <id>
     * @return Potential errors that occured on command execution
     * 
     * @todo If an object is deleted, the reference in GlobalSpace::selectedRenderObject is not cleared!
     * Fix idea: Make Renderer a domain itself, with this function as a domainmodule part
     * Then, we can have the SelectedRenderObject as a private member of the Renderer domain
     * and manipulate it directly in Renderer::update()
     * This would also make sense, as the Renderer is the owner of the RenderObjects
     * and should thus also be the owner of the selected object reference
     */
    Nebulite::Constants::Error selectedObject_get(int argc, char* argv[]);
    std::string selectedObject_get_desc = R"(Get a renderobject by its ID.

    Usage: selected-object get <id>

    )";

    /**
     * @brief Parses a command on the selected RenderObject
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error selectedObject_Parse(int argc, char* argv[]);
    std::string selectedObject_Parse_desc = R"(Parse a command on the selected RenderObject.

    Usage: selected-object parse <command>

    Use 'selected-object get <id>' to select a RenderObject first.
    Use 'selected-object parse help' to see available commands for the selected object.
    )";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     * 
     * @todo Move functions for Renderer and Environment to domains themselves,
     * once they are implemented as such.
     * 
     * This will declutter the globalspace, separating its usage from the Renderer and Environment.
     * The only downside currently is that we have to implement a method to lazy-init the SDL Renderer within the Renderer domain itself.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Renderer){
        bindFunction(&Renderer::spawn,               "spawn",        spawn_desc);
        bindFunction(&Renderer::setResolution,       "set-res",      setResolution_desc);
        bindFunction(&Renderer::setFPS,              "set-fps",      setFPS_desc);
        bindFunction(&Renderer::showFPS,             "show-fps",     showFPS_desc);
        bindFunction(&Renderer::snapshot,            "snapshot",     snapshot_desc);
        bindFunction(&Renderer::beep,                "beep",         beep_desc);

        bindSubtree("cam", "Renderer Camera Functions");
        bindFunction(&Renderer::cam_move,             "cam move",     cam_move_desc);
        bindFunction(&Renderer::cam_set,              "cam set",      cam_set_desc);

        bindSubtree("selected-object", "Functions to select and interact with a selected RenderObject");
        bindFunction(&Renderer::selectedObject_get,   "selected-object get",   selectedObject_get_desc);
        bindFunction(&Renderer::selectedObject_Parse, "selected-object parse", selectedObject_Parse_desc);

        bindSubtree("env", "Environment management functions");
        bindFunction(&Renderer::env_load,             "env load",     env_load_desc);
        bindFunction(&Renderer::env_deload,           "env deload",   env_deload_desc);
    }

private:
    /**
     * @brief Pointer to the currently selected RenderObject
     */
    Nebulite::Core::RenderObject* selectedRenderObject = nullptr;
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite