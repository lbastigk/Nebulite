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

                                up:     global invoke commands

*/

// -----------------------------------
// Includes

/* Include the Nebulite Namespace with all its functions
 * Initializes callable functions from both user CLI and runtime environment.
 * Also sets up the global Renderer used across Tree-based function calls.
 */
#include "Nebulite.h" 

// Used to build a tree of callable Functions and parsing arguments
#include "FuncTree.h"



// -----------------------------------
// NEBULITE main
/*
 * 
 * TODO:    Current implementation of result return only returns the result of the last maintree parse
 *          Perhaps the result should be used to determine when to stop the program as well, 
 *          instead of just checking Renderer::isQuit()
 *          E.g. close program if env-load is called on a non-existing file
 *          Idea here is to pass some sort of Nebulite::ErrorParse Struct that contains all errors and from what function in some vector 
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
    //
    // Holds functions that are called through the invoke class through functioncalls
    // These are appended to the Nebulite::taskque: Nebulite::tasks_internal 
    // and then parsed like any usual argc/argv from the main would be to call functions in Nebulite::mainTreeFunctions
    // Allowing renderobjects to:
    // - spawn another renderobject
    // - echo a status to cout
    // - echo an error to cerr
    // - append additional tasks
    // - exit the programm
    // on successful interaction through an invoke
    //
    // This allows the main loop and the internal renderer to communicate
    // - Renderobjects send functioncalls
    // - The user itself could, for instance, spawn an temporary renderobject at with an invoke that says:
    //   "hey if you are at my position, set your deleteFlag to true so the renderer destroys you on the next update"
    //   or:
    //   "if you have the attribute xyz, set yourself to my position"
    // This allows for more dynamic interactions without having to communicate each renderobjects position to the user
    // as these would usually need more complicated, hardcoded solutions of getting and searching the internal RenderObjectContainer
    //
    // MainTreeFunctions are easily implemented by:
    //  - defining them in the Nebulite.h/.cpp file
    //  - adding them to the mainTree 
    // Example:
    //
    // // Definition
    // int Nebulite::mainTreeFunctions::foo(int argc, char** argv){std::cout << "bar" << std::endl;};
    // // Add to mainTree:     Function                               Name    Description
    // mainTree.attachFunction(Nebulite::mainTreeFunctions::foo,      "foo",  "Prints foo to cout");
    // Then, calling ./bin/Nebulite help will include the entry:
    // foo - Prints foo to cout
    // And calling "./bin/Nebulite foo" prints bar to cout
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
    // so if the user is running a task with wait, meaning to all tasks are finished in the first loop, a renderer has to be initialized.
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