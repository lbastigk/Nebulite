#pragma once

#include "ErrorTypes.h"
#include "FuncTreeWrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace MainTreeCategory {
class General {
public:
    General(Invoke* invoke, Nebulite::GlobalSpace* globalSpace, FuncTree<ERROR_TYPE>* funcTreePtr) 
        : global(globalSpace), invoke(invoke), funcTree(funcTreePtr) {}

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
private:
    Nebulite::GlobalSpace* global;
    Nebulite::Invoke* invoke;
    FuncTree<ERROR_TYPE>* funcTree;
};
}
}