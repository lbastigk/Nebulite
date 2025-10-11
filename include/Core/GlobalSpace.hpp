
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
#include "Utility/RNG.hpp"

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
     * @brief Gets the global queue for function calls.
     */
    std::deque<std::string>* getTaskQueue(){return &tasks.script.taskQueue;};

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
     * 
     * @return If a critical error occurred, the corresponding error code. None otherwise.
     */
    Nebulite::Constants::Error update();

    //------------------------------------------
    // Public Variables

    /**
     * @struct Tasks
     * @brief Contains task queues for different types of tasks.
     */
    struct Tasks{
        Nebulite::Core::taskQueueWrapper script;     // Task queue for script files loaded with "task"
        Nebulite::Core::taskQueueWrapper internal;   // Internal task queue from renderObjects, console, etc.
        Nebulite::Core::taskQueueWrapper always;     // Always-tasks added with the prefix "always "
    } tasks;

    // Wait counter for script tasks
    uint64_t scriptWaitCounter = 0;

    // Error Table for error descriptions
    Nebulite::Constants::ErrorTable errorTable;

    //------------------------------------------
    // DomainModule variables
    struct commandLineVariables{
        std::string headless = "false"; // Headless mode (no window)
        std::string recover = "false";  // Enable recoverable error mode
        /*Add more variables as needed*/
    };
    commandLineVariables cmdVars;

    /**
     * @brief Rolls back all RNGs to their previous state.
     */
    void rngRollback(){
        rng.A.rollback();
        rng.B.rollback();
        rng.C.rollback();
        rng.D.rollback();
    }

    /**
     * @brief Checks if the main loop should continue running.
     * 
     * @return True if the main loop should continue, false otherwise.
     */
    bool shouldContinueLoop() const { 
        return continueLoop; 
    };

private:
    //------------------------------------------
    // General Variables

    // Check if main loop should continue
    bool continueLoop = true;

    // Global JSON Document
    Nebulite::Utility::JSON global;

    // Renderer
    Nebulite::Core::Renderer renderer;

    // Invoke Object for parsing expressions etc.
    Nebulite::Interaction::Invoke invoke;

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

    /**
     * @brief Contains RNG instances used in the global space.
     */
    struct RNGvars{
        using rng_size_t = uint16_t;            // Modify this to change the size of the RNGs
        Nebulite::Utility::RNG<rng_size_t> A;   // RNG with key random.A
        Nebulite::Utility::RNG<rng_size_t> B;   // RNG with key random.B
        Nebulite::Utility::RNG<rng_size_t> C;   // RNG with key random.C
        Nebulite::Utility::RNG<rng_size_t> D;   // RNG with key random.D
    } rng;

    //------------------------------------------
    // Methods

    /**
     * @brief Sets the pre-parse function for the domain.
     * 
     * This function binds a pre-parse function to the domain's function tree,
     * which is called before parsing any command. It is used to properly handle RNG
     */
    Nebulite::Constants::Error preParse() override;

    /**
     * @brief Updates all RNGs with a new seed.
     * 
     * @param seed The normalized seed string used to update the RNGs.
     * Make sure the seed contains no user-specific information like absolute paths!
     * Otherwise the RNG is not consistent across different users.
     */
    void updateRNGs(std::string seed);

    /**
     * @brief Updates all inner domains.
     * 
     * @return If a critical error occurred, the corresponding error code. None otherwise.
     */
    Nebulite::Constants::Error updateInnerDomains();
};
}   // namespace Core
}   // namespace Nebulite