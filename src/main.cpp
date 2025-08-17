// ----------------------------------------------------------------------
//     _   ____________  __  ____    ________________
//    / | / / ____/ __ )/ / / / /   /  _/_  __/ ____/
//   /  |/ / __/ / __  / / / / /    / /  / / / __/   
//  / /|  / /___/ /_/ / /_/ / /____/ /  / / / /___   
// /_/ |_/_____/_____/\____/_____/___/ /_/ /_____/   
//                                                     

/*
 * NEBULITE Engine - main.cpp
 *
 * Entry point for the NEBULITE game engine. This file initializes the core systems,
 * parses command-line arguments, and coordinates execution scenarios such as
 * loading levels or running performance benchmarks.
 *
 * Usage:
 *   Nebulite <command> <options>
 *
 * Example Commands:
 *  spawn <RenderObject.jsonc>       Load a single renderobject
 *  env-load <level.jsonc>           Load and render a level
 *  task <task.nebs>                 Run a tasktile
 */


// ----------------------------------------------------------------------
// Includes

// Include the Global Space class
// Initializes callable functions from both user CLI and runtime environment.
// Also sets up the global Renderer used across Tree-based function calls.
#include "GlobalSpace.h" 

// ----------------------------------------------------------------------
// NEBULITE main
/*
 * 
 * TODO:    settings.jsonc
 *          - Renderer size, fps setting
 *          - mapping of keyboard/mouse to specific actions like up/down/left/right, inventory, map etc.
 *          - issue here is, keys of invokes need to be dynamic?
 *            if key is "global.keyboard.current.$(settings.action.left)", resolve to "global.keyboard.current.a"
 *            so if key contains '$', resolve. This needs to happen in realtime in case the user changes settings midgame, so this cant be resolved on invoke load!
 *            probably the more dynamic approach, as linking the settings doc within the global doc through mapping is messy, convoluted and slower  
 *          - this should then allow for functioncall input set/release 0/1
 *          - keyboard state not in global anymore, all actions from global.input.xyz defined in settings.jsonc:
 *              "keyMapping" : {
 *                  "moveUp" : {
 *                      "primary" : "input.keyboard.d"
 *                      "secondary" : "input.keyboard.up"
 *                  }
 *              }
 *          - plus, move current/delta to last hierarchy: input.keyboard.d.current
 *          - could be solved through read-only files with invoke/filemanagement!
 * 
 */
int main(int argc, char* argv[]){

    //--------------------------------------------------
    // Initialize the global space
    std::string binaryName = argv[0];
    std::streambuf* originalCerrBuf = std::cerr.rdbuf();
    Nebulite::GlobalSpace globalSpace(binaryName, originalCerrBuf);

    //--------------------------------------------------
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
                globalSpace.tasks_script.taskList.push_back(command);
            }
        }
    }
    else{
        // If no addition arguments were provided:
        //
        // For now, an empty Renderer is initiated via set-fps 60
        // Later on it might be helpful to insert a task like:
        // "env-load ./Resources/Levels/main.jsonc" 
        // Which represents the menue screen of the game
        // or, for a more scripted task:
        // task TaskFiles/main.nebs
        // Making sure that any state currently loaded is cleared
        // Having main be a state itself is also an idea, but this might become challenging as the user could accidentally overwrite the main state
        //
        // Best solution is therefore an env-load, with the environment architecture yet to be defined
        // best solution is probably:
        // - a field with the container 
        // - a vector which contains tasks to be executed on environment load
        // - potentially an extra task vector for tasks that are executed BEFORE the env is loaded
        // - potentially an extra task vector for tasks that are executed BEFORE the env is de-loaded
        // Keys like: after-load, after-deload, before-load, before-deload
        //
        // For easier usage, hardcoding the env-load task is not a good idea, 
        // instead call some function like "entrypoint" or "main" which is defined in the GlobalSpaceTree
        // This is important, as it's now clear what the entrypoint is, without knowing exactly what main file is loaded
        // If a user ever defines addition arguments via, e.g. Steam when launching the game, this might become a problem
        // as any additional argument would make the entrypoint not be called.
        // So later on, we might consider always calling entrypoint as first task AFTER the command line arguments are parsed
        // This is necessary, as the user might define important configurations like --headless, which would not be set if the renderer is initialized before them.
        globalSpace.tasks_script.taskList.push_back(std::string("set-fps 60"));
    }
    
    //--------------------------------------------------
    // Render loop

    // For resolving tasks
    Nebulite::taskQueueResult result_tasks_script;
    Nebulite::taskQueueResult result_tasks_internal;
    Nebulite::taskQueueResult result_tasks_always;
    Nebulite::ERROR_TYPE lastCriticalResult;
    bool critical_stop = false;
    uint64_t* noWaitCounter = nullptr;

    //--------------------------------------------------
    // At least one loop, to handle taskQueues
    do {
        //------------------------------------------------------------
        // Parse queue in GlobalSpaceTree
        // Result determines if a critical stop is initiated
        //
        // For now, all tasks are parsed even if the program is in console mode
        // This is useful as tasks like "spawn" or "echo" are directly executed
        // But might break for more complex tasks, so this should be taken into account later on
        // e.G. inside the FuncTree, checking state of Renderer might be useful

        // 1.) Clear errors from last loop
        result_tasks_script.errors.clear();
        result_tasks_internal.errors.clear();
        result_tasks_always.errors.clear();

        // 2.) Parse script tasks
        if(!critical_stop){
            result_tasks_script = globalSpace.resolveTaskQueue(globalSpace.tasks_script, &globalSpace.scriptWaitCounter);
        }
        if(result_tasks_script.stoppedAtCriticalResult && globalSpace.recover == "false") {
            critical_stop = true; 
            lastCriticalResult = result_tasks_script.errors.back();
            break;
        } 

        // 3.) Parse internal tasks
        if(!critical_stop){
            result_tasks_internal = globalSpace.resolveTaskQueue(globalSpace.tasks_internal, noWaitCounter);
        }
        if(result_tasks_internal.stoppedAtCriticalResult && globalSpace.recover == "false") {
            critical_stop = true; 
            lastCriticalResult = result_tasks_internal.errors.back();
            break;
        }

        // 4.) Parse always-tasks
        if(!critical_stop){
            result_tasks_always = globalSpace.resolveTaskQueue(globalSpace.tasks_always, noWaitCounter);
        }
        if(result_tasks_always.stoppedAtCriticalResult && globalSpace.recover == "false") {
            critical_stop = true; 
            lastCriticalResult = result_tasks_always.errors.back();
            break;
        }

        //------------------------------------------------------------
        // Update and render, only if initialized
        // If renderer wasnt initialized, it is still a nullptr
        if (!critical_stop && globalSpace.RendererExists() && globalSpace.getRenderer()->timeToRender()) {
            globalSpace.GlobalSpaceTree->update();
            globalSpace.getRenderer()->tick();

            // Reduce script wait counter if not in console mode
            if(!globalSpace.getRenderer()->isConsoleMode()){
                if(globalSpace.scriptWaitCounter > 0) globalSpace.scriptWaitCounter--; 
                if(globalSpace.scriptWaitCounter < 0) globalSpace.scriptWaitCounter = 0;
            }  
        }
    // Note 1:
    // Continue only if renderer exists, and if quit wasnt called.
    // It might be tempting to add the condition that all tasks are done, 
    // but this could cause issues if the user wishes to quit while a task is still running.
    //
    // Note 2:
    // A current limitation is that, if the user is running a taskfile with a wait-call, a renderer has to be initialized.
    } while (!critical_stop && globalSpace.RendererExists() && !globalSpace.getRenderer()->isQuit());


    //--------------------------------------------------
    // Exit

    // Destroy renderer
    if(globalSpace.RendererExists()) globalSpace.getRenderer()->destroy();

    // Inform user about any errors and return error code
    if(critical_stop){
        std::cerr << "Critical Error: " << globalSpace.errorTable.getErrorDescription(lastCriticalResult) << std::endl;
    }

    // turn off error log
    globalSpace.parseStr("log off");

    // Return 1 on critical stop, 0 otherwise
    return (int)critical_stop;
}