/**
 * @file GlobalSpace.hpp
 * @brief Contains the Nebulite::Core::GlobalSpace class declaration 
 *        for the Nebulite Engine for core functionality
 *        and structures in Nebulite::Core namespace.
 *        Manages rendering, task queues, RNGs, and similar global features.
 */

#ifndef CORE_GLOBALSPACE_HPP
#define CORE_GLOBALSPACE_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Core/Renderer.hpp"
#include "Data/Document/DocumentCache.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Invoke.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Rules {
class Ruleset;
struct Listener;
} // namespace Nebulite::Interaction::Rules

namespace Nebulite::Module::Domain::GlobalSpace {
class RNG;
} // namespace Nebulite::Module::Domain::GlobalSpace

//------------------------------------------
namespace Nebulite::Core {
/**
 * @class Nebulite::Core::GlobalSpace
 * @brief Declares the core types, global objects, and functions for the Nebulite Engine.
 *        Used as a global workspace for functionality such as Rendering, Time, RNGs, etc.
 * @details In order to avoid infinite recursion, GlobalSpace itself does not own its Document.
 *          Instead, it accesses the global document via Global::
 *          This prevents issues where DomainModules might try to access GlobalSpace during their construction
 *          in order to access the global document, leading to infinite recursion.
 *          By separating the GlobalSpace and the Global Document, we ensure that the construction of both is independent.
 *          This allows DomainModules to safely access the global document without causing recursion problems.
 */
class GlobalSpace final : public Interaction::Execution::Domain {
public:
    //------------------------------------------
    // Special Member Functions

    // Constructor itself notices if multiple instances are created
    // and throws an error in that case

    explicit GlobalSpace(std::string const& name);

    /**
     * @brief In order to avoid infinite recursion, we initialize GlobalSpace after construction.
     * @details The issue is that some DomainModules may need to access GlobalSpace during their construction,
     *          leading to infinite recursion if GlobalSpace is not fully constructed yet.
     *          By separating initialization from construction, we ensure that GlobalSpace is fully constructed
     *          before any DomainModule attempts to access it.
     */
    void initialize();

    ~GlobalSpace() override ;

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
     * @brief Parses the task queue for execution.
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         the last critical error code otherwise.
     */
    [[nodiscard]] Constants::Event parseQueue();

    /**
     * @brief Updates the global space.
     * @return If a critical error occurred, the corresponding error code. None otherwise.
     */
    [[nodiscard]] Constants::Event update() override;

    /**
     * @brief Quits the renderer by setting the quit flag.
     */
    void quitRenderer();

    //------------------------------------------
    // Broadcast/Listen

    /**
     * @brief Broadcasts a ruleset to other domains.
     * @param entry The ruleset to broadcast. Make sure the topic is not empty, as this implies a local-only entry!
     */
    void broadcast(std::shared_ptr<Interaction::Rules::Ruleset> const& entry) const ;

    /**
     * @brief Listens for rulesets on a specific topic.
     * @param listener The listener to add.
     */
    void listen(std::shared_ptr<Interaction::Rules::Listener> const& listener);

    //------------------------------------------
    // Getters

    /**
     * @brief Gets a reference to the Renderer instance.
     * @return Reference to the Renderer instance.
     */
    Renderer& getRenderer();

    /**
     * @brief Gets a pointer to the SDL Renderer instance.
     * @return Pointer to the SDL_Renderer instance.
     */
    [[nodiscard]] SDL_Renderer* getSdlRenderer() const ;

    /**
     * @brief Gets a reference to the global document cache.
     * @return Reference to the DocumentCache instance.
     */
    Data::DocumentCache& getDocCache();

    //------------------------------------------
    // Id-index mapping

    /**
    * @brief Gets the RenderObject ID from its index in the rendering pipeline.
    * @param index The index of the RenderObject in the rendering pipeline.
    * @return An optional containing the ID of the RenderObject if found, or std::nullopt if no object is associated with the given index.
    */
    std::optional<size_t> getIdFromIndex(size_t const& index) const ;

    /**
     * @brief Gets the RenderObject index in the rendering pipeline from its ID.
     * @param searchId The ID of the RenderObject to search for.
     * @return An optional containing the index of the RenderObject in the rendering pipeline if found, or std::nullopt if no object is associated with the given ID.
     */
    std::optional<size_t> getIndexFromId(size_t const& searchId) const ;

    //------------------------------------------
    // DomainModule variables

    struct commandLineVariables {
        bool headless = false; // Headless mode (no window)
        bool recover = false; // Enable recoverable error mode
        /*Add more variables as needed*/
    } cmdVars;

    //------------------------------------------
    // Status

    /**
     * @brief Checks if the main loop should continue running.
     * @return True if the main loop should continue, false otherwise.
     */
    [[nodiscard]] bool shouldContinueLoop() const ;

    [[nodiscard]] bool criticalErrorOccurred() const ;

    //------------------------------------------
    // Event Management

    void notifyEvent(Constants::Event const& event);

    //------------------------------------------
    // Special Functions

    void rngRollback() const ;

private:
    //------------------------------------------
    // General Variables

    // Check if main loop should continue
    bool continueLoop = true;

    // Flag to indicate if a critical error has occurred
    bool errorOccurred = false;

    // DocumentCache for read-only documents
    Data::DocumentCache docCache;

    // Renderer
    Renderer renderer;

    // Invoke Object for managing broadcasted rulesets
    Interaction::Invoke invoke;

    //------------------------------------------
    // Structs

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
        std::unique_ptr<Module::Domain::GlobalSpace::RNG> rng;
    } floatingDM;

    //------------------------------------------
    // Methods

    /**
     * @brief Called before any command parsing.
     * @details Updates RNGs before parsing commands.
     * @return Error code `Constants::ErrorTable::NONE()` if there was no critical stop,
     *         an error code otherwise.
     */
    [[nodiscard]] Constants::Event preParse() override ;

    /**
     * @brief Updates all inner domains.
     * @return If a critical error occurred, the corresponding error code. None otherwise.
     */
    [[nodiscard]] Constants::Event updateInnerDomains();
};
} // namespace Nebulite::Core
#endif // CORE_GLOBALSPACE_HPP
