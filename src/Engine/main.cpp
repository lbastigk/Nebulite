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
- attach    | load renderobject     |
- exit      | exit game             |
- save      | save state            |
- load      | load state            |

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

#include "Environment.h"
#include "FileManagement.h"
#include "Renderer.h"
#include "FuncTree.h"
#include "TestEnv.h"

// load is the main engines function to interact with provided .json level files
// The json file is, for now, assumed to be a level. No precheck is provided.
// The level is loaded into the Renderers environment via deserialize
// A high fps is used for now for performance benchmarks
// The levels last state is saved as lastLevel.log.json for later call with './bin/Nebulite load lastLevel.log.json'
int load(int argc, char* argv[]){
    Renderer Renderer;
    Renderer.setFPS(1000);
    Renderer.deserialize(argv[0]);
    while (!Renderer.isQuit()) {
        if (Renderer.timeToRender()) {
            Renderer.update();          // 1.) Update objects
            Renderer.renderFrame();     // 2.) Render frame
            Renderer.renderFPS();       // 3.) Render fps count
            Renderer.showFrame();       // 4.) Show Frame
            Renderer.clear();           // 5.) Clear screen
        }
    }

    // Store full env for inspection
    FileManagement::WriteFile("lastLevel.log.json",Renderer.serialize());

    //End of Program!
    Renderer.destroy();
    return 0;
}

int main(int argc, char* argv[]) {
    // Remove bin name from arg list
    argc--;
    argv++;

    //--------------------------------------------------
    // Startup
    std::ofstream errorFile("errors.log");
    if (!errorFile) {
        std::cerr << "Failed to open error file." << std::endl;
        return 1;
    }

    // Redirect std::cerr to the file
    std::streambuf* originalCerrBuf = std::cerr.rdbuf(); // Store the original cerr buffer
    std::cerr.rdbuf(errorFile.rdbuf());

    //--------------------------------------------------
    // Build main FuncTree
    FuncTree mainTree("Nebulite");
    mainTree.attachFunction(load, "load", "Load");       // Attaching main game entry function
    TestEnv testEnv;
    mainTree.attachFunction(
        [&testEnv](int argc, char* argv[]) -> int {
            // Call the passArgs method of the TestEnv object
            return testEnv.passArgs(argc, argv);
        }, 
        "test",                         // Command name to call this function
        "Testing Engine Capabilities"   // Description for the help message
    );

    //--------------------------------------------------
    // Process args provided by user
    int result = 0;
    if (argc == 0) {
        // assume normal session
        char* newArgs[2] = {"load", "./Resources/Levels/main.json"};
        result =  mainTree.parse(2, newArgs);
    } else {
        result =  mainTree.parse(argc, argv);
    }

    //--------------------------------------------------
    // Exit

    // Explicitly flush std::cerr before closing the file stream
    std::cerr.flush();  // Ensures everything is written to the file

    // Restore the original buffer to std::cerr (important for cleanup)
    std::cerr.rdbuf(originalCerrBuf); 

    // Close the file
    errorFile.close();

    // Exit
    return result;
}

