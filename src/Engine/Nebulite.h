//------------------------------------------------
// Core functionality of the Nebulite Engine

#pragma once

#include <deque>

#include "Renderer.h"
#include "MainFuncTree.h"
#include "ErrorTypes.h"

namespace Nebulite {
    //-------------------------------------------------
    // Types

    // hold tasks that need to be parsed as well as parsing info
    struct taskQueue {
        std::deque<std::string> taskList;
        uint64_t waitCounter = 0;
        bool clearAfterResolving = true;
    };

    

    // Each taskque resolving logs errors encountered and if resolving was stopped due to a critical error
    struct taskQueueResult{
        bool stoppedAtCriticalResult = false;
        std::vector<Nebulite::ERROR_TYPE> errors;
    };

    //-------------------------------------------------
    // Pre-Declaration of global instances in Nebulite scope

    // Objects
    extern Nebulite::MainFuncTree mainFuncTree;
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

    // Function to get and lazily initialize the renderer, if its still nullptr
    Nebulite::Renderer* getRenderer();

    // Check if renderer exists
    bool RendererExists();

    // Resolves a given taskqueue by parsing each line into argc/argv and calling the mainTree on the arguments
    Nebulite::taskQueueResult resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree);

    //--------------------------------------
    // Namespace mainTreeFunctions
    // functions accessible through global functioncalls
    namespace mainTreeFunctions{


    }
}