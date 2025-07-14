//------------------------------------------------
// Main Tree attached functions 
#include "Nebulite.h"

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
    std::unique_ptr<Nebulite::Renderer> renderer = nullptr;
    Invoke invoke;
    std::unique_ptr<Nebulite::JSON> global = std::make_unique<Nebulite::JSON>();
    Nebulite::MainFuncTree mainFuncTree(&invoke);

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

    // Init nebulite functions
    

    // Getting renderer pointer. If Renderer isnt initialized, initialize first
    Renderer* getRenderer() {
        if (!renderer) {
            renderer = std::make_unique<Nebulite::Renderer>(invoke,*global);
            renderer->setFPS(60);
        }
        return renderer.get();
    }

    bool RendererExists(){
        return renderer != nullptr;
    }
}

Nebulite::taskQueueResult Nebulite::resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree){
    Nebulite::ERROR_TYPE currentResult = Nebulite::ERROR_TYPE::NONE;
    Nebulite::taskQueueResult result;

    bool processedPersistentTask = false;
    while (!tq.taskList.empty() && (counter == nullptr || *counter == 0) && !result.stoppedAtCriticalResult) {
        // Get task
        std::string argStr = tq.taskList.front();

        // Pop only if configured to clear
        if (tq.clearAfterResolving) {
            tq.taskList.pop_front();
        } else if (processedPersistentTask) {
            break;  // Avoid infinite loop
        }
        processedPersistentTask = true;

        // Convert std::string to argc, argv
        *argc_mainTree = 0;
        *argv_mainTree = nullptr;

        if (!argStr.starts_with(Nebulite::binName + " ")) {
            argStr = Nebulite::binName + " " + argStr;
        }

        currentResult = Nebulite::mainFuncTree.parseStr(argStr);

        if(currentResult < Nebulite::ERROR_TYPE::NONE){
            result.stoppedAtCriticalResult = true;
        }
        result.errors.push_back(currentResult);
    }

    return result;
}


