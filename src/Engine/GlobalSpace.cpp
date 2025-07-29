#include "GlobalSpace.h"
#include "MainTree.h"

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
    global = new Nebulite::JSON();
    renderer = nullptr; // Renderer will be initialized later

    //-------------------------------------------------
    // Modify structs                         
    tasks_always.clearAfterResolving = false;

    //-------------------------------------------------
    // Linkages 
    invoke = std::make_unique<Invoke>();
    invoke->linkGlobal(*global);
    invoke->linkQueue(tasks_internal.taskList);

    //-------------------------------------------------
    // MainTree
    mainTree = std::make_unique<Nebulite::MainTree>(invoke.get(), this);

    //-------------------------------------------------
    // General Variables
    errorLogStatus = false;
    _binName = binName;
    stateName = "";
}

Nebulite::Renderer* Nebulite::GlobalSpace::getRenderer() {
    if (renderer == nullptr) {
        renderer = new Nebulite::Renderer(*invoke, *global, mainTree->headless == "true");
        renderer->setFPS(60);
    }
    return renderer;
}

bool Nebulite::GlobalSpace::RendererExists(){
    return renderer != nullptr;
}

Nebulite::taskQueueResult Nebulite::GlobalSpace::resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree){
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
            currentResult = mainTree->parseStr(argStr);

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
            currentResult = mainTree->parseStr(argStr);

            // Check result
            if (currentResult < Nebulite::ERROR_TYPE::NONE) {
                result.stoppedAtCriticalResult = true;
            }
            result.errors.push_back(currentResult);
        }
    }

    return result;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpace::resolveTask(std::string task) {
    if (!task.starts_with(_binName + " ")) {
        task = _binName + " " + task; // Add binary name if missing
    }
    return mainTree->parseStr(task);
}