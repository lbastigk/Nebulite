#pragma once

#include "ErrorTypes.h"
#include "FuncTreeWrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace MainTreeCategory {
class Renderer {
public:
    Renderer(Invoke* invoke, Nebulite::GlobalSpace* globalSpace, FuncTree<ERROR_TYPE>* funcTreePtr) 
        : global(globalSpace), invoke(invoke), funcTree(funcTreePtr) {}

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
private:
    Nebulite::GlobalSpace* global;
    Nebulite::Invoke* invoke;
    FuncTree<ERROR_TYPE>* funcTree;
};
}
}