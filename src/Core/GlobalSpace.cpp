//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "Core/JsonScope.hpp"
#include "DomainModule/Initializer.hpp"
#include "DomainModule/GlobalSpace/Settings.hpp"

//------------------------------------------
namespace Nebulite::Core {

GlobalSpace::GlobalSpace(std::string const& name) :
    Domain("Nebulite", Global::shareScope(ScopeAccessor::Full(), "")), // Domain with reference to GlobalSpace and its full scope
    renderer(
        Global::shareScope(ScopeAccessor::Full(), "renderer"),
        &cmdVars.headless
    ) // Share only the renderer portion of the global document
{
    //------------------------------------------
    // Initialize floating DomainModules

    floatingDM.rng = createModule<GlobalSpace, DomainModule::GlobalSpace::RNG>(
        "RNG",
        Global::settings(),
        *this,
        getFuncTree()
    );



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
    inherit(&Global::shareScope(ScopeAccessor::Full(), ""));
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
    static auto& fullScope = Global::shareScope(ScopeAccessor::Full(), "");
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

        //========================================================

        // This part is a bit annoying, since we require a renderer for GlobalSpace module updates
        // But without this, the time domainmodule and all DomainModules depending on it
        // are inconsistent...

        // Update modules first
        updateModules();

        // Then, update inner domains
        if (auto const result = updateInnerDomains(); result.isCritical() && !cmdVars.recover) {
            criticalStop = true;
            lastCriticalResult = result;
        }

        //========================================================

        // Update renderer if nothing is stopping us
        if (!renderer.isSkippingUpdate()) { // e.g. Console mode might flag renderer to skip update
            for (auto const& tq : std::views::values(tasks)) {
                tq->decrementWaitCounter();
            }
            invoke.update();        // Invoke broadcasted-listen-updates
            renderer.update();

            // Increment frame count
            static size_t frameCount = 0;
            static auto constexpr frameCountKey = Data::ScopedKeyView("time.frameCount");
            domainScope.set<uint64_t>(frameCountKey, frameCount); // Starts at 0
            frameCount++;
        }

        // Render frame
        renderer.render();

        // Frame was rendered, meaning we potentially have new tasks to process next frame
        queueParsed = false;
    }

    //------------------------------------------
    // Check if we need to continue the loop
    continueLoop = !criticalStop && renderer.isSdlInitialized() && !renderer.shouldQuit();
    if (tasks[StandardTasks::script]->isWaiting() && !renderer.isSdlInitialized()) {
        /**
         * @brief Overwrite: If there is a wait operation and no renderer exists,
         *        we need to continue the loop and decrease scriptWaitCounter
         * @note It might be tempting to add the condition that all tasks are done,
         *       but this could cause issues if the user wishes to quit while a task is still running.
         */
        continueLoop = true;
        tasks[StandardTasks::script]->decrementWaitCounter();
        queueParsed = false;
    }

    //------------------------------------------
    // Return last critical result if there was a critical stop
    // main then evaluates this and decides what to do
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
        // Load standard commands from settings, type 'parseIfNoArgs', if no args provided
        auto const cmdCount = Global::settings().memberSize(DomainModule::GlobalSpace::Settings::Key::parseIfNoArgs);
        for (std::size_t i = 0; i < cmdCount; ++i) {
            std::string const cmd = Global::settings().get<std::string>(
                DomainModule::GlobalSpace::Settings::Key::parseIfNoArgs + "[" + std::to_string(i) + "]",
                ""
            );
            if (!cmd.empty()) {
                tasks[StandardTasks::script]->pushBack(cmd);
            }
        }
    }
}

Constants::Error GlobalSpace::parseQueue() {
    queueResult.clear();
    for (auto const& [name, queue] : tasks) {
        queueResult[name] = queue->resolve(*this, cmdVars.recover);
        if (queueResult[name].encounteredCriticalResult && !cmdVars.recover) {
            return queueResult[name].errors.back();
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
