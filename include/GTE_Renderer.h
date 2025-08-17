#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration for RenderObject selection
class GlobalSpace;  // Forward declaration of domain class GlobalSpace

namespace GlobalSpaceTreeExpansion {
class Renderer : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, Renderer> {
public:
    using Wrapper<Nebulite::GlobalSpace, Renderer>::Wrapper;   // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    // Spawn a renderobject
    Nebulite::ERROR_TYPE spawn(int argc, char* argv[]);

    // Load environment/level
    Nebulite::ERROR_TYPE envload(int argc, char* argv[]);

    // deload entire environment, leaving an empty renderer
    Nebulite::ERROR_TYPE envdeload(int argc, char* argv[]);

    // Sets resolution of renderer
    Nebulite::ERROR_TYPE setResolution(int argc, char* argv[]);

    // Sets fps of renderer
    Nebulite::ERROR_TYPE setFPS(int argc, char* argv[]);

    // Toggle fps on/off
    Nebulite::ERROR_TYPE showFPS(int argc, char* argv[]);

    // Move cam to a delta position
    Nebulite::ERROR_TYPE moveCam(int argc, char* argv[]);

    // Set cam to concrete position
    Nebulite::ERROR_TYPE setCam(int argc, char* argv[]);

    // Create a snapshot of the current renderer state
    Nebulite::ERROR_TYPE snapshot(int argc, char* argv[]);

    // Beep noise, from SDL
    Nebulite::ERROR_TYPE beep(int argc, char* argv[]);

    // If the user ever needs to get an object from the renderer by its ID
    // If an object is found with the given ID, its pointer is stored in GlobalSpace::selectedRenderObject
    // If no object is found, GlobalSpace::selectedRenderObject is set to nullptr
    //
    // TODO: If an object is deleted, the reference in GlobalSpace::selectedRenderObject is not cleared!
    // Fix idea: Renderer::update() gets currently attached object as argument: RenderObject** 
    // reference to pointer, so we can manipulate the pointer!
    // If its a match, clear the reference
    Nebulite::ERROR_TYPE getObjectFromId(int argc, char* argv[]);

    // Print selected renderobject
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