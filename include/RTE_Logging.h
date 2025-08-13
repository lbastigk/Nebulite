#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration of Domain class RenderObject

namespace RenderObjectTreeExpansion {
class Logging : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::RenderObject, Logging> {
public:
    using Wrapper<Nebulite::RenderObject, Logging>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE echo(int argc, char* argv[]);
    Nebulite::ERROR_TYPE log(int argc, char* argv[]);
    Nebulite::ERROR_TYPE logValue(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions

    void setupBindings() {
        bindFunction(&Logging::echo, "echo", "Prints the arguments to the console");
        bindFunction(&Logging::log, "log", "Logs the RenderObject to a file");
        bindFunction(&Logging::logValue, "log-value", "Logs a specific value");
    }
};
}
}