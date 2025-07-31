#pragma once

#include "ErrorTypes.h"
#include "FuncTreeWrapper.h"

namespace Nebulite{

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace MainTreeCategory {
class Debug {
public:
    Debug(Invoke* invoke, Nebulite::GlobalSpace* globalSpace, FuncTree<ERROR_TYPE>* funcTreePtr) 
        : global(globalSpace), invoke(invoke), funcTree(funcTreePtr) {}

    // Error log activation/deactivation
    Nebulite::ERROR_TYPE errorlog(int argc, char* argv[]);

    // Print global doc to cout
    Nebulite::ERROR_TYPE printGlobal(int argc, char* argv[]);

    // Print state to cout
    Nebulite::ERROR_TYPE printState(int argc, char* argv[]);

    // Log global doc to file
    Nebulite::ERROR_TYPE logGlobal(int argc, char* argv[]);

    // Log state to file
    Nebulite::ERROR_TYPE logState(int argc, char* argv[]);

    // Attaches functioncall that is executed on each tick
    Nebulite::ERROR_TYPE always(int argc, char* argv[]);

    // Clears all always-functioncalls
    Nebulite::ERROR_TYPE alwaysClear(int argc, char* argv[]);

    // [DEBUG] Get and store a standard renderobject for reference to ./Resources/Renderobjects/standard.json
    Nebulite::ERROR_TYPE render_object(int argc, char** argv);

    // Print all internal values
    Nebulite::ERROR_TYPE printVar(int argc, char** argv);  
private:
    Nebulite::GlobalSpace* global;
    Nebulite::Invoke* invoke;
    FuncTree<ERROR_TYPE>* funcTree;
};
}
}