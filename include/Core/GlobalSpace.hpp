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
#include "Data/TaskQueue.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Invoke.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/RNG.hpp"

//------------------------------------------
namespace Nebulite::Core {

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
    Data::TaskQueueResult resolveTaskQueue(Data::TaskQueue& tq, uint64_t const* waitCounter) const;

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

    //------------------------------------------
    // Getters

    /**
     * @brief Gets the global queue for function calls.
     * @return Pointer to the task queue deque for script tasks.
     * @todo Consider more versatile task queue management in the future.
     *       Returning the correct deque based on task type, etc.
     *       With fallback to a default queue if type is unknown.
     */
    std::deque<std::string>* getTaskQueue() { return &tasks.script.tasks; }

    /**
     * @brief Gets a pointer to the Renderer instance.
     * @return Pointer to the Renderer instance.
     */
    Renderer* getRenderer() { return &renderer; }

    /**
     * @brief Gets a pointer to the SDL Renderer instance.
     * @return Pointer to the SDL_Renderer instance.
     */
    [[nodiscard]] SDL_Renderer* getSdlRenderer() const { return renderer.getSdlRenderer(); }

    /**
     * @brief Gets a pointer to the Invoke instance.
     * @return Pointer to the Invoke instance.
     * @todo Exposing Invoke directly may not be necessary, should be enough to just have a broadcast/listen system in GlobalSpace?
     */
    Interaction::Invoke& getInvoke() { return invoke; }

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
        Data::TaskQueue script; // Task queue for script files loaded with "task"
        Data::TaskQueue internal; // Internal task queue from renderObjects, console, etc.
        Data::TaskQueue always; // Always-tasks added with the prefix "always "
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
    [[nodiscard]] bool shouldContinueLoop() const { return continueLoop; }

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

    //------------------------------------------
    // Structs

    /**
     * @struct QueueResult
     * @brief Holds the results of resolving different task queues.
     */
    struct QueueResult {
        Data::TaskQueueResult script; // Result of script-tasks
        Data::TaskQueueResult internal; // Result of internal-tasks
        Data::TaskQueueResult always; // Result of always-tasks
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
    [[nodiscard]] Constants::Error updateInnerDomains() const;
};
} // namespace Nebulite::Core
#endif // NEBULITE_CORE_GLOBALSPACE_HPP