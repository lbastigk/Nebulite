#include "Core/GlobalSpace.hpp"
#include "DomainModule/GSDM.hpp"

#include "Constants/KeyNames.hpp"


Nebulite::Core::GlobalSpace::GlobalSpace(const std::string binName)
: Nebulite::Interaction::Execution::Domain<Nebulite::Core::GlobalSpace>("Nebulite", this, &global)
{
    //------------------------------------------
    // Modify structs                         
    tasks.always.clearAfterResolving = false;

    //------------------------------------------
    // Objects and linkages 
    renderer = nullptr; // Uninitialized
    invoke = std::make_unique<Nebulite::Interaction::Invoke>(&global);
    invoke->linkTaskQueue(tasks.internal.taskQueue);

    //------------------------------------------
    // General Variables
    names.binary = binName;
    names.state  = "";

    //------------------------------------------
    // Domain-Related

    // Set preParse function
    setPreParse(std::bind(&Nebulite::Core::GlobalSpace::preParse, this));

    // Link inherited Domains
    inherit<Nebulite::Utility::JSON>(&global);

    // Initialize DomainModules
    Nebulite::DomainModule::GSDM_init(this);

    //------------------------------------------
    // Update

    // Cannot be done here directly, as GlobalSpace::update() requires command line arguments to be parsed first!
    // Instead, we just update the inner modules and domains once

    // Update modules first
    updateModules();

    // Then, update inner domains
    updateInnerDomains();
}

Nebulite::Constants::Error Nebulite::Core::GlobalSpace::updateInnerDomains(){
    // For now, just update the JSON domain
    // Later on the logic here might be more complex
    // As more inner domains are added
    Nebulite::Constants::Error result = getDoc()->update();
    return result;
}

Nebulite::Constants::Error Nebulite::Core::GlobalSpace::update() {
    static bool queueParsed = false;   // Indicates if the task queue has been parsed on this frame render
    static bool criticalStop = false;  // Indicates if a critical stop has occurred
    Nebulite::Constants::Error lastCriticalResult = Nebulite::Constants::ErrorTable::NONE(); // Last critical error result

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
    if(!queueParsed){
        lastCriticalResult = parseQueue();
        criticalStop = (lastCriticalResult != Nebulite::Constants::ErrorTable::NONE());
        queueParsed = true;
    }

    //------------------------------------------
    // Update and render, only if initialized
    // If renderer wasnt initialized, it is still a nullptr
    if (!criticalStop && RendererExists() && getRenderer()->timeToRender()) {
        // Update modules first
        updateModules();

        // Then, update inner domains
        updateInnerDomains();

        // Update Renderer
        bool didUpdate = getRenderer()->tick();

        // Reduce script wait counter if not in console mode or other halting states
        if(didUpdate){
            if(scriptWaitCounter > 0) scriptWaitCounter--; 
            if(scriptWaitCounter < 0) scriptWaitCounter = 0;
        }  

        // Frame was rendered, meaning we potentially have new tasks to process
        queueParsed = false;
    }

    //------------------------------------------
    // Check if we need to continue the loop
    continueLoop = !criticalStop && RendererExists() && !getRenderer()->isQuit();

    // Overwrite: If there is a wait operation and no renderer exists, 
    // we need to continue the loop and decrease scriptWaitCounter
    /**
     * @note It might be tempting to add the condition that all tasks are done,
     *       but this could cause issues if the user wishes to quit while a task is still running.
     */
    if(scriptWaitCounter > 0 && !RendererExists()){
        continueLoop = true;
        scriptWaitCounter--;

        // Parse new tasks on next loop
        queueParsed = false;
    }

    //------------------------------------------
    // Return last critical result if there was a critical stop
    return lastCriticalResult;
}

void Nebulite::Core::GlobalSpace::parseCommandLineArguments(int argc, char* argv[]){
    //------------------------------------------
    // Add main args to taskList, split by ';'
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) oss << ' ';
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
                tasks.script.taskQueue.push_back(command);
            }
        }
    }
    else{
        /**
         * If no addition arguments were provided:
         *
         * @note For now, an empty Renderer is initiated via set-fps 60
         * 
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
         * 
         *       - a field with the container 
         * 
         *       - a vector which contains tasks to be executed on environment load
         * 
         *       - potentially an extra task vector for tasks that are executed BEFORE the env is loaded
         * 
         *       - potentially an extra task vector for tasks that are executed BEFORE the env is de-loaded
         * 
         *       Keys like: after-load, after-deload, before-load, before-deload
         *       For easier usage, hardcoding the env-load task is not a good idea, 
         *       instead call some function like "entrypoint" or "main" which is defined in a GlobalSpace DomainModule
         *       This is important, as it's now clear what the entrypoint is, without knowing exactly what main file is loaded
         *       If a user ever defines addition arguments via, e.g. Steam when launching the game, this might become a problem
         *       as any additional argument would make the entrypoint not be called.
         *       So later on, we might consider always calling entrypoint as first task AFTER the command line arguments are parsed
         *       This is necessary, as the user might define important configurations like --headless, which would not be set if the renderer is initialized before them.
         *    
         */
        tasks.script.taskQueue.push_back(std::string("set-fps 60"));
    }
}

Nebulite::Core::Renderer* Nebulite::Core::GlobalSpace::getRenderer() {
    if (!rendererInitialized) {
        renderer = std::make_unique<Nebulite::Core::Renderer>(this, cmdVars.headless == "true");
        renderer->setTargetFPS(60); // Standard value for a fresh renderer
        rendererInitialized = true;
    }
    return renderer.get();
}

SDL_Renderer* Nebulite::Core::GlobalSpace::getSDLRenderer() {
    return getRenderer()->getSdlRenderer();
}

bool Nebulite::Core::GlobalSpace::RendererExists(){
    return rendererInitialized;
}

Nebulite::Core::taskQueueResult Nebulite::Core::GlobalSpace::resolveTaskQueue(Nebulite::Core::taskQueueWrapper& tq, uint64_t* waitCounter){
    Nebulite::Constants::Error currentResult = Nebulite::Constants::ErrorTable::NONE();
    Nebulite::Core::taskQueueResult result;

    // If clearAfterResolving, process and pop each element
    if (tq.clearAfterResolving) {
        while (!tq.taskQueue.empty() && !result.stoppedAtCriticalResult) {
            if (waitCounter != nullptr && *waitCounter > 0) break;

            std::string argStr = tq.taskQueue.front();
            tq.taskQueue.pop_front();

            // Add binary name if missing
            // While args from command line have binary name in them, 
            // commands from Renderobject, taskfile or console do not.
            // Is needed for correct parsing; argv[0] is alwys binary name.
            if (!argStr.starts_with(names.binary + " ")) {
                argStr = names.binary + " " + argStr;
            }

            // Parse
            currentResult = parseStr(argStr);

            // Check result
            if (currentResult.isCritical()) {
                result.stoppedAtCriticalResult = true;
            }
            result.errors.push_back(currentResult);
        }
    } else {
        // If not clearing, process every element without popping
        for (const auto& argStrOrig : tq.taskQueue) {
            if (result.stoppedAtCriticalResult) break;
            if (waitCounter != nullptr && *waitCounter > 0) break;

            // Add binary name if missing
            // While args from command line have binary name in them, 
            // commands from Renderobject, taskfile or console do not.
            // Is needed for correct parsing; argv[0] is alwys binary name.
            std::string argStr = argStrOrig;
            if (!argStr.starts_with(names.binary + " ")) {
                argStr = names.binary + " " + argStr;
            }

            // Parse
            currentResult = parseStr(argStr);

            // Check result
            if (currentResult.isCritical()) {
                result.stoppedAtCriticalResult = true;
            }
            result.errors.push_back(currentResult);
        }
    }

    return result;
}

Nebulite::Constants::Error Nebulite::Core::GlobalSpace::parseQueue() {
    uint64_t* noWaitCounter = nullptr;
    Nebulite::Constants::Error lastCriticalResult = Nebulite::Constants::ErrorTable::NONE();

    // 1.) Clear errors from last loop
    queueResult.script.errors.clear();
    queueResult.internal.errors.clear();
    queueResult.always.errors.clear();

    // 2.) Parse script tasks
    queueResult.script = resolveTaskQueue(tasks.script, &scriptWaitCounter);
    if(queueResult.script.stoppedAtCriticalResult && cmdVars.recover == "false") {
        lastCriticalResult = queueResult.script.errors.back();
        return lastCriticalResult;
    } 

    // 3.) Parse internal tasks
    queueResult.internal = resolveTaskQueue(tasks.internal, noWaitCounter);
    if(queueResult.internal.stoppedAtCriticalResult && cmdVars.recover == "false") {
        lastCriticalResult = queueResult.internal.errors.back();
        return lastCriticalResult;
    }

    // 4.) Parse always-tasks
    queueResult.always = resolveTaskQueue(tasks.always, noWaitCounter);
    if(queueResult.always.stoppedAtCriticalResult && cmdVars.recover == "false") {
        lastCriticalResult = queueResult.always.errors.back();
        return lastCriticalResult;
    }

    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::Core::GlobalSpace::preParse() {
    // NOTE: This function is only called once there is a parse-command
    // Meaning its timing is consistent and not dependent on framerate, frametime variations, etc.
    // Meaning everything we do here is, timing wise, deterministic!

    // Update RNGs
    // Disabled if renderer skipped update last frame, active otherwise
    bool RNG_update_enabled = rendererInitialized && getRenderer()->hasSkippedUpdate() == false;
    RNG_update_enabled |= !rendererInitialized; // If renderer is not initialized, we always update RNGs
    if(RNG_update_enabled){
        std::string seed = getLastParsedString();
        updateRNGs(seed);
    }

    return Nebulite::Constants::ErrorTable::NONE();
}

/**
 * @define RNG_SEEDER_VOLATILE
 * @brief If defined, the RNG updater will use the provided seed + last RNG value to generate a new RNG value.
 * if not defined, it will only use the last RNG value + a constant string.
 * 
 * @note If we ever notice inconsistencies between platforms, we should undefine this.
 */
#define RNG_SEEDER_VOLATILE 0

void Nebulite::Core::GlobalSpace::updateRNGs(std::string seed){
    // Set Min and Max values for RNGs in document
    // Always set, so overwrites dont stick around
    global.set<RNGvars::rng_size_t>(Nebulite::Constants::keyName.random.min.c_str(), std::numeric_limits<RNGvars::rng_size_t>::min());
    global.set<RNGvars::rng_size_t>(Nebulite::Constants::keyName.random.max.c_str(), std::numeric_limits<RNGvars::rng_size_t>::max());

    // Set a normalized seed if volatile seeding is enabled
    // if not, the normalized seed remains empty
    std::string normalized_seed = "";
    #ifdef RNG_SEEDER_VOLATILE
        // We need to remove all links from the seed, so it's consistent
        // This is because links arent relative to the binaries root, but absolute paths
        // Meaning two users running the same command with different folder structures would get different RNG values otherwise
        std::vector<std::string> args = Nebulite::Utility::StringHandler::parseQuotedArguments(seed);
        for(auto& arg : args){
            // All args that arent links are added to the seed
            if(arg.find("/") != std::string::npos || arg.find("\\") != std::string::npos){
                continue;

            } else {
                normalized_seed += arg + " ";
            }
        }
    #endif

    // Generate seeds
    // If we ever notice inconsistencies, we can always just hash the last rng + some constant string
    // Perhaps the provided seed becomes hard to normalize (removing platform-specific paths etc.)
    std::string seedA = normalized_seed + "A" + std::to_string(rng.A.get());
    std::string seedB = normalized_seed + "B" + std::to_string(rng.B.get());
    std::string seedC = normalized_seed + "C" + std::to_string(rng.C.get());
    std::string seedD = normalized_seed + "D" + std::to_string(rng.D.get());

    // Hash seeds
    rng.A.update(seedA);
    rng.B.update(seedB);
    rng.C.update(seedC);
    rng.D.update(seedD);

    // Set RNG values in global document
    global.set<RNGvars::rng_size_t>(Nebulite::Constants::keyName.random.A.c_str(), rng.A.get());
    global.set<RNGvars::rng_size_t>(Nebulite::Constants::keyName.random.B.c_str(), rng.B.get());
    global.set<RNGvars::rng_size_t>(Nebulite::Constants::keyName.random.C.c_str(), rng.C.get());
    global.set<RNGvars::rng_size_t>(Nebulite::Constants::keyName.random.D.c_str(), rng.D.get());
}