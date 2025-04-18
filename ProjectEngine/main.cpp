#include "Environment.h"
#include "FileManagement.h"
#include "Renderer.h"
#include "FuncTree.h"
#include "TestEnv.h"

int gameEntry(int argc, char* argv[]){
	std::cout << "Starting normal session..." << std::endl;
	
    

    Renderer Renderer(false,4,160,160);

    Renderer.setFPS(60);
    Renderer.deserializeEnvironment("./Resources/Levels/example.json");
    

    //General Variables
    bool quit = false;
    int event = 0;
    

    while (!quit) {
        //Event handling
        SDL_Event sdlEvent = Renderer.getEventHandle();

        if (sdlEvent.type == SDL_QUIT) {
            quit = true;
        }

        if (Renderer.timeToRender()) {
            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            //Delete objects
            Renderer.update();
        }
    }
    //End of Program!
    std::cout << "Destroying Renderer...\n";
    Renderer.destroy();
    return 0;
}

int main(int argc, char* argv[]) {
    std::cerr.rdbuf(std::cout.rdbuf());  // Redirect cerr to cout

    // Environments:
    TestEnv TestEnv;

    // Build main tree
	FuncTree mainTree("Nebulite");
	mainTree.attachFunction(gameEntry,"session","start normal session");
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
		const char* defaultArgs[] = {argv[0], "session"};
		char* newArgs[2] = {"unknown", "session"};
		return mainTree.parse(2, newArgs);
	}
	else{
		return mainTree.parse(argc, argv);
	}
}

