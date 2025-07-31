#pragma once

#include "ErrorTypes.h"
#include "MT__Wrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace MainTreeCategory {
class Renderer : public Wrapper<Renderer> {
public:
    using Wrapper<Renderer>::Wrapper;   // Templated constructor from Wrapper, call this->setupBindings()

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

    // Move cam to a delta position
    Nebulite::ERROR_TYPE moveCam(int argc, char* argv[]);

    // Set cam to concrete position
    Nebulite::ERROR_TYPE setCam(int argc, char* argv[]);

    // Create a snapshot of the current renderer state
    Nebulite::ERROR_TYPE snapshot(int argc, char* argv[]);

    // Beep noise, from SDL
    Nebulite::ERROR_TYPE beep(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        bindFunction(&Renderer::spawn,              "spawn",        "Spawn a renderobject");
        bindFunction(&Renderer::envload,            "env-load",     "Load environment/level");
        bindFunction(&Renderer::envdeload,          "env-deload",   "Deload entire environment");
        bindFunction(&Renderer::setResolution,      "set-res",      "Set resolution of renderer");
        bindFunction(&Renderer::setFPS,             "set-fps",      "Set FPS of renderer");
        bindFunction(&Renderer::moveCam,            "cam-move",     "Move camera to a delta position");
        bindFunction(&Renderer::setCam,             "cam-set",      "Set camera to concrete position");
        bindFunction(&Renderer::snapshot,           "snapshot",     "Create a snapshot of the current renderer state");
        bindFunction(&Renderer::beep,               "beep",         "Beep noise from SDL");
    }
};
}
}