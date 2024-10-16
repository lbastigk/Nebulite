#include "TestEnv.h"

void TestEnv::_MoveRuleSet::serialize() {
    MoveRuleSet mrs = MoveRuleSet::Examples::Move::linear(3,3,1);
    std::cout << mrs.serialize();
    std::cout << "\n\nPress any key to return.\n";
    Time::wait(500);
    Platform::flushKeyboardInput();
    (void)Platform::getCharacter();
}

void TestEnv::_MoveRuleSet::testMoveSetWithoutThreads() {
    int objCount = 32;
    int ringCount = 16;
    
    //Renderer
    Renderer Renderer;
    //Renderer.setFPS(60);

    //Add objects

    //4 "Squares"
    int pixSize = 1;
    for (int k = 0; k < 4; k++) {
        //with rings
        for (int j = 0; j < ringCount; j++) {
            //made out of little objects
            for (int i = 0; i < objCount; i++) {
                int dX, dY;
                if (k == 0) {
                    dX = -110;
                    dY = -110;
                }
                if (k == 1) {
                    dX = 110;
                    dY = -110;
                }
                if (k == 2) {
                    dX = 110;
                    dY = 110;
                }
                if (k == 3) {
                    dX = -110;
                    dY = 110;
                }

                RenderObject obj;
                obj.valueSet(namenKonvention.renderObject.positionX, 540 - pixSize + dX);
                obj.valueSet(namenKonvention.renderObject.positionY, 540 - pixSize + dY);

                obj.valueSet(namenKonvention.renderObject.pixelSizeX, pixSize);
                obj.valueSet(namenKonvention.renderObject.pixelSizeY, pixSize);

                double col = 128.0 / (4 * ringCount);
                int textureIndex = ((k + 1) * (j + 1) * col);
                if (textureIndex < 1) {
                    textureIndex = 1;
                }
                if (textureIndex > 128) {
                    textureIndex = 128;
                }
                obj.valueSet(namenKonvention.renderObject.imageLocation, namenKonvention.testImages.folder001 + std::to_string(textureIndex) + ".bmp");

                obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionX, 100,(i * 2 * 3.141 / objCount) + k * (3.141 / 4), 1));
                obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionY, 100,(i * 2 * 3.141 / objCount) + (j * 3.141 / 8) + 3.141 / 4, 0.8));
                Renderer.append(obj);
            }
        }
    }

    bool quit = false;
    int event = 0;
    while (!quit) {
        if (Renderer.timeToRender()) {

            //Event handling
            event = Renderer.handleEvent();

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

            Renderer.update();
            //Renderer.update_withThreads();
        }
    }
    //End of Program!
    Renderer.destroy();
}

void TestEnv::_MoveRuleSet::testMoveSetWithThreads() {
    int objCount = 32;
    int ringCount = 16;

    //Renderer
    Renderer Renderer;
    //Renderer.setFPS(60);

    //Add objects

    //4 "Squares"
    int pixSize = 1;
    for (int k = 0; k < 4; k++) {
        //with rings
        for (int j = 0; j < ringCount; j++) {
            //made out of little objects
            for (int i = 0; i < objCount; i++) {
                int dX, dY;
                if (k == 0) {
                    dX = -110;
                    dY = -110;
                }
                if (k == 1) {
                    dX = 110;
                    dY = -110;
                }
                if (k == 2) {
                    dX = 110;
                    dY = 110;
                }
                if (k == 3) {
                    dX = -110;
                    dY = 110;
                }

                RenderObject obj;
                obj.valueSet(namenKonvention.renderObject.positionX, 540 - pixSize + dX);
                obj.valueSet(namenKonvention.renderObject.positionY, 540 - pixSize + dY);

                obj.valueSet(namenKonvention.renderObject.pixelSizeX, pixSize);
                obj.valueSet(namenKonvention.renderObject.pixelSizeY, pixSize);

                double col = 128.0 / (4 * ringCount);
                int textureIndex = ((k + 1) * (j + 1) * col);
                if (textureIndex < 1) {
                    textureIndex = 1;
                }
                if (textureIndex > 128) {
                    textureIndex = 128;
                }
                obj.valueSet(namenKonvention.renderObject.imageLocation, namenKonvention.testImages.folder001 + std::to_string(textureIndex) + ".bmp");

                obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionX, 100,(i * 2 * 3.141 / objCount) + k * (3.141 / 4), 1));
                obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionY, 100,(i * 2 * 3.141 / objCount) + (j * 3.141 / 8) + 3.141 / 4, 0.8));
                Renderer.append(obj);
            }
        }
    }

    bool quit = false;
    int event = 0;
    while (!quit) {
        if (Renderer.timeToRender()) {

            //Event handling
            event = Renderer.handleEvent();

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

            //Renderer.update();
            Renderer.update_withThreads();
        }
    }
    //End of Program!
    Renderer.destroy();
}

void TestEnv::_MoveRuleSet::testSingleObject() {

    //Renderer
    Renderer Renderer;
    Renderer.setFPS(60);

    //Add object with Moveruleset for X and Y
    RenderObject Obj;
    Obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionX, 100, 0, 0.01));
    Obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionY, 100, 0, 0.01));
    Obj.valueSet(namenKonvention.renderObject.positionX, 500);
    Obj.valueSet(namenKonvention.renderObject.positionY, 500);

    

    

    bool quit = false;
    int event = 0;
    while (!quit) {
        if (Renderer.timeToRender()) {
            // Purging all objects from renderer
            Renderer.purgeObjects();

            Obj.update();
            Renderer.append(Obj);

            //Event handling
            event = Renderer.handleEvent();

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

            Renderer.update();
        }
    }
    //End of Program!
    Renderer.destroy();
}
