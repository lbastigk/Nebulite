/**
 * @file GlobalSpace.hpp
 * 
 * @brief Contains the Nebulite::Core::GlobalSpace class declaration 
 *        for the Nebulite Engine for core functionality
 *        and structures in Nebulite::Core namespace.
 */

#ifndef NEBULITE_CORE_GLOBALSPACE_HPP
#define NEBULITE_CORE_GLOBALSPACE_HPP

//------------------------------------------
// Includes

// Standard library
#include <deque>

// Nebulite
#include "Core/Renderer.hpp"
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/RNG.hpp"

//------------------------------------------
namespace Nebulite::Core {

/**
 * @struct taskQueueWrapper
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
 *        This structure holds the outcome of processing a task queue, including any errors
 *        encountered during resolution and whether the process was halted due to a critical error.
 */
struct taskQueueResult{
    bool encounteredCriticalResult = false;
    std::vector<Constants::Error> errors;
};

//------------------------------------------
// Global Space object

/**
 * @class Nebulite::Core::GlobalSpace
 * @brief Declares the core types, global objects, and functions for the Nebulite Engine.
 *        Used as a global workspace for functionality such as Rendering, Time, RNGs, etc.
 */
NEBULITE_DOMAIN(GlobalSpace){
public:
    //------------------------------------------
    // Special Member Functions

    // Constructor
    explicit GlobalSpace(std::string const& name = "Unnamed GlobalSpace");

    // Destructor
    ~GlobalSpace() override = default;

    // Prevent copying
    GlobalSpace(GlobalSpace const&) = delete;
    GlobalSpace& operator=(GlobalSpace const&) = delete;

    // Prevent moving
    GlobalSpace(GlobalSpace&&) = delete;
    GlobalSpace& operator=(GlobalSpace&&) = delete;

    //------------------------------------------
    // Functions

    /**
     * @brief Parses command line arguments from the main function and sets corresponding variables.
     * @param argc The number of command line arguments.
     * @param argv The array of command line argument strings.
     */
    void parseCommandLineArguments(int const& argc, char const* argv[]);

    /**
     * @brief Resolves a task queue by parsing each task and executing it.
     * @param tq The task queue to resolve.
     * @param waitCounter A counter for checking if the task execution should wait a certain amount of frames.
     * @return The result of the task queue resolution.
     */
    taskQueueResult resolveTaskQueue(taskQueueWrapper& tq, uint64_t const* waitCounter) const ;

    /**
     * @brief Parses the task queue for execution.
     * 
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     * the last critical error code otherwise.
     */
    Constants::Error parseQueue();

    /**
     * @brief Updates the global space.
     * @return If a critical error occurred, the corresponding error code. None otherwise.
     */
    Constants::Error update() override;

    /**
     * @brief Quits the renderer by setting the quit flag.
     */
    void quitRenderer(){
        renderer.setQuit();
    }

    /**
     * @brief Evaluates a string.
     * @param expr The expression to evaluate.
     * @return The evaluated result as a string.
     */
    std::string eval(std::string const& expr) const {
        return invoke.evaluateStandaloneExpression(expr);
    }

    /**
     * @brief Evaluates a string with context of a RenderObject.
     * @param expr The expression to evaluate.
     * @param context The RenderObject providing context for the evaluation.
     * @return The evaluated result as a string.
     */
    std::string eval(std::string const& expr, RenderObject const* context) const {
        return invoke.evaluateStandaloneExpression(expr, context);
    }

    /**
     * @brief Evaluates a string and returns the result as a double.
     * @param expr The expression to evaluate.
     * @return The evaluated result as a double.
     */
    double evalAsDouble(std::string const& expr) const {
        return invoke.evaluateStandaloneExpressionAsDouble(expr);
    }

    /**
     * @brief Evaluates a string with context of a RenderObject and returns the result as a double.
     * @param expr The expression to evaluate.
     * @param context The RenderObject providing context for the evaluation.
     * @return The evaluated result as a double.
     */
    double evalAsDouble(std::string const& expr, RenderObject const* context) const {
        return invoke.evaluateStandaloneExpressionAsDouble(expr, context);
    }

    /**
     * @brief Evaluates a string and returns the result as a boolean.
     * @param expr The expression to evaluate.
     * @return The evaluated result as a boolean.
     */
    bool evalAsBool(std::string const& expr) const {
        return invoke.evaluateStandaloneExpressionAsBool(expr);
    }

    /**
     * @brief Evaluates a string with context of a RenderObject and returns the result as a boolean.
     * @param expr The expression to evaluate.
     * @param context The RenderObject providing context for the evaluation.
     * @return The evaluated result as a boolean.
     */
    bool evalAsBool(std::string const& expr, RenderObject const* context) const {
        return invoke.evaluateStandaloneExpressionAsBool(expr, context);
    }

    //------------------------------------------
    // Getters

    /**
     * @brief Gets the global queue for function calls.
     */
    std::deque<std::string>* getTaskQueue(){ return &tasks.script.taskQueue; }

    /**
     * @brief Gets a pointer to the Renderer instance.
     * @return Pointer to the Renderer instance.
     */
    Renderer* getRenderer(){ return &renderer; }

    /**
     * @brief Gets a pointer to the SDL Renderer instance.
     * @return Pointer to the SDL_Renderer instance.
     */
    SDL_Renderer* getSdlRenderer() const { return renderer.getSdlRenderer(); }

    /**
     * @brief Gets a pointer to the Invoke instance.
     * @return Pointer to the Invoke instance.
     */
    Interaction::Invoke* getInvoke(){ return &invoke; }

    /**
     * @brief Gets a pointer to the global document cache.
     * @return Pointer to the DocumentCache instance.
     */
    Utility::DocumentCache* getDocCache(){ return &docCache; }

    //------------------------------------------
    // Public Variables

    /**
     * @struct Tasks
     * @brief Contains task queues for different types of tasks.
     */
    struct Tasks{
        taskQueueWrapper script;     // Task queue for script files loaded with "task"
        taskQueueWrapper internal;   // Internal task queue from renderObjects, console, etc.
        taskQueueWrapper always;     // Always-tasks added with the prefix "always "
    } tasks;

    // Wait counter for script tasks
    uint64_t scriptWaitCounter = 0;

    // Error Table for error descriptions
    Constants::ErrorTable errorTable;

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
     * @return True if the main loop should continue, false otherwise.
     */
    bool shouldContinueLoop() const { return continueLoop; }

    enum class UniqueIdType{
        expression = 0,
        jsonKey = 1
    };
    static constexpr size_t UniqueIdTypeSize = 2;

    /**
     * @brief Gets a unique ID based on a hash string.
     *        Threadsafe. Uses a mutex-lock per UniqueIdType.
     * @param hash The hash string to get the unique ID for.
     * @param type Which rolling counter to use for the unique ID, allowing for separate ID spaces.
     * @return The unique ID corresponding to the hash.
     */
    uint64_t getUniqueId(std::string const& hash, UniqueIdType type){
        // Lock and check if hash exists. If not, create new ID
        std::scoped_lock lock(uniqueIdMutex[static_cast<size_t>(type)]);
        if(auto const it = uniqueIdMap[static_cast<size_t>(type)].find(hash); it != uniqueIdMap[static_cast<size_t>(type)].end()){
            return it->second;
        }
        uint64_t const newId = uniqueIdCounter[static_cast<size_t>(type)]++;
        uniqueIdMap[static_cast<size_t>(type)][hash] = newId;
        return newId;
    }

private:
    //------------------------------------------
    // General Variables

    // Check if main loop should continue
    bool continueLoop = true;

    // Global JSON Document
    Utility::JSON document;

    // DocumentCache for read-only documents
    Utility::DocumentCache docCache;

    // Renderer
    Renderer renderer;

    // Invoke Object for parsing expressions etc.
    Interaction::Invoke invoke;

    // Unique ID map
    uint64_t uniqueIdCounter[UniqueIdTypeSize] = {0, 0};
    absl::flat_hash_map<std::string, uint64_t> uniqueIdMap[UniqueIdTypeSize];
    std::mutex uniqueIdMutex[UniqueIdTypeSize];

    //------------------------------------------
    // Structs

    /**
     * @struct QueueResult
     * @brief Holds the results of resolving different task queues.
     */
    struct QueueResult {
        taskQueueResult script;       // Result of script-tasks
        taskQueueResult internal;     // Result of internal-tasks
        taskQueueResult always;       // Result of always-tasks
    } queueResult;

    /**
     * @struct names
     * @brief Contains names used in the global space that are not bound to the global document.
     */
    struct names{
        std::string state;      // Name of the state where files are saved (equal to savegame name)
        std::string binary;     // Name of the binary, used for parsing arguments
    }names;

    /**
     * @struct RngVars
     * @brief Contains RNG instances used in the global space.
     */
    struct RngVars{
        using rngSize_t = uint16_t;            // Modify this to change the size of the RNGs
        Utility::RNG<rngSize_t> A;   // RNG with key random.A
        Utility::RNG<rngSize_t> B;   // RNG with key random.B
        Utility::RNG<rngSize_t> C;   // RNG with key random.C
        Utility::RNG<rngSize_t> D;   // RNG with key random.D
    } rng;

    //------------------------------------------
    // Methods

    /**
     * @brief Sets the pre-parse function for the domain,
     *        which is called before parsing any command.
     *        It is used here to properly handle RNG
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         an error code otherwise.
     */
    Constants::Error preParse() override;

    /**
     * @brief Updates all RNGs
     */
    void updateRNGs();

    /**
     * @brief Updates all inner domains.
     * @return If a critical error occurred, the corresponding error code. None otherwise.
     */
    Constants::Error updateInnerDomains() const;
};
}   // namespace Nebulite::Core
#endif // NEBULITE_CORE_GLOBALSPACE_HPP