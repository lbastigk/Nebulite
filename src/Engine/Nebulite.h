/*
 * Nebulite.h
 * ----------
 * Declares the core types, global objects, and functions for the Nebulite Engine.
 *
 * Overview:
 *   - Provides the main engine interface, including task queue management, renderer access,
 *     and error logging facilities.
 *   - Defines the `taskQueue` and `taskQueueResult` structures for managing and tracking
 *     the execution of queued engine tasks.
 *   - Declares global engine objects (such as the main function tree, renderer, and global state)
 *     and task queues used throughout the engine.
 *   - Exposes functions for engine initialization, renderer management, and task queue resolution.
 *
 * Key Components:
 *   - taskQueue: Holds a list of tasks to be executed, along with parsing and state info.
 *   - taskQueueResult: Stores the result of processing a task queue, including error codes and
 *     whether execution was stopped due to a critical error.
 *   - MainTree: The main function tree for parsing and executing engine commands.
 *   - renderer: Pointer to the main rendering engine, lazily initialized.
 *   - error logging: Facilities for redirecting and storing error output.
 *   - stateName, binName: Strings for tracking the current engine state and binary name.
 *
 * Main Functions:
 *   - init(): Initializes core engine variables and state.
 *   - getRenderer(): Returns a pointer to the renderer, initializing it if necessary.
 *   - RendererExists(): Checks if the renderer has been initialized.
 *   - resolveTaskQueue(): Parses and executes all tasks in a given queue, returning a result
 *     structure with error information.
 *
 * See main.cpp and other engine modules for usage examples and integration details.
 */

//------------------------------------------------
// Core functionality of the Nebulite Engine

#pragma once

#include <deque>

#include "Renderer.h"
#include "ErrorTypes.h"

// TODO: Rework into class Nebulite::Global or similar
//       to encapsulate all global state and functionality.
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
    //Nebulite::MainTree MainTree;
    extern Invoke invoke;
    extern Nebulite::JSON* global;
    extern Nebulite::Renderer* renderer; 

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

    // Instead of calling MainTree.parse, this function resolves the task queue and returns the result
    Nebulite::ERROR_TYPE resolveTask(std::string task);

}