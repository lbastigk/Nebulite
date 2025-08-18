#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class GlobalSpace; // Forward declaration of domain class GlobalSpace 
namespace GlobalSpaceTreeExpansion {
class Debug : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, Debug> {
public:
    using Wrapper<Nebulite::GlobalSpace, Debug>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //--------------------------------------------------------
    // TEST: Binding a dummy function with a name that already exists in the subtree
    Nebulite::ERROR_TYPE set(int argc, char* argv[]) {
        // Binding a function with the name "set" is not allowed as it already exists in the JSONTree
        return Nebulite::ERROR_TYPE::NONE;
    }

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
    // TODO: move to GTE_General
    Nebulite::ERROR_TYPE always(int argc, char* argv[]);

    // Clears all always-functioncalls
    // TODO: move to GTE_General
    Nebulite::ERROR_TYPE alwaysClear(int argc, char* argv[]);

    // [DEBUG] Get and store a standard renderobject for reference to ./Resources/Renderobjects/standard.jsonc
    Nebulite::ERROR_TYPE render_object(int argc, char** argv);


    
    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        bindFunction(&Debug::errorlog,          "log",                      "Activate/Deactivate error logging: log <on/off>");
        bindFunction(&Debug::printGlobal,       "print-global",             "Print global document");
        bindFunction(&Debug::printState,        "print-state",              "Print current state");
        bindFunction(&Debug::logGlobal,         "log-global",               "Log global document: log-global [filename]");
        bindFunction(&Debug::logState,          "log-state",                "Log current state: log-state [filename]");
        bindFunction(&Debug::always,            "always",                   "Attach function to always run: always <command>");
        bindFunction(&Debug::alwaysClear,       "always-clear",             "Clear all always functions");
        bindFunction(&Debug::render_object,     "standard-render-object",   "Generates a standard render object at ./Resources/Renderobjects/standard.jsonc");

        //-----------------------
        // Example Bindings that will fail

        // TEST: Binding an already existing sub-function
        //bindFunction(&Debug::set, "set", "Dummy function to test binding with existing name in subtree");  // <- THIS WILL FAIL

        // TEST: Binding an already existing function
        //bindFunction(&Debug::set, "log", "Dummy function to test binding with existing name in own tree"); // <- THIS WILL FAIL
    }

private:
    std::streambuf* originalCerrBuf = nullptr;
    std::unique_ptr<std::ofstream> errorFile;
    
    // Current status of error logging
    // false : logging to cerr
    // true  : logging to file
    bool errorLogStatus = false;
};
}
}