
/**
 * @file GlobalSpace.h
 * 
 * GlobalSpace class declaration for the Nebulite Engine for core functionality.
 */


//------------------------------------------------
// Core functionality of the Nebulite Engine

#pragma once

#include <deque>

#include "Renderer.h"
#include "ErrorTypes.h"
#include "GlobalSpaceTree.h"

namespace Nebulite {

    //-------------------------------------------------
    // Types

    // hold tasks that need to be parsed as well as parsing info
    /**
     * @brief Represents a queue of tasks to be processed.
     */
    struct taskQueue {
        std::deque<std::string> taskList;
        bool clearAfterResolving = true;
    };

    /**
     * @brief Represents the result of resolving a task queue.
     *
     * This structure holds the outcome of processing a task queue, including any errors
     * encountered during resolution and whether the process was halted due to a critical error.
     */
    struct taskQueueResult{
        bool stoppedAtCriticalResult = false;
        std::vector<Nebulite::ERROR_TYPE> errors;
    };

    //-------------------------------------------------
    // Global Space object

    /**
     * @brief Declares the core types, global objects, and functions for the Nebulite Engine
     *
     * Overview:
     * 
     *   - Provides the main engine interface, including task queue management, renderer access,
     *     and error logging facilities.
     * 
     *   - Defines the `taskQueue` and `taskQueueResult` structures for managing and tracking
     *     the execution of queued engine tasks.
     * 
     *   - Declares global engine objects (such as the main function tree, renderer, and global state)
     *     and task queues used throughout the engine.
     * 
     *   - Exposes functions for engine initialization, renderer management, and task queue resolution.
     *
     * Key Components:
     * 
     *   - taskQueue: Holds a list of tasks to be executed, along with parsing and state info.
     * 
     *   - taskQueueResult: Stores the result of processing a task queue, including error codes and
     *     whether execution was stopped due to a critical error.
     * 
     *   - GlobalSpaceTree: The main function tree for parsing and executing engine commands.
     * 
     *   - renderer: Pointer to the main rendering engine, lazily initialized.
     * 
     *   - error logging: Facilities for redirecting and storing error output.
     * 
     *   - stateName, binName: Strings for tracking the current engine state and binary name.
     *
     * See main.cpp and other engine modules for usage examples and integration details.
     */
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

        /**
         * @brief Allows for access to the Renderer instance
         *
         * This function retrieves the renderer instance, creating it if it doesn't already exist.
         *
         * @return A pointer to the Renderer instance.
         */
        Nebulite::Renderer* getRenderer();

        /**
         * @brief Checks if the renderer instance exists.
         * 
         * @return True if the renderer exists, false otherwise.
         */
        bool RendererExists();

        /**
         * @brief Resolves a task queue by parsing each task and executing it.
         * 
         * @param tq The task queue to resolve.
         * @param waitCounter A counter for checking if the task execution should wait a certain amount of frames.
         * @return The result of the task queue resolution.
         */
        Nebulite::taskQueueResult resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* waitCounter);

        /**
         * @brief Parses a given command string in the GlobalSpaceTree
         * 
         * The function ensures that the first argument is the binary name
         * 
         * @return The error type resulting from the parsing operation.
         */
        Nebulite::ERROR_TYPE parseStr(std::string str);

        //----------------------------------------------
        // Public Variables

        // Task queues

        // Task queue for script files loaded with "task"
        Nebulite::taskQueue tasks_script;

        // Internal task queue from renerObjects, console, etc.
        Nebulite::taskQueue tasks_internal;

        // Always-tasks added with the prefix "always "
        Nebulite::taskQueue tasks_always;

        // Wait counter for script tasks
        uint64_t scriptWaitCounter = 0;

        // Error Table for error descriptions
        ErrorTable errorTable;

        //-------------------------------------------------
        // Removal of private keyword for easier access for Expansion classes
        // This allows for easier extension and modification of the GlobalSpace
        // without having to specify its access here
    /*
    private:
        //-------------------------------------------------
        // Allow GlobalSpaceTree Categories to access private members
        friend class Nebulite::GlobalSpaceTreeExpansion::General;
        friend class Nebulite::GlobalSpaceTreeExpansion::Renderer;
        friend class Nebulite::GlobalSpaceTreeExpansion::Debug;
    //*/

        

        //-------------------------------------------------
        // Internal Variables, linked to GlobalSpaceTree
        std::string headless = "false"; // Headless mode (no window)
        std::string recover = "false";  // Enable recoverable error mode
        /*Add more variables as needed*/

        //-------------------------------------------------
        // Other Variables

        // Name of the state where files are saved (equal to savegame name)
        std::string stateName;

        // Name of the binary, used for parsing arguments
        std::string _binName;

        // Current status of error logging
        // false : logging to cerr
        // true  : logging to file
        bool errorLogStatus;

        //---------------------------------------
        // Objects

        // Invoke Object for parsing expressions etc.
        std::unique_ptr<Invoke> invoke;

        // GlobalSpaceTree for parsing and executing commands
        std::unique_ptr<Nebulite::GlobalSpaceTree> GlobalSpaceTree;

        // Global Space document 
        Nebulite::JSON global;

        // Pointer to the renderer, initialized on first use via getRenderer()
        std::unique_ptr<Nebulite::Renderer> renderer;

        // File for error logging, if errorLogStatus is true
        std::unique_ptr<std::ofstream> errorFile;

        // Original cerr buffer, used for restoring after redirecting
        std::streambuf*& originalCerrBuf;
    };
}