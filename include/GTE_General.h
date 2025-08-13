#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class GlobalSpace; // Forward declaration of domain class GlobalSpace

namespace GlobalSpaceTreeExpansion {
class General : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, General> {
public:
    using Wrapper<Nebulite::GlobalSpace, General>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    // Evaluate all following expressions before parsing further:
    //
    // calling:     echo $(1+1)         outputs:    $(1+1)
    // calling:     eval echo $(1+1)    outputs:    2.000000
    Nebulite::ERROR_TYPE eval(int argc, char* argv[]);

    // exit entire program
    Nebulite::ERROR_TYPE exitProgram(int argc, char* argv[]);

    // Wait a given amount of frames
    Nebulite::ERROR_TYPE wait(int argc, char* argv[]);

    // Load a scripting file for tasks to do
    Nebulite::ERROR_TYPE loadTaskList(int argc, char* argv[]);

    // for-loop of other functioncalls: for <var> <start> <end> <functioncall>
    Nebulite::ERROR_TYPE forLoop(int argc, char* argv[]);

    // Return custom value of ERROR_TYPE
    Nebulite::ERROR_TYPE func_return(int argc, char* argv[]);

    // Echo a given string to cout
    Nebulite::ERROR_TYPE echo(int argc, char* argv[]);

    // Echo a given string to cerr/errorfile
    Nebulite::ERROR_TYPE error(int argc, char* argv[]);

    // Assert CRITICAL_CUSTOM_ASSERT
    Nebulite::ERROR_TYPE func_assert(int argc, char* argv[]);

    // Set a global variable
    Nebulite::ERROR_TYPE setGlobal(int argc, char* argv[]);

    // Force a global value to a certain value
    Nebulite::ERROR_TYPE forceGlobal(int argc, char* argv[]);

    // Release all forced global values
    Nebulite::ERROR_TYPE clearForceGlobal(int argc, char* argv[]);

    // Save entire game state
    Nebulite::ERROR_TYPE stateSave(int argc, char* argv[]);

    // Load game state
    Nebulite::ERROR_TYPE stateLoad(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        bindFunction(&General::eval,                "eval",                 "Evaluate an expression and execute the result. Example: eval echo $(1+1)");
        bindFunction(&General::exitProgram,         "exit",                 "Exit the program");
        bindFunction(&General::wait,                "wait",                 "Wait a given amount of frames: wait <frames>");
        bindFunction(&General::loadTaskList,        "task",                 "Load a task list from a file: task <filename>");
        bindFunction(&General::forLoop,             "for",                  "Execute a for-loop with a function call: for <var> <start> <end> <functioncall>");
        bindFunction(&General::func_return,         "return",               "Return a custom value");
        bindFunction(&General::echo,                "echo",                 "Echo a string to cout: echo <string>");
        bindFunction(&General::error,               "error",                "Echo a string to cerr/errorfile: error <string>");
        bindFunction(&General::func_assert,         "assert",               "Assert a condition and throw an error if false: assert <condition>");
        bindFunction(&General::setGlobal,           "set-global",           "Set a global variable: set-global <key> <value>");
        bindFunction(&General::forceGlobal,         "force-global",         "Force a global variable to a value: force-global <key> <value>");
        bindFunction(&General::clearForceGlobal,    "force-global-clear",   "Clear all forced global variables");
        bindFunction(&General::stateSave,           "state-save",           "Save the current game state: state-save <name>");
        bindFunction(&General::stateLoad,           "state-load",           "Load a saved game state: state-load <name>");
    }
};
}
}