#include "Environment.h"
#include "FileManagement.h"
#include "Renderer.h"

#include "FuncTree.h"
int foo(int argc, char* argv[]);
int bar(int argc, char* argv[]);


int foo(int argc, char* argv[]){
	FuncTree mainTree;
	mainTree.attachFunction(foo,"foo","does foo");
	mainTree.attachFunction(bar,"bar","does bar");

	std::cout << argv[0] << " called!" << std::endl;

	// List all other provided args
	std::cout << "Rest is:" << std::endl;
	for (int i = 1; i < argc ; i++){
		std::cout << "[arg] " << argv[i] << std::endl;
	}
	std::cout << std::endl;

	return mainTree.parse(argc, argv);
}

int bar(int argc, char* argv[]){
	FuncTree mainTree;
	mainTree.attachFunction(foo,"foo","does foo");
	mainTree.attachFunction(bar,"bar","does bar");

	std::cout << argv[0] << " called!" << std::endl;

	// List all other provided args
	std::cout << "Rest is:" << std::endl;
	for (int i = 1; i < argc ; i++){
		std::cout << "[arg] " << argv[i] << std::endl;
	}
	std::cout << std::endl;

	return mainTree.parse(argc, argv);
}

void createRandomObjects(Renderer& Renderer, int amount, int radius) {

    for (int i = 0; i < (amount > 128 ? 128 : amount); i++) {
        RenderObject Object1;
        int posX = 500;
        int posY = 1000;
        float phi = 2 * 3.141 * (float)rand() / (float)RAND_MAX;
        float rad = rand();
        rad /= RAND_MAX;
        int dX = radius * rad * rad * sin(phi);
        int dY = radius * rad * rad * cos(phi);

        Object1.valueSet(namenKonvention.renderObject.positionX, posX + dX);
        Object1.valueSet(namenKonvention.renderObject.positionY, posY + dY);
        Object1.valueSet(namenKonvention.renderObject.pixelSizeX, 100);
        Object1.valueSet(namenKonvention.renderObject.pixelSizeY, 100);
        Object1.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Sprites/TEST100P/" + std::to_string(1 + i) + ".bmp");

        Renderer.append(Object1);
    }
}


int randomImages() {
    int objectCount = 128;

    //Renderer Object
    Renderer Renderer;  

    //General Variables
    bool quit = false;
    int event = 0;
    while (!quit) {
        if (Renderer.timeToRender()) {
            //Render pipeline
            createRandomObjects(Renderer, objectCount,300);

            //Event handling
            event = Renderer.handleEvent();

            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            //Delete objects
            Renderer.purgeObjects();

            //Analyze event:
            switch (event) {
            case SDL_QUIT:
                quit = true;
                break;
            }
        }
    }
    //End of Program!
    Renderer.destroy();

    return 0;
}

int gameEntry(int argc, char* argv[]){
	std::cout << "Starting normal session..." << std::endl;
	return randomImages();
}

int main(int argc, char* argv[]) {
	std::cout << "argc is " << argc << std::endl;

	FuncTree mainTree;
	mainTree.attachFunction(gameEntry,"session","start normal session");
	mainTree.attachFunction(foo,"foo","does foo");
	mainTree.attachFunction(bar,"bar","does bar");

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

