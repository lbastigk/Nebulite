#pragma once

#include "ErrorTypes.h"
#include "MT__Wrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace MainTreeCategory {
class Debug : public Wrapper<Debug> {
public:
    using Wrapper<Debug>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions

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

    //----------------------------------------
    // Binding Functions

    void setupBindings(){
        bindFunction(&Debug::errorlog,          "log",                      "Activate/Deactivate error logging");
        bindFunction(&Debug::printGlobal,       "print-global",             "Print global document");
        bindFunction(&Debug::printState,        "print-state",              "Print current state");
        bindFunction(&Debug::logGlobal,         "log-global",               "Log global document");
        bindFunction(&Debug::logState,          "log-state",                "Log current state");
        bindFunction(&Debug::always,            "always",                   "Attach function to always run");
        bindFunction(&Debug::alwaysClear,       "always-clear",             "Clear all always functions");
        bindFunction(&Debug::render_object,     "standard-render-object",   "Get standard render object");
        bindFunction(&Debug::printVar,          "print-var",                "Print variable");
    }
};

}
}