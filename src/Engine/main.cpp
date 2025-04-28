#include "Environment.h"
#include "FileManagement.h"
#include "Renderer.h"
#include "FuncTree.h"
#include "TestEnv.h"

int gameEntry(int argc, char* argv[]){
    std::cerr << "Loading level: " << argv[1] << std::endl;
    Renderer Renderer;
    Renderer.setFPS(1000);
    Renderer.deserializeEnvironment(argv[1]);
    
    // Main Render loop
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
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();
        }
    }

    // Store last global values
    FileManagement::WriteFile("lastGlobalValues.json",Renderer.serializeGlobal());

    //End of Program!
    Renderer.destroy();
    return 0;
}

int main(int argc, char* argv[]) {
    std::ofstream errorFile("errors.log");
    if (!errorFile) {
        std::cerr << "Failed to open error file." << std::endl;
        return 1;
    }

    // Redirect cerr to the file
    std::cerr.rdbuf(errorFile.rdbuf());
    //std::cerr.rdbuf(std::cout.rdbuf());  // Redirect cerr to cout

    // Environments:
    TestEnv TestEnv;

    // Build main tree
	FuncTree mainTree("Nebulite");
	mainTree.attachFunction(gameEntry,"load","Load");
    mainTree.attachFunction(
        [&TestEnv](int argc, char* argv[]) -> int {
            // Call the passArgs method of the TestEnv object
            return TestEnv.passArgs(argc, argv);
        }, 
        "test",                         // Command name to call this function
        "Testing Engine Capabilities"   // Description for the help message
    );

    // Process args
	if (argc == 1){
		// assume normal session
        std::cerr << "Starting normal session" << std::endl;
		char* newArgs[3] = {"","load", "./Resources/Levels/main.json"};
		return mainTree.parse(3, newArgs);
	}
	else{
		return mainTree.parse(argc, argv);
	}
    return 0;
}

