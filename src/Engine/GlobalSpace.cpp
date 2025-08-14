#include "GlobalSpace.h"
#include "GlobalSpaceTree.h"

Nebulite::GlobalSpace::GlobalSpace(const std::string binName, std::streambuf*& originalCerrBufRef)
    : originalCerrBuf(originalCerrBufRef)
{

    //-------------------------------------------------
    // Initialize error logging members
    errorFile = nullptr;              // Don't create ofstream yet
    errorLogStatus = false;
    originalCerrBuf = nullptr;

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
    errorLogStatus = false;
    _binName = binName;
    stateName = "";

    //-------------------------------------------------
    // Do first update
    GlobalSpaceTree->update();
}

Nebulite::Renderer* Nebulite::GlobalSpace::getRenderer() {
    if (renderer == nullptr) {
        renderer = std::make_unique<Nebulite::Renderer>(*invoke, global, headless == "true");
        renderer->setFPS(60);
    }
    return renderer.get();
}

bool Nebulite::GlobalSpace::RendererExists(){
    return renderer != nullptr;
}

Nebulite::taskQueueResult Nebulite::GlobalSpace::resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_GlobalSpaceTree, char*** argv_GlobalSpaceTree){
    Nebulite::ERROR_TYPE currentResult = Nebulite::ERROR_TYPE::NONE;
    Nebulite::taskQueueResult result;

    // If clearAfterResolving, process and pop each element
    if (tq.clearAfterResolving) {
        while (!tq.taskList.empty() && !result.stoppedAtCriticalResult) {
            // Counter logic
            if (counter && *counter != 0) break;

            std::string argStr = tq.taskList.front();
            tq.taskList.pop_front();

            // Add binary name if missing
            // While args from command line have binary name in them, 
            // commands from Renderobject, taskfile or console do not.
            // Is needed for correct parsing; argv[0] is alwys binary name.
            if (!argStr.starts_with(_binName + " ")) {
                argStr = _binName + " " + argStr;
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
            if (counter && *counter != 0) break;

            // Add binary name if missing
            // While args from command line have binary name in them, 
            // commands from Renderobject, taskfile or console do not.
            // Is needed for correct parsing; argv[0] is alwys binary name.
            std::string argStr = argStrOrig;
            if (!argStr.starts_with(_binName + " ")) {
                argStr = _binName + " " + argStr;
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

// TODO: Since there is a pseudo-inheritance GlobalSpaceTree -> JSONTree, 
// make sure that calling help shows both GlobalSpaceTree and JSONTree functions
// Same for RenderObjectTree -> JSONTree
// Best solution: inherit/bind inside the FuncTree class!
Nebulite::ERROR_TYPE Nebulite::GlobalSpace::parseStr(std::string str) {
    // Strings first arg must be the binary name or similar
    if (!str.starts_with(_binName + " ")) {
        str = _binName + " " + str; // Add binary name if missing
    }

    // Since JSON is linked inside the GlobalSpaceTree, we can parse directly
    return GlobalSpaceTree->parseStr(str);
}