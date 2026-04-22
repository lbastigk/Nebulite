//------------------------------------------
// Includes

#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "Module/Domain/Initializer.hpp"
#include "Module/Domain/GlobalSpace/Settings.hpp"

//------------------------------------------
namespace Nebulite::Core {

GlobalSpace::GlobalSpace(std::string const& name) :
    Domain("GlobalSpace", Global::shareScope(ScopeAccessor::Full(), "")), // Domain with reference to GlobalSpace and its full scope
    renderer(
        Global::shareScope(ScopeAccessor::Full(), "renderer"),
        &cmdVars.headless,
        capture
    )
{
    //------------------------------------------
    // Ensure GlobalSpace id is zero
    if (getId() != 0) {
        throw std::runtime_error("GlobalSpace must have an id of zero! Current id: " + std::to_string(getId()) + ". This means another Domain is initialized before GlobalSpace. Please fix.");
    }

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

Constants::Event GlobalSpace::updateInnerDomains() {
    // Update renderer if nothing is stopping us
    if (!renderer.isSkippingUpdate()) { // e.g. Console mode might flag renderer to skip update
        for (auto const& tq : std::views::values(tasks)) {
            tq->decrementWaitCounter();
        }
        invoke.update();        // Invoke broadcasted-listen-updates
        auto const event = renderer.update();      // Renderer updates its inner domains (e.g. RenderObjects)

        // Increment frame count and return event
        static size_t frameCount = 0;
        static auto constexpr frameCountKey = Data::ScopedKeyView("time.frameCount");
        domainScope.set<uint64_t>(frameCountKey, frameCount); // Starts at 0
        frameCount++;
        return event;
    }
    return Constants::Event::Success;
}

Constants::Event GlobalSpace::update() {
    static bool queueParsed = false; // Indicates if the task queue has been parsed on this frame render

    //------------------------------------------
    // TaskQueue parsing
    if (!queueParsed) {
        notifyEvent(parseQueue());
        queueParsed = true;
    }

    //------------------------------------------
    // Compared to all other domains,
    // we cannot simply update inner domains all the time.
    // This is because the GlobalSpace is the uppermost Domain
    // And is responsible for the proper update timing.
    // We do this by checking if enough time has passed since the last renderer to reach the target FPS.
    if (continueLoop && renderer.isSdlInitialized() && renderer.timeToRender()) {
        // Update modules first
        updateModules();

        // Then, update inner domains
        notifyEvent(updateInnerDomains());

        // Render frame
        renderer.render();

        // Frame was rendered, meaning we potentially have new tasks to process next frame
        queueParsed = false;
    }

    //------------------------------------------
    // Check if we need to continue the loop
    continueLoop = continueLoop && renderer.isSdlInitialized() && !renderer.shouldQuit();
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

    return Constants::Event::Success;
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
            if (std::string const cmd = Global::settings().get<std::string>(DomainModule::GlobalSpace::Settings::Key::parseIfNoArgs + "[" + std::to_string(i) + "]").value_or(""); !cmd.empty()) {
                tasks[StandardTasks::script]->pushBack(cmd);
            }
        }
    }
}

Constants::Event GlobalSpace::parseQueue() {
    Interaction::Context ctx{*this, *this, *this};
    Interaction::ContextScope ctxScope{domainScope, domainScope, domainScope};
    queueResult.clear();
    for (auto const& [name, queue] : tasks) {
        queueResult[name] = queue->resolve(ctx, ctxScope, cmdVars.recover);
        if (queueResult[name].encounteredCriticalResult && !cmdVars.recover) {
            return queueResult[name].events.back();
        }
    }
    return Constants::Event::Success;
}

Constants::Event GlobalSpace::preParse() {
    // NOTE: This function is only called once there is a parse-command
    // Meaning its timing is consistent and not dependent on framerate, frame time variations, etc.
    // Meaning everything we do here is, timing wise, deterministic!
    (void)floatingDM.rng->update();
    return Constants::Event::Success;
}

} // namespace Nebulite::Core
