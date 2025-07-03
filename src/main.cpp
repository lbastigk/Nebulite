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
 *  spawn <RenderObject.json>       load a single renderobject
 *  load <level.json>               Load and render a level
 *  task <task.txt>                 Run a tasktile
 */

// General Architecture:
/*

----------------------------------------------------
main-loop:                      Parse command-line arguments and populate task queues
                                Initialize core engine systems
                                Build the main function tree

                                down: simple commands like setting fps, cam-position etc

Invoke:                         Resolve interactions between objects, if logical expression is true
                                Either global   self-other-relationship
                                or local        just-self-relationship

                                Information is stored in Renderobjects: self and other
                                as well as in a global document, containing

                                down:

Renderer:                       SDL-Wrapper for all functions concerning rendering,
                                managing Container

                                up:     information about runtime (stored in global doc)
                                        information about inputs  (stored in global doc)

    Environment:                Container for all asstes in level

    RenderObjectContainer[N]:   N many layers of RenderObjects
                                Sorted into tiles the size of Display, with 9 tiles active
                                so that at any point, everything in sight is loaded

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

    RenderObjects:              Holds information about each object as well as their Interaction Rulesets - Invokes
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

                                up:     global invoke commands: echo, error, setting fps, moving cam, appending a preset taskfile etc.

*/



// ----------------------------------------------------------------------
// Includes

//Include the Nebulite Namespace with all its functions
// Initializes callable functions from both user CLI and runtime environment.
// Also sets up the global Renderer used across Tree-based function calls.
#include "Nebulite.h" 

// Used to build a tree of callable Functions and parsing arguments
#include "FuncTree.h"



// ----------------------------------------------------------------------
// NEBULITE main
/*
 * 
 * TODO:    Current implementation of result return only returns the result of the last maintree parse
 *          Perhaps the result should be used to determine when to stop the program as well, 
 *          instead of just checking Renderer::isQuit()
 *          E.g. close program if env-load is called on a non-existing file
 *          Idea here is to pass some sort of Nebulite::ErrorParse Struct that contains all errors and from what function in some vector 
 *          This might involve a restructuring of the return value!
 *          It might be bette to do some simpler return value catches and see if the return value is <0 -> abort or >0 warning, but continue.
 */
int main(int argc, char* argv[]) {
    //--------------------------------------------------
    // Startup, args handling
    Nebulite::binName = argv[0];

    // add main arg to argTokens:
    if(argc > 1){
        std::ostringstream oss;
        for (int i = 0; i < argc; ++i) {
            if (i > 0) oss << ' ';          // Add space between arguments
            oss << argv[i];
        }
        Nebulite::tasks_script.taskList.push_back(oss.str());
    }
    else{
        // If argc is 0, no arg was provided.
        // For now, an empty Renderer is initiated
        // Later on it might be helpful to insert a task like:
        // "env-load ./Resources/Levels/main.json" 
        // Which represents the menue screen of the game
        Nebulite::tasks_script.taskList.push_back(std::string("set-fps 60"));
    }

    //--------------------------------------------------
    // Init general variables from Nebulite namespace, Build main FuncTree
    Nebulite::init();
    Nebulite::init_functions();
    
    //--------------------------------------------------
    // Render loop

    // argc/argv for functioncalls
    int    argc_mainTree = 0;
    char** argv_mainTree = nullptr;

    // At least one loop, to handle taskQueues
    do {
        //--------------------
        // Handle args, parse queue into mainTree and then call internal functions from Nebulite::mainTreeFunctions
        // Currently ignoring return values, plan is a more complex result handling in the future
        (void) Nebulite::resolveTaskQueue(Nebulite::tasks_script,  &Nebulite::tasks_script.waitCounter,&argc_mainTree,&argv_mainTree);
        (void) Nebulite::resolveTaskQueue(Nebulite::tasks_internal,nullptr,                            &argc_mainTree,&argv_mainTree);

        //--------------------
        // Update and render, only if initialized
        // If renderer wasnt initialized, it is still a nullptr
        if (Nebulite::renderer != nullptr && Nebulite::getRenderer()->timeToRender()) {
            Nebulite::getRenderer()->update();          // 1.) Update objects:      Allowing for them to communicate through their invokes
            Nebulite::getRenderer()->renderFrame();     // 2.) Render frame:        Rendering all Container layers from bottom to top
            Nebulite::getRenderer()->renderFPS();       // 3.) Render fps count
            Nebulite::getRenderer()->showFrame();       // 4.) Show Frame
            Nebulite::getRenderer()->clear();           // 5.) Clear screen

            // In order to allow scripting to be more versatile, a wait function was implemented that sets the waitCounter 
            // and halts any following taskQueues for a set amount of frames. This is only necessary for the script tasks, 
            // not for any tasks given from renderobjects as they should never be halted
            // -> After each frame: lower waitCounter in script task
            if(Nebulite::tasks_script.waitCounter>0){ 
                // If renderer doesnt exist, still reduce
                if(Nebulite::renderer == nullptr){
                    Nebulite::tasks_script.waitCounter--; 
                }
                // If that isnt the case: renderer exists: calling getRenderer is possible without accidently creating the renderer
                // Now, just check if the renderer isnt in console mode (essentially freezing all processes)
                else if(!Nebulite::getRenderer()->isConsoleMode()){
                    Nebulite::tasks_script.waitCounter--; 
                }
            }    
        }
    // Continue only if renderer exists, and if quit wasnt called.
    // It might be tempting to add the condition that all tasks are done, 
    // but this could cause issues if the user wishes to quit while a task is still running.
    //
    // A current limitation here that, if the user is running a taskfile with a wait-call, a renderer has to be initialized.
    } while (Nebulite::renderer != nullptr && !Nebulite::getRenderer()->isQuit());


    //--------------------------------------------------
    // Exit

    // Destroy renderer
    if(Nebulite::renderer != nullptr) Nebulite::getRenderer()->destroy();

    // turn off error log
    Nebulite::mainTreeFunctions::errorlog(1,(char*[]){"off"});

    // Exit
    return 0;
}