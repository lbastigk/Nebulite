
/**
 * @file GlobalSpace.hpp
 * 
 * @brief Contains the Nebulite::Core::GlobalSpace class declaration 
 * for the Nebulite Engine for core functionality
 * and structures in Nebulite::Core namespace.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include <deque>

// Nebulite
#include "Core/Renderer.hpp"
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
namespace Nebulite {
namespace Core {
//------------------------------------------
// General Types used

/**
 * @struct Nebulite::Core::taskQueueWrapper
 * @brief Represents a queue of tasks to be processed by the engine, including metadata.
 */
struct taskQueueWrapper {
    std::deque<std::string> taskQueue;  // List of tasks
    bool clearAfterResolving = true;    // Whether to clear the task list after resolving
    /**
     * @note Add more metadata as needed, for resolveTaskQueue() to use
     */
};

/**
 * @brief Represents the result of resolving a task queue.
 *
 * This structure holds the outcome of processing a task queue, including any errors
 * encountered during resolution and whether the process was halted due to a critical error.
 */
struct taskQueueResult{
    bool stoppedAtCriticalResult = false;
    std::vector<Nebulite::Constants::Error> errors;
};

//------------------------------------------
// Global Space object

/**
 * @class Nebulite::Core::GlobalSpace
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
 *   - renderer: Pointer to the main rendering engine, lazily initialized.
 * 
 *   - error logging: Facilities for redirecting and storing error output.
 * 
 *   - stateName, binName: Strings for tracking the current engine state and binary name.
 *
 * See main.cpp and other engine modules for usage examples and integration details.
 * 
 * @todo internal variables like "--headless" as bool instead of string should be enough
 */
NEBULITE_DOMAIN(GlobalSpace) {
public:
    //------------------------------------------
    // Special Member Functions

    // Constructor
    GlobalSpace(const std::string binName);

    // Destructor
    ~GlobalSpace() = default;

    // Prevent copying
    GlobalSpace(const GlobalSpace&) = delete;
    GlobalSpace& operator=(const GlobalSpace&) = delete;

    // Prevent moving
    GlobalSpace(GlobalSpace&&) = delete;
    GlobalSpace& operator=(GlobalSpace&&) = delete;

    //------------------------------------------
    // Functions

    /**
     * @brief Parses command line arguments and sets corresponding variables.
     * 
     * This function processes the command line arguments passed to the program,
     * setting internal variables based on recognized flags and options.
     * 
     * @param argc The number of command line arguments.
     * @param argv The array of command line argument strings.
     */
    void parseCommandLineArguments(int argc, char* argv[]);

    /**
     * @brief Allows for access to the Renderer instance
     *
     * This function retrieves the renderer instance, creating it if it doesn't already exist.
     *
     * @return A pointer to the Renderer instance.
     */
    Nebulite::Core::Renderer* getRenderer();

    /**
     * @brief Allows for access to the SDL_Renderer instance
     *
     * This function retrieves the SDL_Renderer instance, creating it if it doesn't already exist.
     *
     * @return A pointer to the SDL_Renderer instance.
     */
    SDL_Renderer* getSDLRenderer();

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
    Nebulite::Core::taskQueueResult resolveTaskQueue(Nebulite::Core::taskQueueWrapper& tq, uint64_t* waitCounter);

    /**
     * @brief Parses the task queue for execution.
     * 
     * @return Errorcode `Nebulite::Constants::ErrorTable::NONE()` if there was no critical stop,
     * the last critical error code otherwise.
     */
    Nebulite::Constants::Error parseQueue();

    /**
     * @brief Updates the global space.
     */
    void update();

    //------------------------------------------
    // Public Variables

    /**
     * @struct Tasks
     * @brief Contains task queues for different types of tasks.
     */
    struct Tasks{
        Nebulite::Core::taskQueueWrapper script;     // Task queue for script files loaded with "task"
        Nebulite::Core::taskQueueWrapper internal;   // Internal task queue from renerObjects, console, etc.
        Nebulite::Core::taskQueueWrapper always;     // Always-tasks added with the prefix "always "
    } tasks;

    // Wait counter for script tasks
    uint64_t scriptWaitCounter = 0;

    // Error Table for error descriptions
    Nebulite::Constants::ErrorTable errorTable;

    // Invoke Object for parsing expressions etc.
    std::unique_ptr<Nebulite::Interaction::Invoke> invoke;

    //------------------------------------------
    // DomainModule variables
    struct commandLineVariables{
        std::string headless = "false"; // Headless mode (no window)
        std::string recover = "false";  // Enable recoverable error mode
        /*Add more variables as needed*/
    };
    commandLineVariables cmdVars;

private:
    //------------------------------------------
    // General Variables

    // Global JSON Document
    Nebulite::Utility::JSON global;

    //------------------------------------------
    // Structs

    // For resolving tasks
    struct QueueResult {
        Nebulite::Core::taskQueueResult script;       // Result of script-tasks
        Nebulite::Core::taskQueueResult internal;     // Result of internal-tasks
        Nebulite::Core::taskQueueResult always;       // Result of always-tasks
    }queueResult;

    /**
     * @brief Contains names used in the global space that are not bound to the global document.
     */
    struct names{
        std::string state;      // Name of the state where files are saved (equal to savegame name)
        std::string binary;     // Name of the binary, used for parsing arguments
    }names;

    //------------------------------------------
    // Methods

    /**
     * @brief Sets the pre-parse function for the domain.
     * 
     * This function binds a pre-parse function to the domain's function tree,
     * which is called before parsing any command. It is used to properly handle RNG
     */
    Nebulite::Constants::Error preParse() override;

    //------------------------------------------
    // Objects

    /**
     * @brief Pointer to the renderer instance.
     * 
     * This pointer is used to access the renderer for drawing operations.
     * It is initialized on first use via getRenderer(), all access is through this function.
     * 
     * Due to the way the renderer is initialized, the object is marked as private.
     * Under no circumstances should processes try to access the renderer directly!
     */
    std::unique_ptr<Nebulite::Core::Renderer> renderer;

    /**
     * @brief Flag indicating whether the renderer has been initialized.
     */
    bool rendererInitialized = false;

    /**
     * @brief Hasher for generating RNG values from last parsed command.
     */
    std::hash<std::string> rng_hasher;
};
}   // namespace Core
}   // namespace Nebulite