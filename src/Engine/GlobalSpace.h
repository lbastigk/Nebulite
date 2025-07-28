/*
 * GlobalSpace.h
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
 * See main.cpp and other engine modules for usage examples and integration details.
 */

//------------------------------------------------
// Core functionality of the Nebulite Engine

#pragma once

#include <deque>

#include "Renderer.h"
#include "ErrorTypes.h"
#include "MainTree.h"

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
    // Global Space object
    class GlobalSpace {
    public:
        //-------------------------------------------------
        // Special Member Functions

        // Constructor
        GlobalSpace(const std::string binName, std::streambuf*& originalCerrBufRef);

        // Destructor
        ~GlobalSpace() = default;

        // Prevent copying
        GlobalSpace(const GlobalSpace&) = delete;
        GlobalSpace& operator=(const GlobalSpace&) = delete;

        // Prevent moving
        GlobalSpace(GlobalSpace&&) = delete;
        GlobalSpace& operator=(GlobalSpace&&) = delete;

        //-------------------------------------------------
        // Functions

        // Function to get and lazily initialize the renderer, if its still nullptr
        Nebulite::Renderer* getRenderer();

        // Check if renderer exists
        bool RendererExists();

        // Resolves a given taskqueue by parsing each line into argc/argv and calling the mainTree on the arguments
        Nebulite::taskQueueResult resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree);

        // Instead of calling MainTree.parse, this function resolves the task queue and returns the result
        Nebulite::ERROR_TYPE resolveTask(std::string task);

        //----------------------------------------------
        // Public Variables

        // Task queues
        Nebulite::taskQueue tasks_script;
        Nebulite::taskQueue tasks_internal;
        Nebulite::taskQueue tasks_always;

    private:
        //----------------------------------------------
        // Private Variables

        friend class Nebulite::MainTree; // Allow MainTree to access private members

        // General variables
        std::string stateName;
        std::string _binName;

        // Objects
        std::unique_ptr<Invoke> invoke;
        std::unique_ptr<Nebulite::MainTree> mainTree;
        Nebulite::JSON* global;
        Nebulite::Renderer* renderer;

        // Error log variables
        bool errorLogStatus;
        std::unique_ptr<std::ofstream> errorFile;
        std::streambuf*& originalCerrBuf;
    };
}