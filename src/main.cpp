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
 *
 */


/*
 * Initializes callable functions from both user CLI and runtime environment.
 * Also sets up the global Renderer used across Tree-based function calls.
 */
#include "mainTreeFunctions.h" 

// Used to build a tree of callable Functions and parsing arguments
#include "FuncTree.h"



/*
 * NEBULITE main
 * 
 * 
 * TODO:    Make error log an on/off toggle via an additional mainTree function
 *          log on
 *          log off
 * 
 * TODO:    Current implementation of result return only returns the result of the last maintree parse
 *          Perhaps the result should be used to determine when to stop the program as well, 
 *          instead of just checking Renderer::isQuit()
 *          E.g. close program if env-load is called on a non-existing file       
 */
int main(int argc, char* argv[]) {
    //--------------------------------------------------
    // Startup

    // Log errors in separate file
    std::ofstream errorFile("errors.log");
    if (!errorFile) {
        std::cerr << "Failed to open error file." << std::endl;
        return 1;
    }
    std::streambuf* originalCerrBuf = std::cerr.rdbuf(); // Store the original cerr buffer
    std::cerr.rdbuf(errorFile.rdbuf());
    
    //--------------------------------------------------
    // args handling

    // Remove bin name from arg list
    argc--;
    argv++;

    // add main arg to argTokens:
    if(argc > 0){
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
        // Which represents the menue screen
    }
    

    //--------------------------------------------------
    // Build main FuncTree

    FuncTree mainTree("Nebulite");

    // General
    mainTree.attachFunction(Nebulite::mainTreeFunctions::setGlobal,       "set-global",   "Set any global variable: [key] [value]");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::envload,         "env-load",     "Loads an environment");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::envdeload,       "env-deload",   "Deloads an environment");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::spawn,           "spawn",        "Spawn a renderobject");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::exitProgram,     "exit",         "exits the program");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::save,            "state-save",   "Saves the state");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::load,            "state-load",   "Loads a state");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::loadTaskList,    "task",         "Loads a txt file of tasks");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::wait,            "wait",         "Halt all commands for a set amount of frames");
    
    // Renderer Settings
    mainTree.attachFunction(Nebulite::mainTreeFunctions::setFPS,          "set-fps",      "Sets FPS to an integer between 1 and 10000. 60 if no arg is provided");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::setResolution,   "set-res",      "Sets resolution size: [w] [h]");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::setCam,          "cam-set",      "Sets Camera position [x] [y] <c>");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::moveCam,         "cam-move",     "Moves Camera position [dx] [dy]");

    // Debug
    mainTree.attachFunction(Nebulite::mainTreeFunctions::serialize,       "serialize",    "Serialize current State to file");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::echo,            "echo",         "Echos all args provided to cout");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::printGlobal,     "print-global", "Prints global doc to cout");
    mainTree.attachFunction(Nebulite::mainTreeFunctions::printState,      "print-state",  "Prints state doc to cout");
    
    //--------------------------------------------------
    // Render loop
    int    argc_mainTree = 0;
    char** argv_mainTree = nullptr;
    int result = 0;
    do {
        //--------------------
        // Handle args
        while (!Nebulite::tasks_script.taskList.empty() && Nebulite::tasks_script.waitCounter == 0) {
            // Get task
            std::string argStr = Nebulite::tasks_script.taskList.front();
            Nebulite::tasks_script.taskList.pop_front();  // remove the used task

            // Resolve global vars in task
            argStr = Nebulite::invoke.resolveGlobalVars(argStr);

            // Convert std::string to argc,argv
            argc_mainTree = 0;
            argv_mainTree = nullptr;
            mainTree.convertStrToArgcArgv(argStr, argc_mainTree, argv_mainTree);

            if (argv != nullptr) {
                result = mainTree.parse(argc_mainTree, argv_mainTree);
            }
        }
        while (!Nebulite::tasks_internal.taskList.empty()) {
            // Get task
            std::string argStr = Nebulite::tasks_internal.taskList.front();
            Nebulite::tasks_internal.taskList.pop_front();  // remove the used task

            // Resolve global vars in task
            argStr = Nebulite::invoke.resolveGlobalVars(argStr);

            // Convert std::string to argc,argv
            argc_mainTree = 0;
            argv_mainTree = nullptr;
            mainTree.convertStrToArgcArgv(argStr, argc_mainTree, argv_mainTree);

            if (argv != nullptr) {
                result = mainTree.parse(argc_mainTree, argv_mainTree);
            }
        }
        
        //--------------------
        // Update and render
        if (Nebulite::renderer != nullptr && Nebulite::getRenderer()->timeToRender()) {
            Nebulite::getRenderer()->update();          // 1.) Update objects
            Nebulite::getRenderer()->renderFrame();     // 2.) Render frame
            Nebulite::getRenderer()->renderFPS();       // 3.) Render fps count
            Nebulite::getRenderer()->showFrame();       // 4.) Show Frame
            Nebulite::getRenderer()->clear();           // 5.) Clear screen

            // lower waitCounter in script task
            if(Nebulite::tasks_script.waitCounter>0) Nebulite::tasks_script.waitCounter--; 
        }
    } while (Nebulite::renderer != nullptr && !Nebulite::getRenderer()->isQuit());


    //--------------------------------------------------
    // Exit

    // Destroy renderer
    if(Nebulite::renderer != nullptr) Nebulite::getRenderer()->destroy();

    // Close error log
    std::cerr.flush();                  // Explicitly flush std::cerr before closing the file stream. Ensures everything is written to the file
    std::cerr.rdbuf(originalCerrBuf);   // Restore the original buffer to std::cerr (important for cleanup)
    errorFile.close();

    // Exit
    return result;
}