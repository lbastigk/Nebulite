#include "Environment.h"
#include "FileManagement.h"
#include "Renderer.h"
#include "FuncTree.h"
#include "TestEnv.h"

int gameEntry(int argc, char* argv[]){
    Renderer Renderer;
    Renderer.setFPS(1000);
    Renderer.deserializeEnvironment(argv[0]);

    

    while (!Renderer.isQuit()) {
        // for now ,see max fps
        //if (true) {
        if (Renderer.timeToRender()) {
            // Update
            Renderer.update();  // Updates Renderer:
                                // - update invoke
                                // - Update Renderobjects
                                // - Each RO is checked against invokes
                                // - draw ROs new position

            //Render Current instances
            Renderer.renderFrame();     // No memory leaks here!

            //Render FPS
            Renderer.renderFPS();       // No memory leaks here!

            // Present the renderer
            Renderer.showFrame();

        }
    }

    // Store last global values
    FileManagement::WriteFile("lastGlobalValues.log.json",Renderer.serializeGlobal());

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
    // Build main tree
    FuncTree mainTree("Nebulite");
    mainTree.attachFunction(gameEntry, "load", "Load");       // Attaching main game entry function
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
    // Process args
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

