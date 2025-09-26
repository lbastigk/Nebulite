#include "Core/GlobalSpace.hpp"
#include "DomainModule/GDM.hpp"

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
    Nebulite::DomainModule::GDM_init(this);

    //------------------------------------------
    // Do first update
    update();
}

void Nebulite::Core::GlobalSpace::update() {
    //------------------------------------------
    // Update Domain

    // Update modules first
    updateModules();

    // Then, update inner domains
    getDoc()->update();
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
    // Meaning everything we do here is deterministic!

    // We need to remove all links from the seed, so it's consistent
    auto args = Nebulite::Utility::StringHandler::parseQuotedArguments(getLastParsedString());

    std::string seed = "";
    for(auto& arg : args){
        // All args that arent links are added to the seed
        if(arg.starts_with("/") || arg.starts_with("\\")){
            continue;

        } else {
            seed += arg + " ";
        }
    }

    // Using static variables to reuse them next time
    static uint16_t A, B, C, D = 0;

    // Generate seeds
    std::string seedA = seed + "A" + std::to_string(A);
    std::string seedB = seed + "B" + std::to_string(B);
    std::string seedC = seed + "C" + std::to_string(C);
    std::string seedD = seed + "D" + std::to_string(D);

    // Hash seeds
    A = static_cast<uint16_t>(rng_hasher(seedA));
    B = static_cast<uint16_t>(rng_hasher(seedB));
    C = static_cast<uint16_t>(rng_hasher(seedC));
    D = static_cast<uint16_t>(rng_hasher(seedD));

    // Set RNG values in global document
    global.set<int>(Nebulite::Constants::keyName.random.A.c_str(), A);
    global.set<int>(Nebulite::Constants::keyName.random.B.c_str(), B);
    global.set<int>(Nebulite::Constants::keyName.random.C.c_str(), C);
    global.set<int>(Nebulite::Constants::keyName.random.D.c_str(), D);

    return Nebulite::Constants::ErrorTable::NONE();
}