/*
NEBULITE main

This file is designed to hold the main binary and basic attached function such as: 
- load          | used to load a level json file    | basic version implemented
- attach        | used to attach renderobjects      | not implemented, still in idea-phase

The tests are attached here, but implemented elsewhere due to their size
Later on, the idea is to have the main exe start the Renderer and all other functions to interact with the rendererer globally: 

- user calls main with arguments
- arguments are parsed to start certain scenarios or tests
- these tests/scenaries have arguments themselfes in order to load/delete/etc. 

Currently, load is a function that is called and executed until the user closes the program. 
with this implementation, functions should send the main instructions on what to or be a function for this instruction. As an example: 
- load as instruction: gets Renderer, attaches given level to renderer
- tests like "test benchmark" might add arguments to main in order to load a level or attach certain objects

The point is that, instead of having game loops for every single test case, there is only one game loop left, 
and all test functions simple configure the main game loop.

What instructions on main are needed?
- envload   | load level            | 
- envdeload | deload level          | return to main level, as in titlescreen?
- spawn     | load renderobject     |
- exit      | exit game             |
- save      | save state            |
- load      | load state            |
- task      | load set of tasks     |

The idea with states is that, they hold a copy of files inside ./Resources/ if they were modified.
This enables the usage of savegames: 

./
├── bin
│   └── Nebulite
├── Nebulite.sh
├── States
│   ├── Autosave
│   │   └── Resources
│   ├── Save_2025-05-09
│   │   └── Resources
│   ├── Save_2025-05-10
│   │   └── Resources
│   └── Quicksave
├── Resources
│   ├── Cursor
│   ├── Editor
│   ├── Fonts
│   ├── Invokes
│   ├── Levels
│   ├── Renderobjects
│   └── Sprites
└── Tests.sh

If a file does not exist in that State prefix, use load it from ./Resources directly
*/
#include "mainTreeFunctions.cpp"
#include "Environment.h"
#include "FileManagement.h"
#include "Renderer.h"
#include "FuncTree.h"
#include "TestEnv.h"




// global variables used by main functions

static char* argvBuffer = nullptr;
static int argvCapacity = 0;
//int waitCounter = 0;

void convertStrToArgcArgv(const std::string& cmd, int& argc, char**& argv) {
    // Free previous buffer if any
    if (argvBuffer) {
        delete[] argvBuffer;
        argvBuffer = nullptr;
        argvCapacity = 0;
    }

    // Make a modifiable copy of cmd
    argvCapacity = static_cast<int>(cmd.size()) + 1; // +1 for '\0'
    argvBuffer = new char[argvCapacity];
    std::memcpy(argvBuffer, cmd.c_str(), argvCapacity);

    // Count tokens and split in-place by replacing spaces with '\0'
    argc = 0;
    bool inToken = false;

    // Maximum argv size = number of tokens <= number of chars
    static std::vector<char*> argvVec;
    argvVec.clear();
    argvVec.reserve(argvCapacity);

    for (int i = 0; i < argvCapacity; ++i) {
        if (argvBuffer[i] == ' ' || argvBuffer[i] == '\t') {
            argvBuffer[i] = '\0';
            inToken = false;
        } else if (!inToken) {
            // Start of a new token
            argvVec.push_back(&argvBuffer[i]);
            argc++;
            inToken = true;
        }
    }

    // Null terminate argv list
    argvVec.push_back(nullptr);

    argv = argvVec.data();
}




/*
argv must correspond to one total task like: 
- test json-handler speed
- load level.json 
etc... 

for multiple tasks, the keyword task is used: 
- task file.txt
*/
int main(int argc, char* argv[]) {

    //--------------------------------------------------
    // Startup

    // General Presets
    renderer.setFPS(60);

    // Error logs
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
        // If argc is 0, no arg was provided. Insert a load for test level?
        // TODO...
    }

    // Debug: Output tasks:
    /*
    std::cout << "-------------------" << std::endl;
    std::cout << "Provided tasks are:" << std::endl;
    for(auto task : tasks){
        std::cout << task << std::endl;
    }
    std::cout << std::endl;
    */
    

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
    mainTree.attachFunction(mainTreeFunctions::setResolution,   "set-res",      "Sets resolution size: <w> <h>");
    mainTree.attachFunction(mainTreeFunctions::setCam,          "cam-set",      "Sets Camera position <x> <y>");
    mainTree.attachFunction(mainTreeFunctions::moveCam,         "cam-move",     "Moves Camera position <dx> <dy>");

    // Debug
    mainTree.attachFunction(mainTreeFunctions::serialize,       "serialize",    "Serialize current State to file");
    mainTree.attachFunction(mainTreeFunctions::echo,            "echo",         "Echos all args provided to cout");
    mainTree.attachFunction(mainTreeFunctions::printGlobal,     "print-global", "Prints global doc to cout");
    mainTree.attachFunction(mainTreeFunctions::printState,      "print-state",  "Prints state doc to cout");
    
    

    // Not using testEnv atm
    /*
    TestEnv testEnv;
    mainTree.attachFunction(
        [&testEnv](int argc, char* argv[]) -> int {
            // Call the passArgs method of the TestEnv object
            return testEnv.passArgs(argc, argv);
        }, 
        "test",                         // Command name to call this function
        "Testing Engine Capabilities"   // Description for the help message
    );
    */
    
    
    //--------------------------------------------------
    // Render loop
    int result = 0;
    while (!renderer.isQuit()) {
        //--------------------
        // Handle args
        while (!tasks.empty() && waitCounter == 0) {
            std::string argStr = tasks.front();
            tasks.pop_front();  // remove the used task

            int argc = 0;
            char** argv = nullptr;
            convertStrToArgcArgv(argStr, argc, argv);

            if (argv != nullptr) {
                result = mainTree.parse(argc, argv);
            }
        }
        
        
        // Render Frame
        if (renderer.timeToRender()) {
            renderer.update();          // 1.) Update objects
            renderer.renderFrame();     // 2.) Render frame
            renderer.renderFPS();       // 3.) Render fps count
            renderer.showFrame();       // 4.) Show Frame
            renderer.clear();           // 5.) Clear screen

            // Implementation of a wait function in the scripting task
            if(waitCounter>0) waitCounter--;
        }
    }


    //--------------------------------------------------
    // Exit

    // Store full env for inspection
    FileManagement::WriteFile("lastLevel.log.json",renderer.serialize());

    // Destroy renderer
    renderer.destroy();

    // Explicitly flush std::cerr before closing the file stream
    std::cerr.flush();  // Ensures everything is written to the file

    // Restore the original buffer to std::cerr (important for cleanup)
    std::cerr.rdbuf(originalCerrBuf); 

    // Close the file
    errorFile.close();

    // Exit
    return result;
}

