/**
 * @file GTE_Renderer.h
 * @brief Provides rendering utilities for the Nebulite engine.
 *
 * This file contains a GlobalTree extension to handle rendering.
 */

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration for RenderObject selection
class GlobalSpace;  // Forward declaration of domain class GlobalSpace

namespace GlobalSpaceTreeExpansion {

/**
 * @brief Basic Renderer-Related Functions
 */
class Renderer : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, Renderer> {
public:
    using Wrapper<Nebulite::GlobalSpace, Renderer>::Wrapper;   // Templated constructor from Wrapper, call this->setupBindings()

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

    // Load environment/level
    /**
     * @brief ....
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE envload(int argc, char* argv[]);

    // deload entire environment, leaving an empty renderer
    /**
     * @brief ....
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE envdeload(int argc, char* argv[]);

    // Sets resolution of renderer
    /**
     * @brief ....
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE setResolution(int argc, char* argv[]);

    // Sets fps of renderer
    /**
     * @brief ....
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE setFPS(int argc, char* argv[]);

    // Toggle fps on/off
    /**
     * @brief ....
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE showFPS(int argc, char* argv[]);

    // Move cam to a delta position
    /**
     * @brief ....
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE moveCam(int argc, char* argv[]);

    // Set cam to concrete position
    /**
     * @brief ....
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE setCam(int argc, char* argv[]);

    // Create a snapshot of the current renderer state
    /**
     * @brief ....
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE snapshot(int argc, char* argv[]);

    /**
     * @brief Makes a beep noise
     * 
     * @param argc The argument count
     * @param argv The argument vector: no inputs available
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
     */
    Nebulite::ERROR_TYPE printSelectedObject(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
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
    // Currently selected RenderObject
    Nebulite::RenderObject* selectedRenderObject = nullptr;
};
}
}