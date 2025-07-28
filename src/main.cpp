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
 *  spawn <RenderObject.json>       Load a single renderobject
 *  env-load <level.json>           Load and render a level
 *  task <task.txt>                 Run a tasktile
 */

// General Architecture:
/*
[SEND UP]:      Flow of information, up
[SEND DOWN]:    Flow of information, down

----------------------------------------------------
main-loop:                      Parse command-line arguments and populate task queues
                                Initialize core engine systems
                                Build the main function tree

                                [SEND DOWN]:    To Renderer:    - simple commands like setting fps, cam-position etc
                                                                - Spawning RenderObjects

Invoke:                         Resolve interactions between objects, if logical expression is true
                                Either global   self-other-relationship
                                or local        just-self-relationship

                                Information is stored in Renderobjects: self and other
                                as well as in a global document, containing

                                [SEND DOWN]:    To RenderObject - modification via pointers

Renderer:                       SDL-Wrapper for all functions concerning rendering,
                                managing Container

                                [SEND UP]:      To Invoke:      - information about runtime (stored in global doc)
                                                                - information about inputs  (stored in global doc)

    Environment:                Container for all asstes in level, 
                                consists of multiple Conainter Layers:

    RenderObjectContainer[N]:   N many layers of RenderObjects
                                Sorted into tiles the size of Display, with 9 tiles active
                                so that at any point, everything in sight is loaded
                                Meaning: In order to always cover a sheet the size w*h, a 3x3 grid of those sheets is needed.

                                [ ][ ][ ][ ][ ]... | # - Loaded Tile
                                [ ][#][#][#][ ]... | X - Tile where Renderer Cam position is in
                                [ ][#][X][#][ ]... |
                                [ ][#][#][#][ ]... |
                                [ ][ ][ ][ ][ ]... |

                                Layer determines order of rendering, layer 1 being renderered first:

                                ||                              ||
                                vv User looking at render stack vv

                                |-----------------|
                                | Layer N         |
                                |-----------------|
                                | Layer N-1       |
                                |-----------------|
                                ...
                                |-----------------|
                                | Layer 1         |
                                |-----------------|

    RenderObjects:              Holds information about each object as well as their Interaction Rulesets via the Invoke class
                                On update, compares itself against all previously send Invokes from other Renderobjects 
                                if it is subscribed to that topic
                                e.g.: I am a moving object, subscribed to all Commands of type: Collision so that i dont moved into them
                                        that Command might do stuff like:   - setting my velocity to 0
                                                                            - setting a variable that tells me the nearest objects
                                                                            - setting a variable that tells me where a collision happened (left, right ...)
                                Local invokes are resolved as well, 
                                concerning only internal commands like:     - acceleration integration
                                                                            - player input resolving
                                                                            - animation

                                [SEND UP]:  To Invoke:      - Invoke commands listed in Object, including pointer to itself  


For more information, see doc/
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
 * TODO:    settings.json
 *          - Renderer size, fps setting
 *          - mapping of keyboard/mouse to specific actions like up/down/left/right, inventory, map etc.
 *          - issue here is, keys of invokes need to be dynamic?
 *            if key is "global.keyboard.current.$(settings.action.left)", resolve to "global.keyboard.current.a"
 *            so if key contains '$', resolve. This needs to happen in realtime in case the user changes settings midgame, so this cant be resolved on invoke load!
 *            probably the more dynamic approach, as linking the settings doc within the global doc through mapping is messy, convoluted and slower  
 *          - this should then allow for functioncall input set/release 0/1
 *          - keyboard state not in global anymore, all actions from global.input.xyz defined in settings.json:
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

    std::cout << "DEBUG: Creatimg a GlobalSpace Object..." << std::endl;
    Nebulite::GlobalSpace globalSpace(binaryName);

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
        // For now, an empty Renderer is initiated
        // Later on it might be helpful to insert a task like:
        // "env-load ./Resources/Levels/main.json" 
        // Which represents the menue screen of the game
        // or, for a more scripted task:
        // task TaskFiles/main.txt
        globalSpace.tasks_script.taskList.push_back(std::string("set-fps 60"));
    }
    
    //--------------------------------------------------
    // Render loop

    // For resolving tasks
    Nebulite::taskQueueResult result_tasks_script;
    Nebulite::taskQueueResult result_tasks_internal;
    Nebulite::taskQueueResult result_tasks_always;
    Nebulite::ERROR_TYPE lastCriticalResult;
    uint64_t* noWaitCounter = nullptr;  // Only the script task queue has a waitCounter, so we pass nullptr for others
    bool critical_stop = false;

    // A new set of argc/argv is needed to parse tasks
    // TaskQueue -> pop out string -> parse into argc/argv -> call MainTree.parse with argc/argv
    int    argc_mainTree = 0;
    char** argv_mainTree = nullptr;

    //--------------------------------------------------
    // At least one loop, to handle taskQueues
    do {
        //------------------------------------------------------------
        // Handle args, parse queue into mainTree and then call internal functions from Nebulite::mainTreeFunctions
        // Result determines if a critical stop is initiated
        //
        // For now, all tasks are parsed even if the program is in console mode
        // This is useful as tasks like "spawn" or "echo" are directly executed
        // But might break for more complex tasks, so this should be taken into account later on
        // e.G. inside the functree, checking state of Renderer might be useful

        // 1.) Clear errors from last loop
        result_tasks_script.errors.clear();
        result_tasks_internal.errors.clear();
        result_tasks_always.errors.clear();

        // 2.) Parse script tasks
        if(!critical_stop){
            // TODO: Windows Release build breaks here!
            result_tasks_script = globalSpace.resolveTaskQueue(globalSpace.tasks_script,&globalSpace.tasks_script.waitCounter,&argc_mainTree,&argv_mainTree);
        }
        if(result_tasks_script.stoppedAtCriticalResult) {
            critical_stop = true; 
            lastCriticalResult = result_tasks_script.errors.back();
            break;
        } 

        // 3.) Parse internal tasks
        if(!critical_stop){
            result_tasks_internal = globalSpace.resolveTaskQueue(globalSpace.tasks_internal,noWaitCounter,&argc_mainTree,&argv_mainTree);
        }
        if(result_tasks_internal.stoppedAtCriticalResult) {
            critical_stop = true; 
            lastCriticalResult = result_tasks_internal.errors.back();
            break;
        }

        // 4.) Parse always-tasks
        if(!critical_stop){
            result_tasks_always = globalSpace.resolveTaskQueue(globalSpace.tasks_always,noWaitCounter,&argc_mainTree,&argv_mainTree);
        }
        if(result_tasks_always.stoppedAtCriticalResult) {
            critical_stop = true; 
            lastCriticalResult = result_tasks_always.errors.back();
            break;
        }

        //------------------------------------------------------------
        // Update and render, only if initialized
        // If renderer wasnt initialized, it is still a nullptr
        if (!critical_stop && globalSpace.RendererExists() && globalSpace.getRenderer()->timeToRender()) {
            globalSpace.getRenderer()->tick();

            // Reduce wait counter if not in console mode
            if(!globalSpace.getRenderer()->isConsoleMode()){
                globalSpace.tasks_script.waitCounter--; 
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
        std::cerr << "Critical Error: " << lastCriticalResult << std::endl;
    }

    // turn off error log
    globalSpace.resolveTask("log off");

    // Return 1 on critical stop, 0 otherwise
    return (int)critical_stop;
}