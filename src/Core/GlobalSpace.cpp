//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "DomainModule/Initializer.hpp"

//------------------------------------------
namespace Nebulite::Core {

GlobalSpace::GlobalSpace(std::string const& name)
    : Domain("Nebulite", *this, Nebulite::globalDoc().shareScope(*this, "")), // Domain with reference to GlobalSpace and its full scope
      renderer(globalDoc().shareScope(*this, "renderer"), &cmdVars.headless) // Share only the renderer portion of the global document
{
    //------------------------------------------
    // Initialize floating DomainModules

    floatingDM.rng = NEBULITE_FLOATING_DOMAINMODULE(Nebulite::DomainModule::GlobalSpace::RNG,"RNG", domainScope, "random");

    //------------------------------------------
    // There should only be one GlobalSpace
    static bool globalSpaceExists = false;
    if (globalSpaceExists) {
        throw std::runtime_error("GlobalSpace instance already exists! Only one instance is allowed.");
    }
    globalSpaceExists = true;

    //------------------------------------------
    // Setup tasks
    tasks[StandardTasks::always] = std::make_shared<Data::TaskQueue>(StandardTasks::always, false);
    tasks[StandardTasks::internal] = std::make_shared<Data::TaskQueue>(StandardTasks::internal, true);
    tasks[StandardTasks::script] = std::make_shared<Data::TaskQueue>(StandardTasks::script, true);

    //------------------------------------------
    // General Variables
    names.binary = name;
    names.state = "";
}

void GlobalSpace::initialize() {
    //------------------------------------------
    // Domain-Related

    // Inherit functions from child objects
    inherit(&globalDoc().shareScope(*this, ""));
    inherit(&renderer);

    // Initialize DomainModules
    DomainModule::Initializer::initGlobalSpace(this);

    //------------------------------------------
    // Update

    // Cannot be done here directly, as GlobalSpace::update() requires command line arguments to be parsed first!

    // Note: DO NOT call updateModules or updateInnerDomains here!
    // Even updating modules here causes a disparity between release and debug builds, possibly due to order of initialization
    // So we skip it all
    // If we ever need a full update beforehand, we should manually call update after full initialization
}

Constants::Error GlobalSpace::updateInnerDomains() {
    // For now, just update the JSON domain
    // Later on the logic here might be more complex
    // As more inner domains are added
    static auto& fullScope = globalDoc().shareScope(*this, "");
    Constants::Error const result = fullScope.update();
    // Renderer is not updated here, as it is updated in GlobalSpace::update()
    // TODO: See if we can generalize this so that we can safely call renderer.update() here as well
    return result;
}

Constants::Error GlobalSpace::update() {
    static bool queueParsed = false; // Indicates if the task queue has been parsed on this frame render
    static bool criticalStop = false; // Indicates if a critical stop has occurred
    Constants::Error lastCriticalResult = Constants::ErrorTable::NONE(); // Last critical error result

    //------------------------------------------
    /**
     * Parse queue in GlobalSpace.
     * Result determines if a critical stop is initiated.
     *
     * We do this once before rendering
     *
     * @note For now, all tasks are parsed even if the program is in console mode.
     *       This is useful as tasks like "spawn" or "echo" are directly executed.
     *       But might break for more complex tasks, so this should be taken into account later on,
     *       e.G. inside the GlobalSpace, checking state of Renderer might be useful
     */
    if (!queueParsed) {
        lastCriticalResult = parseQueue();
        criticalStop = lastCriticalResult != Constants::ErrorTable::NONE();
        queueParsed = true;
    }

    //------------------------------------------
    // Update and render, only if initialized
    if (!criticalStop && renderer.isSdlInitialized() && renderer.timeToRender()) {
        // Update modules first
        updateModules();

        // Then, update inner domains
        if (auto const result = updateInnerDomains(); result.isCritical() && !cmdVars.recover) {
            criticalStop = true;
            lastCriticalResult = result;
        }

        // Update renderer and draw frame
        if (!renderer.isSkippingUpdate()) {
            invoke.update();
            renderer.updateState();
        }
        renderer.update();

        if (!renderer.hasSkippedUpdate()) {
            for (auto const& t : tasks) {
                t.second->decrementWaitCounter();
            }
        }

        // Frame was rendered, meaning we potentially have new tasks to process
        queueParsed = false;
    }

    //------------------------------------------
    // Check if we need to continue the loop
    continueLoop = !criticalStop && renderer.isSdlInitialized() && !renderer.shouldQuit();

    // Overwrite: If there is a wait operation and no renderer exists,
    // we need to continue the loop and decrease scriptWaitCounter
    /**
     * @note It might be tempting to add the condition that all tasks are done,
     *       but this could cause issues if the user wishes to quit while a task is still running.
     */
    if (tasks[StandardTasks::script]->isWaiting() && !renderer.isSdlInitialized()) {
        continueLoop = true;
        tasks[StandardTasks::script]->decrementWaitCounter();

        // Parse new tasks on next loop
        queueParsed = false;
    }

    //------------------------------------------
    // Return last critical result if there was a critical stop
    return lastCriticalResult;
}

void GlobalSpace::parseCommandLineArguments(int const& argc, char const** argv) {
    //------------------------------------------
    // Add main args to taskList, split by ';'
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1)
                oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string argStr = oss.str();
        std::stringstream ss(argStr);
        std::string command;

        while (std::getline(ss, command, ';')) {
            // Trim whitespace from each command
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);
            if (!command.empty()) {
                tasks[StandardTasks::script]->pushBack(command);
            }
        }
    } else {
        /**
         * @note For now, an empty Renderer is initiated via set-fps 60 if no arguments are provided
         * @todo Later on it might be helpful to insert a task like:
         *       `env-load ./Resources/Levels/main.jsonc`
         *       Which represents the menue screen of the game.
         *       Or, for a more scripted task:
         *       `task TaskFiles/main.nebs`.
         *       Making sure that any state currently loaded is cleared.
         *       Having main be a state itself is also an idea,
         *       but this might become challenging as the user could accidentally overwrite the main state.
         *       Best solution is therefore an env-load, with the environment architecture yet to be defined
         *       best solution is probably:
         *       - a field with the container
         *       - a vector which contains tasks to be executed on environment load
         *       - potentially an extra task vector for tasks that are executed BEFORE the env is loaded
         *       - potentially an extra task vector for tasks that are executed BEFORE the env is de-loaded
         *       Keys like: after-load, after-deload, before-load, before-deload
         *       For easier usage, hardcoding the env-load task is not a good idea,
         *       instead call some function like "entrypoint" or "main" which is defined in a GlobalSpace DomainModule
         *       This is important, as it's now clear what the entrypoint is, without knowing exactly what main file is loaded
         *       If a user ever defines addition arguments via, e.g. Steam when launching the game, this might become a problem
         *       as any additional argument would make the entrypoint not be called.
         *       So later on, we might consider always calling entrypoint as first task AFTER the command line arguments are parsed
         *       This is necessary, as the user might define important configurations like --headless, which would not be set if the renderer is initialized before them.
         */
        tasks[StandardTasks::script]->pushBack("set-fps 60");
    }
}

Constants::Error GlobalSpace::parseQueue() {
    queueResult.clear();
    for (auto const& t : tasks) {
        queueResult[t.first] = t.second->resolve(*this, cmdVars.recover);
        if (queueResult[t.first].encounteredCriticalResult && !cmdVars.recover) {
            return queueResult[t.first].errors.back();
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error GlobalSpace::preParse() {
    // NOTE: This function is only called once there is a parse-command
    // Meaning its timing is consistent and not dependent on framerate, frame time variations, etc.
    // Meaning everything we do here is, timing wise, deterministic!
    (void)floatingDM.rng->update();
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::Core
