//------------------------------------------------
// Main Tree attached functions 
#include "Nebulite.h"
#include "MainTree.h"

// TODO:
// Separate queues for script and internal
// Otherwise, a wait from a script can halt the entire game logic
// All wait calls influence script queue for now
namespace Nebulite{
    //-------------------------------------------------
    // Variables/Objects in Nebulite scope

    // Tasks
    taskQueue tasks_script;
    taskQueue tasks_internal;
    taskQueue tasks_always;

    // Objects
    Invoke invoke;
    Nebulite::MainTree MainTree(&invoke);           // TODO: This seems to error in vscode, but works with cmake... "message": "variable \"Nebulite::MainTree\" is not a type name",
    Nebulite::JSON* global = new Nebulite::JSON();
    Nebulite::Renderer* renderer = nullptr;

    // General variables
    std::string stateName = ""; // On startup, no state
    std::string binName = "";   // Initialized in main, from argv[0]

    // Error log variables
    bool errorLogStatus = false;
    std::ofstream errorFile;
    std::streambuf* originalCerrBuf = nullptr;

    
    
    //-------------------------------------------------
    // Initializing variables/Objects
    void init(){
        //-------------------------------------------------
        // Modify structs                         
        tasks_always.clearAfterResolving = false;   // always-tasks are not cleared

        //-------------------------------------------------
        // Linkages 
        invoke.linkGlobal(*global);
	    invoke.linkQueue(tasks_internal.taskList);
    }

    // Getting renderer pointer. If Renderer isnt initialized, initialize first
    Renderer* getRenderer() {
        if (renderer == nullptr) {
            renderer = new Nebulite::Renderer(invoke, *global);
            renderer->setFPS(60);
        }
        return renderer;
    }

    // Check if renderer exists
    bool RendererExists(){
        return renderer != nullptr;
    }
}

Nebulite::taskQueueResult Nebulite::resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree){
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
            if (!argStr.starts_with(Nebulite::binName + " ")) {
                argStr = Nebulite::binName + " " + argStr;
            }

            // Parse
            currentResult = Nebulite::MainTree.parseStr(argStr);

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
            if (!argStr.starts_with(Nebulite::binName + " ")) {
                argStr = Nebulite::binName + " " + argStr;
            }

            // Parse
            currentResult = Nebulite::MainTree.parseStr(argStr);

            // Check result
            if (currentResult < Nebulite::ERROR_TYPE::NONE) {
                result.stoppedAtCriticalResult = true;
            }
            result.errors.push_back(currentResult);
        }
    }

    return result;
}


Nebulite::ERROR_TYPE Nebulite::resolveTask(std::string task) {
    return Nebulite::MainTree.parseStr(task);
}