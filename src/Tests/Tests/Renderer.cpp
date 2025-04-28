#include "TestEnv.h"

int TestEnv::_Renderer::randomImages(int argc, char* argv[]) {
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
            //event = Renderer.handleEvent();

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

int TestEnv::_Renderer::circleBenchmark(int argc, char* argv[]) {
    int objectCount = 128;

    //Renderer Object
    Renderer Renderer;
    Renderer.setFPS(1000);

    int circ = 800;

    //General Variables
    bool quit = false;
    int event = 0;
    
    int i = 0;
    float radmax = 400;
    float radmin = 50;
    
    //Renderer.updatePosition(0, 0);

    

    while (!quit) {
        if (Renderer.timeToRender()) {
            //Render pipeline
            for (int j = 0; j < 16; j++) {
                float sinA = sin(i / 100.0 + j * 3.141 / 4);

                float rad = (radmax - radmin) / 2 * sinA + radmax - (radmax - radmin) / 2;
                float dR = (radmax - rad);
                float rPerc = (rad - radmin) / (radmax - radmin);
                createCircle001P(Renderer, objectCount * rPerc, rad, (sinA + 1.0) * 127.0 / 2.0, sin(i / 100.0) * dR, cos(i / 100.0) * dR);
                createCircle001P(Renderer, objectCount * rPerc, rad, (sinA + 1.0) * 127.0 / 2.0, sin(i / 100.0) * dR, cos(i / 100.0) * dR);
            }

            //DEBUG
            if (!(i % 100)) {
                std::cout << Renderer.getObjectCount() << " Objects, " << Renderer.getTextureAmount() << " Textures" << "\n";
            }

            //Event handling
            //event = Renderer.handleEvent();

            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            // Final stuff
            Renderer.purgeObjects();

            i++;

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

int TestEnv::_Renderer::ThreadTest(int argc, char* argv[]) {
    int objCount = 1024;
    int ringCount = 16;
    int framesToRender = 30;

    for (int i = 1; i <= objCount* ringCount; i*=2) {
        std::cout << "Objects: " << objCount * ringCount << " with " << i << " per Thread\n";
        auto time = stressTest(objCount, ringCount, i, framesToRender);
        std::cout << "\t took " << time << "ms for "<< framesToRender <<" Frames.\n";
    }
    std::cout << "With no threads: " << stressTest(objCount, ringCount, 16, framesToRender, true) << " ms\n";
    std::cout << "Close the sdl window to continue...";
    Renderer Renderer;
    int event = 0;
    while (event != SDL_QUIT) {
        //Event handling
        //event = Renderer.handleEvent();

        //Render Current instances
        Renderer.renderFrame();
        Renderer.showFrame();
    }
}

int TestEnv::_Renderer::testRendererMemLeak(int argc, char* argv[]) {
    int loopamount = 128;
    bool dump = true;

    if (dump) {
        // Send all reports to STDOUT
        //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        //_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
        //_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        //_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
        //_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        //_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    }

    for (int i = 0; i < loopamount; i++) {
        Renderer Renderer;
        createRandomObjects(Renderer,128,300);
        Renderer.renderFrame();
        Renderer.showFrame();
    }
    if (dump){
        //_CrtDumpMemoryLeaks();
        //Time::wait(10000);
    }
};

int TestEnv::_Renderer::simpleSdlWindow(int argc, char* argv[]){
    std::cout << "Creating a simple SDL window for debugging...\n\n";

    std::cout << "Creating Renderer...\n";
    Renderer Renderer;

    std::cout << "Setting FPS...\n";
    Renderer.setFPS(60);

    //General Variables
    bool quit = false;
    int event = 0;

    while (!quit) {
        //Event handling
        std::cout << "";
        SDL_Event sdlEvent = Renderer.getEventHandle();

        if (sdlEvent.type == SDL_QUIT) {
            quit = true;
        }

        if (Renderer.timeToRender()) {

            //Render Current instances
            std::cout << "Render Current instances...\n";
            Renderer.renderFrame();

            //Render FPS
            std::cout << "Render FPS...\n";
            Renderer.renderFPS();

            // Present the renderer
            std::cout << "Present the renderer...\n";
            Renderer.showFrame();

            //Delete objects
            std::cout << "Delete objects...\n";
            Renderer.purgeObjects();
        }
    }
    //End of Program!
    std::cout << "Destroying Renderer...\n";
    Renderer.destroy();
}

int TestEnv::_Renderer::twoSdlWindows(int argc, char* argv[]){
    std::cout << "Creating two simple SDL window for debugging...\n\n";

    std::cout << "Creating Renderer...\n";
    Renderer Renderer1;
    Renderer Renderer2;

    std::cout << "Setting FPS...\n";
    Renderer1.setFPS(60);
    Renderer2.setFPS(60);

    //General Variables
    bool quit = false;
    int event = 0;

    while (!quit) {
        //Event handling
        std::cout << "";
        SDL_Event sdlEvent1 = Renderer1.getEventHandle();
        SDL_Event sdlEvent2 = Renderer2.getEventHandle();

        RenderObject ro1;
        ro1.valueSet<int>(namenKonvention.renderObject.positionX,200);
        ro1.valueSet<int>(namenKonvention.renderObject.positionY,200);
        ro1.valueSet<int>(namenKonvention.renderObject.pixelSizeX,100);
        ro1.valueSet<int>(namenKonvention.renderObject.pixelSizeY,100);
        RenderObject ro2;
        ro2.valueSet<int>(namenKonvention.renderObject.positionX,400);
        ro2.valueSet<int>(namenKonvention.renderObject.positionY,400);

        if (sdlEvent1.type == SDL_QUIT) {
            quit = true;
        }
        if (sdlEvent2.type == SDL_QUIT) {
            quit = true;
        }

        if (Renderer1.timeToRender()) {

            // Attach Object
            Renderer1.append(ro1);

            //Render Current instances
            std::cout << "1: Render Current instances...\n";
            Renderer1.renderFrame();

            // Render FPS
            std::cout << "1: Render FPS...\n";
            Renderer1.renderFPS();

            // Present the renderer
            std::cout << "1: Present the renderer...\n";
            Renderer1.showFrame();

            // Delete objects
            std::cout << "1: Delete objects...\n";
            Renderer1.purgeObjects();
        }
        if (Renderer2.timeToRender()) {

            // Attach Object
            Renderer2.append(ro2);

            // Render Current instances
            std::cout << "2: Render Current instances...\n";
            Renderer2.renderFrame();

            // Render FPS
            std::cout << "2: Render FPS...\n";
            Renderer2.renderFPS();

            // Present the renderer
            std::cout << "2: Present the renderer...\n";
            Renderer2.showFrame();

            // Delete objects
            std::cout << "2: Delete objects...\n";
            Renderer2.purgeObjects();
        }
    }
    //End of Program!
    std::cout << "Destroying Renderer...\n";
    Renderer1.destroy();
    Renderer2.destroy();
}

//--------------------------------------------------------------------------------
// Private members
void TestEnv::_Renderer::createRandomObjects(Renderer& Renderer, int amount, int radius) {

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

void TestEnv::_Renderer::createCircleOLD(Renderer& Renderer, int amount, int radius) {

    for (int i = 0; i < (amount > 128 ? 128 : amount); i++) {
        RenderObject Object1;

        int posX = 500;
        int posY = 500;
        float phi = 2 * 3.14159 * rand() / RAND_MAX;
        int dX = radius * sin(phi);
        int dY = radius * cos(phi);

        Object1.valueSet(namenKonvention.renderObject.positionX, posX + dX);
        Object1.valueSet(namenKonvention.renderObject.positionY, posY + dY);
        Object1.valueSet(namenKonvention.renderObject.pixelSizeX, 10);
        Object1.valueSet(namenKonvention.renderObject.pixelSizeY, 10);
        Object1.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Sprites/TEST1P/" + std::to_string(1 + i) + ".bmp");

        Renderer.append(Object1);
    }
}

void TestEnv::_Renderer::createCircle001P(Renderer& Renderer, int amount, int radius, int colour128, int _dX, int _dY) {

    for (int i = 0; i < (amount > 128 ? 128 : amount); i++) {
        RenderObject Object1;

        int posX = 500;
        int posY = 500;
        float phi = 2 * 3.14159 * rand() / RAND_MAX;
        int dX = radius * sin(phi);
        int dY = radius * cos(phi);

        Object1.valueSet(namenKonvention.renderObject.positionX, posX + dX + _dX);
        Object1.valueSet(namenKonvention.renderObject.positionY, posY + dY + _dY);
        Object1.valueSet(namenKonvention.renderObject.pixelSizeX, 10);
        Object1.valueSet(namenKonvention.renderObject.pixelSizeY, 10);
        Object1.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Sprites/TEST001P/" + std::to_string(colour128 + 1) + ".bmp");

        Renderer.append(Object1);
    }
}

void TestEnv::_Renderer::createCircle100P(Renderer& Renderer, int amount, int radius, int colour128, int _dX, int _dY) {

    for (int i = 0; i < (amount > 128 ? 128 : amount); i++) {
        RenderObject Object1;

        int posX = 500;
        int posY = 500;
        float phi = 2 * 3.14159 * rand() / RAND_MAX;
        int dX = radius * sin(phi);
        int dY = radius * cos(phi);

        Object1.valueSet(namenKonvention.renderObject.positionX, posX + dX + _dX);
        Object1.valueSet(namenKonvention.renderObject.positionY, posY + dY + _dY);
        Object1.valueSet(namenKonvention.renderObject.pixelSizeX, 10);
        Object1.valueSet(namenKonvention.renderObject.pixelSizeY, 10);
        Object1.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Sprites/TEST100P/" + std::to_string(colour128 + 1) + ".bmp");

        Renderer.append(Object1);
    }
}

//Returns render time for x frames
UINT64 TestEnv::_Renderer::stressTest(int objCount, int ringCount, int threadSize, unsigned int framesToRender, bool bypassThreads) {
    //Renderer
    Renderer Renderer;
    Renderer.setThreadSize(threadSize);
    //Renderer.setFPS(60);

    //Add objects
    for (int j = 0; j < 8; j++) {
        for (int i = 0; i < objCount; i++) {
            RenderObject obj;
            obj.valueSet(namenKonvention.renderObject.positionX, 540);
            obj.valueSet(namenKonvention.renderObject.positionY, 540);

            obj.valueSet(namenKonvention.renderObject.pixelSizeX, 5);
            obj.valueSet(namenKonvention.renderObject.pixelSizeY, 5);

            obj.valueSet(namenKonvention.renderObject.imageLocation, namenKonvention.testImages.folder100 + std::to_string(5 + 6 * j) + ".bmp");
            Renderer.append(obj);
        }
    }

    unsigned int totalFrames = 0;
    UINT64 time = Time::gettime();

    bool quit = false;
    int event = 0;
    while (!quit && totalFrames < framesToRender) {
        if (Renderer.timeToRender()) {

            //Event handling
            //event = Renderer.handleEvent();

            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            //Analyze event:
            switch (event) {
            case SDL_QUIT:
                quit = true;
                break;
            }

            if (bypassThreads) {
                Renderer.update();
            }
            else {
                Renderer.update_withThreads();
            }


            totalFrames++;
        }
    }
    //End of Program!
    Renderer.destroy();

    return Time::gettime() - time;
}
