/**
 * @file GTE_Renderer.h
 * @brief Provides rendering utilities for the Nebulite engine.
 *
 * This file contains a GlobalTree expansion for basic rendering-related functioncalls.
 */

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration for RenderObject selection
class GlobalSpace;  // Forward declaration of domain class GlobalSpace

namespace GlobalSpaceTreeExpansion {

/**
 * @class Nebulite::GlobalSpaceTreeExpansion::Renderer
 * @brief Basic Renderer-Related Functions
 */
class Renderer : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, Renderer> {
public:
    using Wrapper<Nebulite::GlobalSpace, Renderer>::Wrapper;   // Templated constructor from Wrapper, call this->setupBindings()

    /**
     * @brief The Renderer Expansion does not make use of any Render-Updates yet. This function is empty.
     * 
     * If we ever wish to implement special rendering features, we can do so here.
     * However, the Nebulite::Renderer class takes care of the core rendering functionality.
     * 
     * Why not implement here? Because all FuncTree expansions are called, then we call the Renderer update function.
     * Implementing all renderer updates here breaks this separation, as we would then have to specify the update order in
     * GlobalSpaceTree.
     * 
     * Current implementation is, simplified:
     * 
     * ```cpp
     * while(true){
     *      globalSpace.GlobalSpaceTree->update();  // Update FuncTree with all its expansions
     *      globalSpace.getRenderer()->tick();      // Update Renderer
     * }
     * ```
     */
    void update();

    //----------------------------------------
    // Available Functions

    // Spawn a renderobject
    /**
     * @brief Spawns a new render object.
     * 
     * @param argc The argument count
     * @param argv The argument vector: RenderObject as link to json/jsonc file
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE spawn(int argc, char* argv[]);

    /**
     * @brief Loads an environment.
     * 
     * @param argc The argument count
     * @param argv The argument vector: Environment as link to json/jsonc file
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE envload(int argc, char* argv[]);

    /**
     * @brief Deload entire environment, leaving an empty renderer
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE envdeload(int argc, char* argv[]);

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
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE setResolution(int argc, char* argv[]);

    /**
     * @brief Sets fps of renderer
     * 
     * @param argc The argument count
     * @param argv The argument vector: [fps]
     * 
     * Defaults to 60 fps if no argument is provided
     * 
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE setFPS(int argc, char* argv[]);

    /**
     * @brief Toggle fps on/off
     * 
     * @param argc The argument count
     * @param argv The argument vector: [on|off]
     * 
     * Defaults to on if no argument is provided
     * 
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE showFPS(int argc, char* argv[]);

    /**
     * @brief Move cam by a given delta
     * 
     * @param argc The argument count
     * @param argv The argument vector: <dx> <dy>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE moveCam(int argc, char* argv[]);

    /**
     * @brief Set cam to concrete position
     * 
     * @param argc The argument count
     * @param argv The argument vector: <x> <y> <c>
     * 
     * Where <c> determines if the given position is the camera's center
     * 
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE setCam(int argc, char* argv[]);

    /**
     * @brief Create a snapshot of the current renderer screen output
     * 
     * @param argc The argument count
     * @param argv The argument vector: <filename>
     * 
     * Defaults to "./Resources/Snapshots/snapshot.png" if no argument is provided
     * 
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE snapshot(int argc, char* argv[]);

    /**
     * @brief Makes a beep noise
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE beep(int argc, char* argv[]);
    
    /**
     * @brief Inserts a reference to the selected renderobject by ID to the GlobalSpace
     * 
     * If the user ever needs to get an object from the renderer by its ID
     * If an object is found with the given ID, its pointer is stored in GlobalSpace::selectedRenderObject
     * If no object is found, GlobalSpace::selectedRenderObject is set to nullptr
     * 
     * @param argc The argument count
     * @param argv The argument vector: <id>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * @todo If an object is deleted, the reference in GlobalSpace::selectedRenderObject is not cleared!
     * Fix idea: Renderer::update() gets currently attached object as argument: RenderObject**
     * reference to pointer, so we can manipulate the pointer!
     * If its a match, clear the reference
     * Other ideas would be a shared pointer starting at the spawn-logic, but we need to be careful
     * with purging pointer copies in the invoke logic and the potentially needed self-reference
     */
    Nebulite::ERROR_TYPE getObjectFromId(int argc, char* argv[]);

    /**
     * @brief Prints the currently selected renderobject to the console
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE printSelectedObject(int argc, char* argv[]);

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&Renderer::spawn,               "spawn",        "Spawn a renderobject");
        bindFunction(&Renderer::envload,             "env-load",     "Load environment/level");
        bindFunction(&Renderer::envdeload,           "env-deload",   "Deload entire environment");
        bindFunction(&Renderer::setResolution,       "set-res",      "Set resolution of renderer: <x> <y> [scalar]");
        bindFunction(&Renderer::setFPS,              "set-fps",      "Set FPS of renderer: <value>");
        bindFunction(&Renderer::showFPS,             "show-fps",     "Show FPS of renderer: <on/off>");
        bindFunction(&Renderer::moveCam,             "cam-move",     "Move camera to a delta position");
        bindFunction(&Renderer::setCam,              "cam-set",      "Set camera to concrete position");
        bindFunction(&Renderer::snapshot,            "snapshot",     "Create a snapshot of the current renderer state");
        bindFunction(&Renderer::beep,                "beep",         "Beep noise from SDL");
        bindFunction(&Renderer::getObjectFromId,     "get-object",   "Get a renderobject by its ID: <id>");
        bindFunction(&Renderer::printSelectedObject, "print-object", "Print the currently selected renderobject");
    }

private:
    /**
     * @brief Pointer to the currently selected RenderObject
     */
    Nebulite::RenderObject* selectedRenderObject = nullptr;
};
}   // namespace GlobalSpaceTreeExpansion
}   // namespace Nebulite