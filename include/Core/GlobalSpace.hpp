
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
    bool encounteredCriticalResult = false;
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
    explicit GlobalSpace(const std::string& binName);

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
    void parseCommandLineArguments(const int argc, const char* argv[]);

    /**
     * @brief Resolves a task queue by parsing each task and executing it.
     * 
     * @param tq The task queue to resolve.
     * @param waitCounter A counter for checking if the task execution should wait a certain amount of frames.
     * @return The result of the task queue resolution.
     */
    Nebulite::Core::taskQueueResult resolveTaskQueue(Nebulite::Core::taskQueueWrapper& tq, const uint64_t* waitCounter);

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
    Nebulite::Constants::Error update() override;

    /**
     * @brief Quits the renderer by setting the quit flag.
     */
    void quitRenderer(){
        renderer.setQuit();
    }

    /**
     * @brief Evaluates a string.
     */
    std::string eval(const std::string& expr){
        return invoke.evaluateStandaloneExpression(expr);
    }

    /**
     * @brief Evaluates a string with context of a RenderObject.
     */
    std::string eval(const std::string& expr, Nebulite::Core::RenderObject* context){
        return invoke.evaluateStandaloneExpression(expr, context);
    }

    //------------------------------------------
    // Getters

    /**
     * @brief Gets the global queue for function calls.
     */
    std::deque<std::string>* getTaskQueue(){return &tasks.script.taskQueue;};

    /**
     * @brief Gets a pointer to the Renderer instance.
     */
    Nebulite::Core::Renderer* getRenderer(){return &renderer;};

    /**
     * @brief Gets a pointer to the SDL Renderer instance.
     */
    SDL_Renderer* getSdlRenderer(){return renderer.getSdlRenderer();};

    /**
     * @brief Gets a pointer to the Invoke instance.
     */
    Nebulite::Interaction::Invoke* getInvoke(){return &invoke;};

    /**
     * @brief Gets a pointer to the global document cache.
     */
    Nebulite::Utility::DocumentCache* getDocCache(){return &docCache;}

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
        bool headless = false; // Headless mode (no window)
        bool recover = false;  // Enable recoverable error mode
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

    enum class UniqueIdType{
        EXPRESSION = 0,
        JSONKEY = 1
    };
    static constexpr size_t UniqueIdTypeSize = 2;

    /**
     * @brief Gets a unique ID based on a hash string.
     * 
     * Threadsafe. Uses a mutex-lock per UniqueIdType.
     * 
     * @param hash The hash string to get the unique ID for.
     * @param type Which rolling counter to use for the unique ID, allowing for separate ID spaces.
     * @return The unique ID corresponding to the hash.
     */
    uint64_t getUniqueId(std::string hash, UniqueIdType type){
        // Lock and check if hash exists
        std::lock_guard<std::mutex> lock(uniqueIdMutex[static_cast<size_t>(type)]);

        auto it = uniqueIdMap[static_cast<size_t>(type)].find(hash);
        if(it != uniqueIdMap[static_cast<size_t>(type)].end()){
            return it->second;
        } else {
            uint64_t newId = uniqueIdCounter[static_cast<size_t>(type)]++;
            uniqueIdMap[static_cast<size_t>(type)][hash] = newId;
            return newId;
        }
    }

    /**
     * @brief cout/cerr capture for logging output
     */
    Nebulite::Utility::Capture capture;

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

    // DocumentCache for read-only documents
    Nebulite::Utility::DocumentCache docCache;

    // Unique ID map
    uint64_t uniqueIdCounter[UniqueIdTypeSize] = {0, 0};
    absl::flat_hash_map<std::string, uint64_t> uniqueIdMap[UniqueIdTypeSize];
    std::mutex uniqueIdMutex[UniqueIdTypeSize];

    //------------------------------------------
    // Structs

    // For resolving tasks
    struct QueueResult
    {
        Nebulite::Core::taskQueueResult script;       // Result of script-tasks
        Nebulite::Core::taskQueueResult internal;     // Result of internal-tasks
        Nebulite::Core::taskQueueResult always;       // Result of always-tasks
    } queueResult;

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
     * @brief Updates all RNGs
     */
    void updateRNGs();

    /**
     * @brief Updates all inner domains.
     * 
     * @return If a critical error occurred, the corresponding error code. None otherwise.
     */
    Nebulite::Constants::Error updateInnerDomains();
};
}   // namespace Core
}   // namespace Nebulite