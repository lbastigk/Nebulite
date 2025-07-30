#pragma once

#include "ErrorTypes.h"
#include "FuncTreeWrapper.h"
#include "Invoke.h"

namespace Nebulite{

// Forwards declaration instead of including GlobalSpace.h
class GlobalSpace;  

class MainTree : public FuncTreeWrapper<ERROR_TYPE>{
public:
    MainTree(Nebulite::Invoke* invoke, Nebulite::GlobalSpace* globalSpace);

    //---------------------------------------
    // Internal Variables
    std::string headless = "false";
private:
    // TODO:
    /*
    
    - if
    - while
    - renderer class of function, calling in-renderer specific tasks?
        instead of set-fps or move-cam, just have a function called renderer
        that parses the rest inside the renderer tree:
        renderer set-fps 60
        renderer move-cam 100 100
        renderer count
        etc...
        it might become too convoluted to use though, since basically all functions are tied to the renderer?
    - force/release
        forcing a global value to a certain value
        release does the opposite, allowing other routines to manipulate this value again
        useful for TAS: force keyboard.current.up 1 ; wait 100 ; release keyboard.current.up
    - setting
        for saving/loading/manipulating a settings.json file
    */

    // Evaluate all following expressions before parsing further:
    //
    // calling:     echo $(1+1)         outputs:    $(1+1)
    // calling:     eval echo $(1+1)    outputs:    2.000000
    Nebulite::ERROR_TYPE eval(int argc, char* argv[]);
    
    // Load environment/level
    Nebulite::ERROR_TYPE envload(int argc, char* argv[]);

    // deload entire environment, leaving an empty renderer
    Nebulite::ERROR_TYPE envdeload(int argc, char* argv[]);

    // Spawn a renderobject
    Nebulite::ERROR_TYPE spawn(int argc, char* argv[]);

    // exit entire program
    Nebulite::ERROR_TYPE exitProgram(int argc, char* argv[]);

    // Save entire game state
    Nebulite::ERROR_TYPE stateSave(int argc, char* argv[]);

    // Load game state
    Nebulite::ERROR_TYPE stateLoad(int argc, char* argv[]);

    // Wait a given amount of frames
    Nebulite::ERROR_TYPE wait(int argc, char* argv[]);

    // Load a scripting file for tasks to do
    Nebulite::ERROR_TYPE loadTaskList(int argc, char* argv[]);

    // Echo a given string to cout
    Nebulite::ERROR_TYPE echo(int argc, char* argv[]);

    // for-loop of other functioncalls: for <var> <start> <end> <functioncall>
    Nebulite::ERROR_TYPE forLoop(int argc, char* argv[]);

    // Echo a given string to cerr
    Nebulite::ERROR_TYPE error(int argc, char* argv[]);

    // Assert CRITICAL_CUSTOM_ASSERT
    Nebulite::ERROR_TYPE func_assert(int argc, char* argv[]);

    // Return custom value
    Nebulite::ERROR_TYPE func_return(int argc, char* argv[]);

    // Sets resolution of renderer
    Nebulite::ERROR_TYPE setResolution(int argc, char* argv[]);

    // Sets fps of renderer
    Nebulite::ERROR_TYPE setFPS(int argc, char* argv[]);

    // Move cam to a delta position
    Nebulite::ERROR_TYPE moveCam(int argc, char* argv[]);

    // Set cam to concrete position
    Nebulite::ERROR_TYPE setCam(int argc, char* argv[]);

    // Print global doc to cout
    Nebulite::ERROR_TYPE printGlobal(int argc, char* argv[]);

    // Print state to cout
    Nebulite::ERROR_TYPE printState(int argc, char* argv[]);

    // Log global doc to file
    Nebulite::ERROR_TYPE logGlobal(int argc, char* argv[]);

    // Log state to file
    Nebulite::ERROR_TYPE logState(int argc, char* argv[]);

    // Set a global variable
    Nebulite::ERROR_TYPE setGlobal(int argc, char* argv[]);

    // Error log activation/deactivation
    Nebulite::ERROR_TYPE errorlog(int argc, char* argv[]);

    // Attaches functioncall that is executed on each tick
    Nebulite::ERROR_TYPE always(int argc, char* argv[]);

    // Clears all always-functioncalls
    Nebulite::ERROR_TYPE alwaysClear(int argc, char* argv[]);

    // Create a snapshot of the current renderer state
    Nebulite::ERROR_TYPE snapshot(int argc, char* argv[]);

    // [DEBUG] Get and store a standard renderobject for reference to ./Resources/Renderobjects/standard.json
    Nebulite::ERROR_TYPE render_object(int argc, char** argv);

    // Beep function for debugging, from SDL
    Nebulite::ERROR_TYPE beep(int argc, char* argv[]);
    
    // Print all internal values
    Nebulite::ERROR_TYPE printVar(int argc, char** argv);

    // Force a global value to a certain value
    Nebulite::ERROR_TYPE forceGlobal(int argc, char* argv[]);

    // Release all forced global values
    Nebulite::ERROR_TYPE clearForceGlobal(int argc, char* argv[]);

    //----------------------------------
    // Private Variables
    Nebulite::Invoke* invoke_ptr;
    Nebulite::GlobalSpace* self;
};

}


