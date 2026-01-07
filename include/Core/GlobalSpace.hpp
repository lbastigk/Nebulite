/**
 * @file GlobalSpace.hpp
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
#include <string>
#include <vector>

// Nebulite
#include "Core/Renderer.hpp"
#include "Constants/ErrorTypes.hpp"
#include "Data/Document/DocumentCache.hpp"
#include "Data/TaskQueue.hpp"
#include "Interaction/Invoke.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

#include "DomainModule/GlobalSpace/Floating/RNG.hpp"

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
    // Provide scopes for DomainModules and RulesetModules, depending on their type

    // GlobalSpace DomainModules root is at "", then we add their own prefix
    [[nodiscard]] JsonScope& shareScope(Interaction::Execution::DomainModule<GlobalSpace> const& dm) const {
        return domainScope.shareScope(dm.getDoc().getScopePrefix());
    }

    // Provide a custom scope for DomainModules from RenderObjects
    // We add a prefix to signal what part these domainModules can access
    [[nodiscard]] JsonScope& shareScope(Interaction::Execution::DomainModule<RenderObject> const& dm) const {
        return domainScope.shareScope("providedScope.domainModule.renderObject." + dm.getDoc().getScopePrefix());
    }

    // Provide a custom scope for DomainModules from JsonScope
    // We add a prefix to signal what part these domainModules can access
    [[nodiscard]] JsonScope& shareScope(Interaction::Execution::DomainModule<JsonScope> const& dm) const {
        return domainScope.shareScope("providedScope.domainModule.jsonScope." + dm.getDoc().getScopePrefix());
    }

    // Provide scope to RulesetModules
    [[nodiscard]] JsonScope& shareScope(Interaction::Rules::RulesetModule const& rm) const {
        (void)rm; // unused, we provide full scope for now
        // TODO: add a getScopePrefix() to RulesetModule later on if needed
        //       e.g. Physics RulesetModule might only need access to physics-related variables.
        //       For this to work properly, we may have to add the ability to share multiple scopes.
        //       -> physics and time for example
        return domainScope.shareScope("");
    }

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
     * @todo Remove this function head and all mentions of it
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
    // Broadcast/Listen

    /**
     * @brief Broadcasts a ruleset to other domains.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
        invoke.broadcast(entry);
    }

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listening domain
     * @param topic The topic to listen for.
     * @param listenerId The unique ID of the listener domain.
     */
    void listen(Interaction::Execution::DomainBase& listener, std::string const& topic, uint32_t const& listenerId) {
        invoke.listen(listener, topic, listenerId);
    }

    //------------------------------------------
    // Getters

    /**
     * @brief Gets a reference to the Renderer instance.
     * @return Reference to the Renderer instance.
     */
    Renderer& getRenderer() { return renderer; }

    /**
     * @brief Gets a pointer to the SDL Renderer instance.
     * @return Pointer to the SDL_Renderer instance.
     */
    [[nodiscard]] SDL_Renderer* getSdlRenderer() const { return renderer.getSdlRenderer(); }

    /**
     * @brief Gets a reference to the global document cache.
     * @return Reference to the DocumentCache instance.
     */
    Data::DocumentCache& getDocCache() { return docCache; }

    //------------------------------------------
    // DomainModule variables

    struct commandLineVariables {
        bool headless = false; // Headless mode (no window)
        bool recover = false; // Enable recoverable error mode
        /*Add more variables as needed*/
    } cmdVars;

    /**
     * @brief Checks if the main loop should continue running.
     * @return True if the main loop should continue, false otherwise.
     */
    [[nodiscard]] bool shouldContinueLoop() const { return continueLoop; }


    //------------------------------------------
    // Task Queue Management

    /**
     * @brief Clears all task queues.
     */
    void clearAllTaskQueues() {
        std::ranges::for_each(tasks | std::views::values, [](auto &tq) {
            if (tq) tq->clear();
        });
    }

    /**
     * @brief Gets a specific task queue by name.
     * @param name The name of the task queue.
     * @return Pointer to the TaskQueue instance, or nullptr if not found.
     */
    std::shared_ptr<Data::TaskQueue> getTaskQueue(std::string const& name) {
        if (auto const it = tasks.find(name); it != tasks.end()) {
            return it->second;
        }
        return nullptr;
    }

    /**
     * @struct StandardTasks
     * @brief Contains standard task queue names used in the GlobalSpace.
     */
    struct StandardTasks {
        inline static constexpr const char* always = "tasks::always";
        inline static constexpr const char* internal = "tasks::internal";
        inline static constexpr const char* script = "tasks::script";
    };

    //------------------------------------------
    // Special Functions

    void rngRollback() const {
        if (floatingDM.rng) {
            floatingDM.rng->rngRollback();
        }
    }

private:
    //------------------------------------------
    // General Variables

    // Check if main loop should continue
    bool continueLoop = true;

    // Global JSON Document
    // Technically better to not have this variable and rely on
    // creating a JsonScope in the Constructor
    // And then using getDoc() to access it,
    // but this is more difficult due to lifetime issues it seems.
    Core::JsonScope globalDoc = Core::JsonScope("GlobalSpace Document");

    // DocumentCache for read-only documents
    Data::DocumentCache docCache;

    // Renderer
    Renderer renderer;

    // Invoke Object for parsing expressions etc.
    Interaction::Invoke invoke;

    //------------------------------------------
    // Structs

    /**
     * @brief Contains task queues for different types of tasks.
     */
    absl::flat_hash_map<std::string,std::shared_ptr<Data::TaskQueue>> tasks; // Custom task queues added at runtime

    /**
     * @brief Contains results of the last task queue resolutions.
     */
    absl::flat_hash_map<std::string, Data::TaskQueueResult> queueResult;

    /**
     * @struct names
     * @brief Contains names used in the global space that are not bound to the global document.
     */
    struct names {
        std::string state; // Name of the state where files are saved (equal to savegame name)
        std::string binary; // Name of the binary, used for parsing arguments
    } names;


    //------------------------------------------
    // Floating DomainModules

    struct FloatingDomainModules {
        std::unique_ptr<DomainModule::GlobalSpace::RNG> rng;
    } floatingDM;

    //------------------------------------------
    // Methods

    /**
     * @brief Called before any command parsing.
     * @details Updates RNGs before parsing commands.
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         an error code otherwise.
     */
    Constants::Error preParse() override;

    /**
     * @brief Updates all inner domains.
     * @return If a critical error occurred, the corresponding error code. None otherwise.
     */
    [[nodiscard]] Constants::Error updateInnerDomains();
};
} // namespace Nebulite::Core
#endif // NEBULITE_CORE_GLOBALSPACE_HPP
