#pragma once

#include "ErrorTypes.h"
#include "RT_ExpansionWrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace RenderObjectTreeExpansion {
class Logging : public Wrapper<Logging> {
public:
    using Wrapper<Logging>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE echo(int argc, char* argv[]);
    Nebulite::ERROR_TYPE log(int argc, char* argv[]);
    Nebulite::ERROR_TYPE logValue(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions

    void setupBindings(){
        bindFunction(&Logging::echo, "echo", "Prints the arguments to the console");
        bindFunction(&Logging::log, "log", "Logs the RenderObject to a file");
        bindFunction(&Logging::logValue, "log-value", "Logs a specific value");
    }
};
}
}