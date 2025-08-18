#include "GlobalSpace.h"
#include "GlobalSpaceTree.h"

Nebulite::GlobalSpace::GlobalSpace(const std::string binName)
{
    //-------------------------------------------------
    // Objects
    // ...

    //-------------------------------------------------
    // Modify structs                         
    tasks_always.clearAfterResolving = false;

    //-------------------------------------------------
    // Linkages 
    invoke = std::make_unique<Invoke>(&global);
    invoke->linkQueue(tasks_internal.taskList);

    //-------------------------------------------------
    // Link GlobalSpaceTree
    GlobalSpaceTree = std::make_unique<Nebulite::GlobalSpaceTree>(this, global.getJSONTree());

    //-------------------------------------------------
    // General Variables
    names.binary = binName;
    names.state  = "";

    //-------------------------------------------------
    // Do first update
    GlobalSpaceTree->update();
}

Nebulite::Renderer* Nebulite::GlobalSpace::getRenderer() {
    if (renderer == nullptr) {
        renderer = std::make_unique<Nebulite::Renderer>(*invoke, global, cmdVars.headless == "true");
        renderer->setFPS(60);
    }
    return renderer.get();
}

bool Nebulite::GlobalSpace::RendererExists(){
    return renderer != nullptr;
}

Nebulite::taskQueueResult Nebulite::GlobalSpace::resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* waitCounter){
    Nebulite::ERROR_TYPE currentResult = Nebulite::ERROR_TYPE::NONE;
    Nebulite::taskQueueResult result;

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
            currentResult = GlobalSpaceTree->parseStr(argStr);

            // Check result
            if (currentResult < Nebulite::ERROR_TYPE::NONE) {
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
            currentResult = GlobalSpaceTree->parseStr(argStr);

            // Check result
            if (currentResult < Nebulite::ERROR_TYPE::NONE) {
                result.stoppedAtCriticalResult = true;
            }
            result.errors.push_back(currentResult);
        }
    }

    return result;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpace::parseStr(std::string str) {
    // Strings first arg must be the binary name or similar
    if (!str.starts_with(names.binary + " ")) {
        str = names.binary + " " + str; // Add binary name if missing
    }

    // Since JSON is linked inside the GlobalSpaceTree, we can parse directly
    return GlobalSpaceTree->parseStr(str);
}