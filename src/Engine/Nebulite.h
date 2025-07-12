//------------------------------------------------
// Core functionality of the Nebulite Engine

#pragma once

#include <deque>
#include <memory>

#include "Renderer.h"
#include "FuncTree.h"
#include "JSON.h"


// Nebulite instances
namespace Nebulite {
    
    struct taskQueue {
        std::deque<std::string> taskList;
        uint64_t waitCounter = 0;
        bool clearAfterResolving = true;
    };

    //--------------------------------------
    // Declare global instances

    // Objects
    extern FuncTree mainTree;
    extern std::unique_ptr<Nebulite::JSON> global;
    extern std::unique_ptr<Renderer> renderer;
    extern Invoke invoke;

    // Tasks
    extern taskQueue tasks_script;
    extern taskQueue tasks_internal;
    extern taskQueue tasks_always;

    // Error log stuff
    extern bool errorLogStatus;
    extern std::ofstream errorFile;
    extern std::streambuf* originalCerrBuf;

    // Names
    extern std::string stateName;
    extern std::string binName;

    //--------------------------------------
    // Functions
    
    // init core variables
    void init();

    // Function to init nebulite arg-bounded functions
    void init_functions();

    // Function to get and lazily initialize the renderer, if its still nullptr
    Nebulite::Renderer* getRenderer();

    // Converting string cmd to argc/argv
    void convertStrToArgcArgv(const std::string& cmd, int& argc, char**& argv);

    // Resolves a given taskqueue by parsing each line into argc/argv and calling the mainTree on the arguments
    int resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree);

    //--------------------------------------
    // Namespace mainTreeFunctions
    // functions accessible through global functioncalls
    namespace mainTreeFunctions{

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
        int eval(int argc, char* argv[]);
        
        // Load environment/level
        int envload(int argc, char* argv[]);

        // deload entire environment, leaving an empty renderer
        int envdeload(int argc, char* argv[]);

        // Spawn a renderobject
        int spawn(int argc, char* argv[]);

        // exit entire program
        int exitProgram(int argc, char* argv[]);

        // Save entire game state
        int stateSave(int argc, char* argv[]);

        // Load game state
        int stateLoad(int argc, char* argv[]);

        // Wait a given amount of frames
        int wait(int argc, char* argv[]);

        // Load a scripting file for tasks to do
        int loadTaskList(int argc, char* argv[]);

        // Echo a given string to cout
        int echo(int argc, char* argv[]);

        // for-loop of other functioncalls: for <var> <start> <end> <functioncall>
        int forLoop(int argc, char* argv[]);

        // Echo a given string to cerr
        int error(int argc, char* argv[]);

        // Sets resolution of renderer
        int setResolution(int argc, char* argv[]);

        // Sets fps of renderer
        int setFPS(int argc, char* argv[]);

        // Move cam to a delta position
        int moveCam(int argc, char* argv[]);

        // Set cam to concrete position
        int setCam(int argc, char* argv[]);

        // Print global doc to cout
        int printGlobal(int argc, char* argv[]);

        // Print state to cout
        int printState(int argc, char* argv[]);

        // Log global doc to file
        int logGlobal(int argc, char* argv[]);

        // Log state to file
        int logState(int argc, char* argv[]);

        // Set a global variable
        int setGlobal(int argc, char* argv[]);

        // Error log activation/deactivation
        int errorlog(int argc, char* argv[]);

        // Attaches functioncall that is executed on each tick
        int always(int argc, char* argv[]);

        // Clears all always-functioncalls
        int alwaysClear(int argc, char* argv[]);

        // [DEBUG] Get and store a standard renderobject for reference to ./Resources/Renderobjects/standard.json
        int render_object(int argc, char** argv);
    }

}