#include "Core/GlobalSpace.h"
#include "Interaction/Execution/GlobalSpaceTree.h"

Nebulite::Core::GlobalSpace::GlobalSpace(const std::string binName)
{
    //------------------------------------------
    // Objects
    // ...

    //------------------------------------------
    // Modify structs                         
    tasks.always.clearAfterResolving = false;

    //------------------------------------------
    // Linkages 
    invoke = std::make_unique<Nebulite::Interaction::Invoke>(&global);
    invoke->linkQueue(tasks.internal.taskList);

    //------------------------------------------
    // Link GlobalSpaceTree
    GlobalSpaceTree = std::make_unique<Nebulite::Interaction::Execution::GlobalSpaceTree>(this, global.getJSONTree());

    //------------------------------------------
    // General Variables
    names.binary = binName;
    names.state  = "";

    //------------------------------------------
    // Do first update
    GlobalSpaceTree->update();
}

Nebulite::Core::Renderer* Nebulite::Core::GlobalSpace::getRenderer() {
    if (renderer == nullptr) {
        renderer = std::make_unique<Nebulite::Core::Renderer>(*invoke, global, cmdVars.headless == "true");
        renderer->setTargetFPS(60);
    }
    return renderer.get();
}

bool Nebulite::Core::GlobalSpace::RendererExists(){
    return renderer != nullptr;
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
            currentResult = GlobalSpaceTree->parseStr(argStr);

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
            currentResult = GlobalSpaceTree->parseStr(argStr);

            // Check result
            if (currentResult < Nebulite::Constants::ERROR_TYPE::NONE) {
                result.stoppedAtCriticalResult = true;
            }
            result.errors.push_back(currentResult);
        }
    }

    return result;
}

Nebulite::Constants::ERROR_TYPE Nebulite::Core::GlobalSpace::parseStr(std::string str) {
    // Strings first arg must be the binary name or similar
    if (!str.starts_with(names.binary + " ")) {
        str = names.binary + " " + str; // Add binary name if missing
    }

    // Since JSON is linked inside the GlobalSpaceTree, we can parse directly
    return GlobalSpaceTree->parseStr(str);
}