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
#include "mainTreeFunctions.cpp" 

// Used to build a tree of callable Functions and parsing arguments
#include "FuncTree.h"



/*
 * NEBULITE main
 * 
 * TODO:    Only init renderer if it's actually used!
 *          This way, help does not start a whole Renderer...
 * 
 * TODO:    Make error log an on/off toggle via an additional mainTree function
 *          log on
 *          log off
 * 
 * TODO:    New Implementation wait
 *          Good approach would be to have 2 different task-objects
 *          perhaps:
 *          struct task{
 *              std::deque<std::string> tasks;
 *              uint64_t waitCounter;
 *          };
 *          reason is that the wait from a script shouldnt halt the 
 *          general tasks being given by renderobjects
 *          This way, there a 2 tasklists: 
 *          - The script tasks
 *          - The 'gameloop' tasks
 *          Scripting tasklist should use wait
 *          Gameloop tasks should not, as this halts all tasks for a set amount of frames
 *          Though, removing the capability might not be worth it, perhaps there is a usecase somewhere
 * 
 * TODO:    Current implementation of result return only returns the result of the last maintree parse
 *          Perhaps the result should be used to determine when to stop the program as well, 
 *          instead of just checking Renderer::isQuit()
 *          E.g. close program if env-load is called on a non-existing file       
 */
int main(int argc, char* argv[]) {
    //--------------------------------------------------
    // Startup

    // General Presets
    renderer.setFPS(60);

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
        tasks.push_back(oss.str());
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
    mainTree.attachFunction(mainTreeFunctions::envload,         "env-load",     "Loads an environment");
    mainTree.attachFunction(mainTreeFunctions::envdeload,       "env-deload",   "Deloads an environment");
    mainTree.attachFunction(mainTreeFunctions::spawn,           "spawn",        "Spawn a renderobject");
    mainTree.attachFunction(mainTreeFunctions::exitProgram,     "exit",         "exits the program");
    mainTree.attachFunction(mainTreeFunctions::save,            "state-save",   "Saves the state");
    mainTree.attachFunction(mainTreeFunctions::load,            "state-load",   "Loads a state");
    mainTree.attachFunction(mainTreeFunctions::loadTaskList,    "task",         "Loads a txt file of tasks");
    mainTree.attachFunction(mainTreeFunctions::wait,            "wait",         "Halt all commands for a set amount of frames");
    
    // Renderer Settings
    mainTree.attachFunction(mainTreeFunctions::setFPS,          "set-fps",      "Sets FPS to an integer between 1 and 10000. 60 if no arg is provided");
    mainTree.attachFunction(mainTreeFunctions::setResolution,   "set-res",      "Sets resolution size: [w] [h]");
    mainTree.attachFunction(mainTreeFunctions::setCam,          "cam-set",      "Sets Camera position [x] [y] <c>");
    mainTree.attachFunction(mainTreeFunctions::moveCam,         "cam-move",     "Moves Camera position [dx] [dy]");

    // Debug
    mainTree.attachFunction(mainTreeFunctions::serialize,       "serialize",    "Serialize current State to file");
    mainTree.attachFunction(mainTreeFunctions::echo,            "echo",         "Echos all args provided to cout");
    mainTree.attachFunction(mainTreeFunctions::printGlobal,     "print-global", "Prints global doc to cout");
    mainTree.attachFunction(mainTreeFunctions::printState,      "print-state",  "Prints state doc to cout");
    
    //--------------------------------------------------
    // Render loop
    int    argc_mainTree = 0;
    char** argv_mainTree = nullptr;
    int result = 0;
    while (!renderer.isQuit()) {
        //--------------------
        // Handle args
        while (!tasks.empty() && waitCounter == 0) {
            // Get task
            std::string argStr = tasks.front();
            tasks.pop_front();  // remove the used task

            // Resolve global vars in task
            argStr = renderer.getInvoke()->resolveGlobalVars(argStr);

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
        if (renderer.timeToRender()) {
            renderer.update();          // 1.) Update objects
            renderer.renderFrame();     // 2.) Render frame
            renderer.renderFPS();       // 3.) Render fps count
            renderer.showFrame();       // 4.) Show Frame
            renderer.clear();           // 5.) Clear screen

            
            if(waitCounter>0) waitCounter--;
        }
    }


    //--------------------------------------------------
    // Exit

    // Destroy renderer
    renderer.destroy();

    // Close error log
    std::cerr.flush();                  // Explicitly flush std::cerr before closing the file stream. Ensures everything is written to the file
    std::cerr.rdbuf(originalCerrBuf);   // Restore the original buffer to std::cerr (important for cleanup)
    errorFile.close();

    // Exit
    return result;
}