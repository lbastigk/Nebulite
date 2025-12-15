/**
 * @file GlobalSpace.hpp
 * 
 * @brief Contains the Nebulite::Core::GlobalSpace class declaration 
 *        for the Nebulite Engine for core functionality
 *        and structures in Nebulite::Core namespace.
 *        Manages rendering, task queues, RNGs, and similar global features.
 */

#ifndef NEBULITE_CORE_GLOBALSPACE_HPP
#define NEBULITE_CORE_GLOBALSPACE_HPP

//------------------------------------------
// Includes

// Standard library
#include <deque>
#include <string>
#include <vector>
#include <mutex>
#include <cstdint>
#include <cstddef>

// Nebulite
#include "Core/Renderer.hpp"
#include "Constants/ErrorTypes.hpp"
#include "Data/DocumentCache.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/RNG.hpp"

//------------------------------------------
namespace Nebulite::Core {

/**
 * @struct taskQueueWrapper
 * @brief Represents a queue of tasks to be processed by the engine, including metadata.
 */
struct taskQueueWrapper {
    std::deque<std::string> taskQueue; // List of tasks
    bool clearAfterResolving = true; // Whether to clear the task list after resolving
    /**
     * @note Add more metadata as needed, for resolveTaskQueue() to use
     *       in case new task types are added in the future.
     *       Perhaps even a hashmap of string to variant around this wrapper for
     *       maximum flexibility.
     *       map string -> taskQueueWrapper{taskQueue, <metadata>}
     *       This way, each task could be sorted into different queues based on type,
     *       so we can simply call <task> for normal tasks,
     *       and specify "on-queue <type> <task>" for specific task types
     *       that we wish to execute in a different manner.
     *       This could allow us to auto-sort tasks into e.g. always-tasks etc.
     *       or even manage tasks with calls such as "modify-task <identifier> <modification>"
     * @todo Implement waitCounter into each taskQueueWrapper, so each queue can have its own wait counter
     *       Then, have function such as wait, task, etc. modify a specify taskQueue.
     *       calls with "on-queue", e.g. "on-queue <always/wait/etc.> <args>" can modify specific queues.
     * @todo Add mutex for thread-safe push/pop of tasks
     * @todo Add its own resolve function, with param for domain.
     * @todo Add own wait counter, being lowered on each frame update.
     */
};

/**
 * @brief Represents the result of resolving a task queue.
 *        This structure holds the outcome of processing a task queue, including any errors
 *        encountered during resolution and whether the process was halted due to a critical error.
 */
struct taskQueueResult {
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
NEBULITE_DOMAIN(GlobalSpace) {
public:
    //------------------------------------------
    // Special Member Functions

    // Constructor itself notices if multiple instances are created
    // and throws an error in that case

    explicit GlobalSpace(std::string const& name = "Unnamed GlobalSpace");

    ~GlobalSpace() override = default;

    // Globalspace is wrapped in a singleton pattern
    // we disallow copying and moving

    // Prevent copying
    GlobalSpace(GlobalSpace const&) = delete;
    GlobalSpace& operator=(GlobalSpace const&) = delete;

    // Prevent moving
    GlobalSpace(GlobalSpace&&) = delete;
    GlobalSpace& operator=(GlobalSpace&&) = delete;

    //------------------------------------------
    // Functions

    /**
     * @brief Parses command line arguments from the main function.
     *        - sets command line variables
     *        - adds given tasks to the script task queue
     * @param argc The number of command line arguments.
     * @param argv The array of command line argument strings.
     */
    void parseCommandLineArguments(int const& argc, char const** argv);

    /**
     * @brief Resolves a task queue by parsing each task and executing it.
     * @param tq The task queue to resolve.
     * @param waitCounter A counter for checking if the task execution should wait a certain amount of frames.
     * @return The result of the task queue resolution.
     */
    taskQueueResult resolveTaskQueue(taskQueueWrapper& tq, uint64_t const* waitCounter) const;

    /**
     * @brief Parses the task queue for execution.
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         the last critical error code otherwise.
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
    void quitRenderer() {
        renderer.setQuit();
    }

    /**
     * @brief Evaluates a string.
     *        Example: "$i(2 + 2)" -> "4"
     *        Allowed context:
     *        - global (GlobalSpace)
     *        - resources (Read only documents)
     * @param expr The expression to evaluate.
     * @return The evaluated result as a string.
     */
    std::string eval(std::string const& expr) const {
        return invoke.evaluateStandaloneExpression(expr);
    }

    /**
     * @brief Evaluates a string with a RenderObject as context "self" and "other".
     *        Example: "My Value is: {other.positionX}" -> "My Value is: 100"
     *        Allowed context:
     *        - self (RenderObject)
     *        - other (RenderObject)
     *        - global (GlobalSpace)
     *        - resources (Read only documents)
     * @param expr The expression to evaluate.
     * @param context The RenderObject providing context for the evaluation.
     * @return The evaluated result as a string.
     */
    std::string eval(std::string const& expr, RenderObject const* context) const {
        return invoke.evaluateStandaloneExpression(expr, context);
    }

    /**
     * @brief Evaluates a string and returns the result as a double.
     *        Example: "$(2 + 2)" -> 4.0
     *        Allowed context:
     *        - global (GlobalSpace)
     *        - resources (Read only documents)
     * @param expr The expression to evaluate.
     * @return The evaluated result as a double.
     */
    double evalAsDouble(std::string const& expr) const {
        return invoke.evaluateStandaloneExpressionAsDouble(expr);
    }

    /**
     * @brief Evaluates a string with context of a RenderObject "self" and "other"
     *        and returns the result as a double.
     *        Example: "$({global.time.dt} * {self.physics.vX})" -> 1.168
     *        Allowed context:
     *        - self (RenderObject)
     *        - other (RenderObject)
     *        - global (GlobalSpace)
     *        - resources (Read only documents)
     * @param expr The expression to evaluate.
     * @param context The RenderObject providing context for the evaluation.
     * @return The evaluated result as a double.
     */
    double evalAsDouble(std::string const& expr, RenderObject const* context) const {
        return invoke.evaluateStandaloneExpressionAsDouble(expr, context);
    }

    /**
     * @brief Evaluates a string and returns the result as a boolean.
     *        Allowed context:
     *        - global (GlobalSpace)
     *        - resources (Read only documents)
     * @param expr The expression to evaluate.
     * @return The evaluated result as a boolean.
     */
    bool evalAsBool(std::string const& expr) const {
        return invoke.evaluateStandaloneExpressionAsBool(expr);
    }

    /**
     * @brief Evaluates a string with context of a RenderObject and returns the result as a boolean.
     *        Allowed context:
     *        - self (RenderObject)
     *        - other (RenderObject)
     *        - global (GlobalSpace)
     *        - resources (Read only documents)
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
     * @return Pointer to the task queue deque for script tasks.
     * @todo Consider more versatile task queue management in the future.
     *       Returning the correct deque based on task type, etc.
     *       With fallback to a default queue if type is unknown.
     */
    std::deque<std::string>* getTaskQueue() { return &tasks.script.taskQueue; }

    /**
     * @brief Gets a pointer to the Renderer instance.
     * @return Pointer to the Renderer instance.
     */
    Renderer* getRenderer() { return &renderer; }

    /**
     * @brief Gets a pointer to the SDL Renderer instance.
     * @return Pointer to the SDL_Renderer instance.
     */
    SDL_Renderer* getSdlRenderer() const { return renderer.getSdlRenderer(); }

    /**
     * @brief Gets a pointer to the Invoke instance.
     * @return Pointer to the Invoke instance.
     */
    Interaction::Invoke* getInvoke() { return &invoke; }

    /**
     * @brief Gets a pointer to the global document cache.
     * @return Pointer to the DocumentCache instance.
     */
    Data::DocumentCache* getDocCache() { return &docCache; }

    //------------------------------------------
    // Public Variables

    /**
     * @struct Tasks
     * @brief Contains task queues for different types of tasks.
     * @todo Use a hashmap instead, with a default task queue for unknown types.
     *       (should be internal task queue)
     */
    struct Tasks {
        taskQueueWrapper script; // Task queue for script files loaded with "task"
        taskQueueWrapper internal; // Internal task queue from renderObjects, console, etc.
        taskQueueWrapper always; // Always-tasks added with the prefix "always "
    } tasks;

    // Wait counter for script tasks
    uint64_t scriptWaitCounter = 0;

    // Error Table for error descriptions
    Constants::ErrorTable errorTable;

    //------------------------------------------
    // DomainModule variables

    struct commandLineVariables {
        bool headless = false; // Headless mode (no window)
        bool recover = false; // Enable recoverable error mode
        /*Add more variables as needed*/
    } cmdVars;

    /**
     * @brief Rolls back all RNGs to their previous state.
     *        Can be called by any domainModule function
     *        if you don't want this functioncall to modify RNG state.
     *        Example: calling a script should not modify RNG, so that we can
     *                 always load scripts for TAS without RNG state changes.
     */
    void rngRollback() {
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

    /**
     * @enum UniqueIdType
     * @brief Types of unique ID spaces.
     *        We allow for multiple unique ID spaces so that different systems
     *        are all in their own container.
     */
    enum class UniqueIdType {
        expression, // Each expression gets a unique ID
        jsonKey, // Each JSON key gets a unique ID
        NONE // Keep this as last entry
    };

    /**
     * @brief Amount of different UniqueIdTypes.
     */
    static constexpr size_t UniqueIdTypeSize = static_cast<size_t>(UniqueIdType::NONE) + 1;

    /**
     * @brief Gets a unique ID based on a hash string.
     *        Threadsafe. Uses a mutex-lock per UniqueIdType.
     * @param hash The hash string to get the unique ID for.
     * @param type Which rolling counter to use for the unique ID, allowing for separate ID spaces.
     * @return The unique ID corresponding to the hash.
     */
    uint64_t getUniqueId(std::string const& hash, UniqueIdType type) {
        std::scoped_lock lock(uniqueIdMutex[static_cast<size_t>(type)]);
        if (auto const it = uniqueIdMap[static_cast<size_t>(type)].find(hash); it != uniqueIdMap[static_cast<size_t>(type)].end()) {
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
    Data::JSON document;

    // DocumentCache for read-only documents
    Data::DocumentCache docCache;

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
        taskQueueResult script; // Result of script-tasks
        taskQueueResult internal; // Result of internal-tasks
        taskQueueResult always; // Result of always-tasks
    } queueResult;

    /**
     * @struct names
     * @brief Contains names used in the global space that are not bound to the global document.
     */
    struct names {
        std::string state; // Name of the state where files are saved (equal to savegame name)
        std::string binary; // Name of the binary, used for parsing arguments
    } names;

    /**
     * @struct RngVars
     * @brief Contains RNG instances used in the global space.
     */
    struct RngVars {
        using rngSize_t = uint16_t; // Modify this to change the size of the RNGs
        Utility::RNG<rngSize_t> A; // RNG with key random.A
        Utility::RNG<rngSize_t> B; // RNG with key random.B
        Utility::RNG<rngSize_t> C; // RNG with key random.C
        Utility::RNG<rngSize_t> D; // RNG with key random.D
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
} // namespace Nebulite::Core
#endif // NEBULITE_CORE_GLOBALSPACE_HPP