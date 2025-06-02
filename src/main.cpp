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

// Resolves a given taskqueue by parsing each line into argc/argv and calling the mainTree on the arguments
int resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree){
    int result = 0;
    while (!tq.taskList.empty() && (counter == nullptr || *counter == 0)) {
        // Get task
        std::string argStr = tq.taskList.front();
        tq.taskList.pop_front();  // remove the used task

        // Convert std::string to argc,argv
        *argc_mainTree = 0;
        *argv_mainTree = nullptr;
        Nebulite::convertStrToArgcArgv(argStr, *argc_mainTree, *argv_mainTree);

        if (*argv_mainTree != nullptr && argStr.size()) {
            result = Nebulite::mainTree.parse(*argc_mainTree, *argv_mainTree);
        }
        else{
            result = 0;
        }
    }
    return result;
}


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
    // Init general variables from Nebulite namespace
    Nebulite::init();

    //--------------------------------------------------
    // Build main FuncTree
    Nebulite::init_functions();
    
    //--------------------------------------------------
    // Render loop
    int    argc_mainTree = 0;
    char** argv_mainTree = nullptr;
    int result = 0;

    // At least one loop, to handle taskQueues
    do {
        //--------------------
        // Handle args
        result = resolveTaskQueue(Nebulite::tasks_script,  &Nebulite::tasks_script.waitCounter,&argc_mainTree,&argv_mainTree);
        result = resolveTaskQueue(Nebulite::tasks_internal,nullptr,                            &argc_mainTree,&argv_mainTree);

        //--------------------
        // Update and render, only if initialized
        if (Nebulite::renderer != nullptr && Nebulite::getRenderer()->timeToRender()) {
            Nebulite::getRenderer()->update();          // 1.) Update objects
            Nebulite::getRenderer()->renderFrame();     // 2.) Render frame
            Nebulite::getRenderer()->renderFPS();       // 3.) Render fps count
            Nebulite::getRenderer()->showFrame();       // 4.) Show Frame
            Nebulite::getRenderer()->clear();           // 5.) Clear screen

            // lower waitCounter in script task
            if(Nebulite::tasks_script.waitCounter>0) Nebulite::tasks_script.waitCounter--; 
        
            // DEBUG: serialize doc
            /*
            Platform::clearScreen();
            std::cout << JSONHandler::serialize(*Nebulite::global);
            //*/
            
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