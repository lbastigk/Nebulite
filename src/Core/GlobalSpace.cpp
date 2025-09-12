#include "Core/GlobalSpace.hpp"
#include "DomainModule/GDM.hpp"

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
    invoke->linkQueue(tasks.internal.taskList);

    //------------------------------------------
    // General Variables
    names.binary = binName;
    names.state  = "";

    //------------------------------------------
    // Link subtree global
    linkSubTree(global.funcTree);

    //------------------------------------------
    // Initialize DomainModules
    Nebulite::DomainModule::GDM_init(this);

    //------------------------------------------
    // Do first update
    update();
}

Nebulite::Core::Renderer* Nebulite::Core::GlobalSpace::getRenderer() {
    if (!rendererInitialized) {
        renderer = std::make_unique<Nebulite::Core::Renderer>(*invoke, *getDoc(), cmdVars.headless == "true");
        renderer->setTargetFPS(60); // Standard value for a fresh renderer
        rendererInitialized = true;
    }
    return renderer.get();
}

bool Nebulite::Core::GlobalSpace::RendererExists(){
    return rendererInitialized;
}

Nebulite::Core::taskQueueResult Nebulite::Core::GlobalSpace::resolveTaskQueue(Nebulite::Core::taskQueue& tq, uint64_t* waitCounter){
    Nebulite::Constants::ERROR_TYPE currentResult = Nebulite::Constants::ERROR_TYPE::NONE;
    Nebulite::Core::taskQueueResult result;

    // If clearAfterResolving, process and pop each element
    if (tq.clearAfterResolving) {
        while (!tq.taskList.empty() && !result.stoppedAtCriticalResult) {
            if (waitCounter != nullptr && *waitCounter > 0) break;

            std::string argStr = tq.taskList.front();
            tq.taskList.pop_front();

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
            if (currentResult < Nebulite::Constants::ERROR_TYPE::NONE) {
                result.stoppedAtCriticalResult = true;
            }
            result.errors.push_back(currentResult);
        }
    } else {
        // If not clearing, process every element without popping
        for (const auto& argStrOrig : tq.taskList) {
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
            if (currentResult < Nebulite::Constants::ERROR_TYPE::NONE) {
                result.stoppedAtCriticalResult = true;
            }
            result.errors.push_back(currentResult);
        }
    }

    return result;
}

Nebulite::Constants::ERROR_TYPE Nebulite::Core::GlobalSpace::parseQueue() {
    uint64_t* noWaitCounter = nullptr;
    Nebulite::Constants::ERROR_TYPE lastCriticalResult = Nebulite::Constants::ERROR_TYPE::NONE;

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

    return Nebulite::Constants::ERROR_TYPE::NONE;
}

void Nebulite::Core::GlobalSpace::update() {
    //------------------------------------------
    // Update Domain
    for(auto& module : modules){
        module->update();
    }
    getDoc()->update();
}