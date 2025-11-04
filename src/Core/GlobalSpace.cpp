#include "Core/GlobalSpace.hpp"
#include "DomainModule/Initializer.hpp"

#include "Constants/KeyNames.hpp"


Nebulite::Core::GlobalSpace::GlobalSpace(std::string const& binName)
: Nebulite::Interaction::Execution::Domain<Nebulite::Core::GlobalSpace>("Nebulite", this, &global, this),
  global(this),                       // Link the global document to the GlobalSpace
  renderer(this, &cmdVars.headless),  // Renderer with reference to GlobalSpace and headless mode boolean
  invoke(this),                       // Invoke with reference to GlobalSpace
  docCache(this)                      // Init with reference to GlobalSpace
{
    //------------------------------------------
    // Setup tasks                         
    tasks.always.clearAfterResolving = false;       // Always tasks are never cleared
    invoke.linkTaskQueue(tasks.internal.taskQueue); // Invoke pushes tasks to internal queue

    //------------------------------------------
    // General Variables
    names.binary = binName;
    names.state  = "";

    //------------------------------------------
    // Domain-Related

    // Set preParse function
    setPreParse([this] { return preParse(); });

    // Link inherited Domains
    inherit<Nebulite::Utility::JSON>(&global);
    inherit<Nebulite::Core::Renderer>(&renderer);

    // Initialize DomainModules
    Nebulite::DomainModule::Initializer::initGlobalSpace(this);

    //------------------------------------------
    // Update

    // Cannot be done here directly, as GlobalSpace::update() requires command line arguments to be parsed first!

    // Note: DO NOT call updateModules or updateInnerDomains here!
    // Even updating modules here causes a disparity between release and debug builds, possibly due to order of initialization
    // So we skip it all
    // If we ever need a full update beforehand, we should manually call update after full initialization
}

Nebulite::Constants::Error Nebulite::Core::GlobalSpace::updateInnerDomains() const {
    // For now, just update the JSON domain
    // Later on the logic here might be more complex
    // As more inner domains are added
    Nebulite::Constants::Error result = getDoc()->update();
    return result;
}

Nebulite::Constants::Error Nebulite::Core::GlobalSpace::update(){
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
    if (!criticalStop && renderer.isSdlInitialized() && renderer.timeToRender()){
        // Update modules first
        updateModules();

        // Then, update inner domains
        if (auto const result = updateInnerDomains(); result.isCritical() && !cmdVars.recover){
            criticalStop = true;
            lastCriticalResult = result;
        }

        // Do a Renderer tick and check if an update occurred
        // Reduce script wait counter if not in console mode or other halting states (tick returns false in those cases)
        if(renderer.tick(&invoke)){
            if(scriptWaitCounter > 0) scriptWaitCounter--; 
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
    if(scriptWaitCounter > 0 && !renderer.isSdlInitialized()){
        continueLoop = true;
        scriptWaitCounter--;

        // Parse new tasks on next loop
        queueParsed = false;
    }

    //------------------------------------------
    // Return last critical result if there was a critical stop
    return lastCriticalResult;
}

void Nebulite::Core::GlobalSpace::parseCommandLineArguments(int const& argc, char const* argv[]){
    //------------------------------------------
    // Add main args to taskList, split by ';'
    if (argc > 1){
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i){
            if (i > 1) oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string argStr = oss.str();
        std::stringstream ss(argStr);
        std::string command;

        while (std::getline(ss, command, ';')){
            // Trim whitespace from each command
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);
            if (!command.empty()){
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
        tasks.script.taskQueue.emplace_back("set-fps 60");
    }
}

Nebulite::Core::taskQueueResult Nebulite::Core::GlobalSpace::resolveTaskQueue(Nebulite::Core::taskQueueWrapper& tq, uint64_t const* waitCounter){
    Nebulite::Constants::Error currentResult;
    Nebulite::Core::taskQueueResult fullResult;

    // If clearAfterResolving, process and pop each element
    if (tq.clearAfterResolving){
        while (!tq.taskQueue.empty() && !fullResult.encounteredCriticalResult){
            if (waitCounter != nullptr && *waitCounter > 0) break;

            std::string argStr = tq.taskQueue.front();
            tq.taskQueue.pop_front();

            // Add binary name if missing
            if (!argStr.starts_with(names.binary + " ")){
                argStr.insert(0, names.binary + " ");
            }

            // Parse
            currentResult = parseStr(argStr);

            // Check result
            if (currentResult.isCritical()){
                fullResult.encounteredCriticalResult = true;
            }
            fullResult.errors.push_back(currentResult);
        }
    } else {
        // If not clearing, process every element without popping
        for (auto const& argStrOrig : tq.taskQueue){
            if (fullResult.encounteredCriticalResult) break;
            if (waitCounter != nullptr && *waitCounter > 0) break;

            // Add binary name if missing
            std::string argStr = argStrOrig;
            if (!argStr.starts_with(names.binary + " ")){
                argStr.insert(0, names.binary + " ");
            }

            // Parse
            currentResult = parseStr(argStr);

            // Check result
            if (currentResult.isCritical()){
                fullResult.encounteredCriticalResult = true;
            }
            fullResult.errors.push_back(currentResult);
        }
    }
    return fullResult;
}

Nebulite::Constants::Error Nebulite::Core::GlobalSpace::parseQueue(){
    static uint64_t const* noWaitCounter = nullptr;
    Nebulite::Constants::Error lastCriticalResult;

    // 1.) Clear errors from last loop
    queueResult.script.errors.clear();
    queueResult.internal.errors.clear();
    queueResult.always.errors.clear();

    // 2.) Parse script tasks
    queueResult.script = resolveTaskQueue(tasks.script, &scriptWaitCounter);
    if(queueResult.script.encounteredCriticalResult && !cmdVars.recover){
        lastCriticalResult = queueResult.script.errors.back();
        return lastCriticalResult;
    } 

    // 3.) Parse internal tasks
    queueResult.internal = resolveTaskQueue(tasks.internal, noWaitCounter);
    if(queueResult.internal.encounteredCriticalResult && !cmdVars.recover){
        lastCriticalResult = queueResult.internal.errors.back();
        return lastCriticalResult;
    }

    // 4.) Parse always-tasks
    queueResult.always = resolveTaskQueue(tasks.always, noWaitCounter);
    if(queueResult.always.encounteredCriticalResult && !cmdVars.recover){
        lastCriticalResult = queueResult.always.errors.back();
        return lastCriticalResult;
    }

    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::Core::GlobalSpace::preParse(){
    // NOTE: This function is only called once there is a parse-command
    // Meaning its timing is consistent and not dependent on framerate, frame time variations, etc.
    // Meaning everything we do here is, timing wise, deterministic!

    // Update RNGs
    // Disabled if renderer skipped update last frame, active otherwise
    bool RNG_update_enabled = renderer.isSdlInitialized() && renderer.hasSkippedUpdate() == false;
    RNG_update_enabled |= !renderer.isSdlInitialized(); // If renderer is not initialized, we always update RNGs
    if(RNG_update_enabled){
        updateRNGs();
    }

    return Nebulite::Constants::ErrorTable::NONE();
}

void Nebulite::Core::GlobalSpace::updateRNGs(){
    // Set Min and Max values for RNGs in document
    // Always set, so overwrites don't stick around
    global.set<RngVars::rngSize_t>(Nebulite::Constants::keyName.RNGs.min, std::numeric_limits<RngVars::rngSize_t>::min());
    global.set<RngVars::rngSize_t>(Nebulite::Constants::keyName.RNGs.max, std::numeric_limits<RngVars::rngSize_t>::max());

    // Generate seeds in a predictable manner
    // Since updateRNG is called at specific times only, we can simply increment RNG with a new seed
    std::string seedA = "A" + std::to_string(rng.A.get());
    std::string seedB = "B" + std::to_string(rng.B.get());
    std::string seedC = "C" + std::to_string(rng.C.get());
    std::string seedD = "D" + std::to_string(rng.D.get());

    // Hash seeds
    rng.A.update(seedA);
    rng.B.update(seedB);
    rng.C.update(seedC);
    rng.D.update(seedD);

    // Set RNG values in global document
    global.set<RngVars::rngSize_t>(Nebulite::Constants::keyName.RNGs.A, rng.A.get());
    global.set<RngVars::rngSize_t>(Nebulite::Constants::keyName.RNGs.B, rng.B.get());
    global.set<RngVars::rngSize_t>(Nebulite::Constants::keyName.RNGs.C, rng.C.get());
    global.set<RngVars::rngSize_t>(Nebulite::Constants::keyName.RNGs.D, rng.D.get());
}