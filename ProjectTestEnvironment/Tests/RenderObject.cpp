#include "TestEnv.h"


void TestEnv::_RenderObject::testSpriteSheets() {
    Renderer Renderer;
    RenderObject obj1;
    obj1.valueSet(namenKonvention.renderObject.positionX, 200);
    obj1.valueSet(namenKonvention.renderObject.positionY, 200);
    obj1.valueSet(namenKonvention.renderObject.pixelSizeX, 100);
    obj1.valueSet(namenKonvention.renderObject.pixelSizeY, 100);
    obj1.valueSet(namenKonvention.renderObject.isSpritesheet, false);
    obj1.valueSet(namenKonvention.renderObject.spritesheetOffsetX, 0);
    obj1.valueSet(namenKonvention.renderObject.spritesheetOffsetY, 0);

    RenderObject obj2;
    obj2.valueSet(namenKonvention.renderObject.positionX, 400);
    obj2.valueSet(namenKonvention.renderObject.positionY, 400);
    obj2.valueSet(namenKonvention.renderObject.pixelSizeX, 200);
    obj2.valueSet(namenKonvention.renderObject.pixelSizeY, 200);

    obj2.valueSet(namenKonvention.renderObject.isSpritesheet, true);
    obj2.valueSet(namenKonvention.renderObject.spritesheetOffsetX, 133);
    obj2.valueSet(namenKonvention.renderObject.spritesheetOffsetY, 133);
    obj2.valueSet(namenKonvention.renderObject.spritesheetSizeX, 200);
    obj2.valueSet(namenKonvention.renderObject.spritesheetSizeY, 200);

    obj2.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.spritesheetOffsetX, 100, 3.14 / 2, 0.1));
    obj2.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.spritesheetOffsetY, 100, 0, 0.1));
    
    obj2.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.spritesheetSizeX, 50, 0, 1));
    obj2.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.spritesheetSizeY, 50, 0, 1));

    std::cout << obj2.serialize();

    Renderer.append(obj1);
    Renderer.append(obj2);


    Renderer.setFPS(30);

    //General Variables
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

            // Update objects
            Renderer.update();

            //Analyze event:
            switch (event) {
            case SDL_QUIT:
                quit = true;
                break;
            }
            
            Platform::clearScreen();
            std::cout << Renderer.serializeEnvironment();
        }
    }
    //End of Program!
    Renderer.destroy();
}

// First test for spritesheet based rendering, using pokemon sprite animations as a basis
void TestEnv::_RenderObject::testPokemonSprites() {
    Renderer Renderer;
    RenderObject Player;
    RenderObject background;

    //Background
    background.valueSet(namenKonvention.renderObject.layer,(int)Environment::RenderObjectLayers::background);
    background.valueSet(namenKonvention.renderObject.positionX, 0);
    background.valueSet(namenKonvention.renderObject.positionY, 0);
    background.valueSet(namenKonvention.renderObject.pixelSizeX, 4770);
    background.valueSet(namenKonvention.renderObject.pixelSizeY, 8000);
    background.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Sprites/Pokemon_Transparent/Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 2.png");
    background.valueSet(namenKonvention.renderObject.isSpritesheet, false);
    background.update();

    //Standard stuff
    Player.valueSet(namenKonvention.renderObject.layer,(int)Environment::RenderObjectLayers::foreground);
    Player.valueSet(namenKonvention.renderObject.positionX, 200);
    Player.valueSet(namenKonvention.renderObject.positionY, 200);
    Player.valueSet(namenKonvention.renderObject.pixelSizeX, 160);
    Player.valueSet(namenKonvention.renderObject.pixelSizeY, 240);

    //Spritesheet, standard values
    Player.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Sprites/Pokemon_Transparent/Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png");
    Player.valueSet(namenKonvention.renderObject.isSpritesheet, true);
    Player.valueSet(namenKonvention.renderObject.spritesheetOffsetX, 9+17);
    Player.valueSet(namenKonvention.renderObject.spritesheetOffsetY, 42);
    Player.valueSet(namenKonvention.renderObject.spritesheetSizeX, 16);
    Player.valueSet(namenKonvention.renderObject.spritesheetSizeY, 24);

    Renderer.setFPS(30);

    MoveRuleSet startValueOffsetX;
    MoveRuleSet startValueOffsetY;
    MoveRuleSet spriteOffsetX;
    MoveRuleSet pos;

    int i = 0;
    int startSpritePosX,startSpritePosY,vecX,vecY;

    //General Variables
    SDL_Event sdlEvent;
    while (sdlEvent.type != SDL_QUIT) {

        //Event handling
        sdlEvent = Renderer.getEventHandle();

        if (Renderer.timeToRender()) {

            // Get the current state of all keys
            const Uint8* keystates = SDL_GetKeyboardState(NULL);

            // add movement depending on keydown, and only if player isnt already moving

            // Check if a specific key is currently pressed (e.g., space key)

            //dummy renderobject to check if player is standing
            RenderObject dummy = Player;
            dummy.update();
            if(0 && !dummy.hasMoveSet()){
                if (keystates[SDL_SCANCODE_W]) {
                    //Move Up
                    //------------------------------------------
                    startSpritePosX = 9 + 1*3*17;
                    startSpritePosY = 42;
                    vecX = 0;
                    vecY = -1;

                    //Set start sprite
                    startValueOffsetX = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetX, startSpritePosX);
                    startValueOffsetY = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetY, startSpritePosY);

                    //Sprite Movement
                    spriteOffsetX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, 1,3,17);

                    //Object Movement
                    pos = MoveRuleSet::Examples::Move::linear(vecX*160,vecY*160,10,10);

                    //Load
                    Player.update();
                    Player.loadMoveSet(startValueOffsetX);
                    Player.loadMoveSet(startValueOffsetY);
                    Player.loadMoveSet(spriteOffsetX);
                    Player.loadMoveSet(pos);
                }
                else if (keystates[SDL_SCANCODE_A]) {
                    //Move Left
                    //------------------------------------------
                    startSpritePosX = 9 + 2*3*17;
                    startSpritePosY = 42;
                    vecX = -1;
                    vecY = 0;

                    //Set start sprite
                    startValueOffsetX = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetX, startSpritePosX);
                    startValueOffsetY = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetY, startSpritePosY);

                    //Sprite Movement
                    spriteOffsetX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, 1,3,17);

                    //Object Movement
                    pos = MoveRuleSet::Examples::Move::linear(vecX*160,vecY*160,10,10);

                    //Load
                    Player.update();
                    Player.loadMoveSet(startValueOffsetX);
                    Player.loadMoveSet(startValueOffsetY);
                    Player.loadMoveSet(spriteOffsetX);
                    Player.loadMoveSet(pos);
                }
                else if (keystates[SDL_SCANCODE_S]) {
                    //Move Down
                    //------------------------------------------
                    startSpritePosX = 9 + 0*3*17;
                    startSpritePosY = 42;
                    vecX = 0;
                    vecY = 1;

                    //Set start sprite
                    startValueOffsetX = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetX, startSpritePosX);
                    startValueOffsetY = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetY, startSpritePosY);

                    //Sprite Movement
                    spriteOffsetX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, 1,3,17);

                    //Object Movement
                    pos = MoveRuleSet::Examples::Move::linear(vecX*160,vecY*160,10,10);

                    //Load
                    Player.update();
                    Player.loadMoveSet(startValueOffsetX);
                    Player.loadMoveSet(startValueOffsetY);
                    Player.loadMoveSet(spriteOffsetX);
                    Player.loadMoveSet(pos);                    
                }
                else if (keystates[SDL_SCANCODE_D]) {
                    //Move Right
                    //------------------------------------------
                    startSpritePosX = 9 + 3*3*17;
                    startSpritePosY = 42;
                    vecX = 1;
                    vecY = 0;

                    //Set start sprite
                    startValueOffsetX = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetX, startSpritePosX);
                    startValueOffsetY = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetY, startSpritePosY);

                    //Sprite Movement
                    spriteOffsetX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, 1,3,17);

                    //Object Movement
                    pos = MoveRuleSet::Examples::Move::linear(vecX*160,vecY*160,10,10);

                    //Load
                    Player.update();
                    Player.loadMoveSet(startValueOffsetX);
                    Player.loadMoveSet(startValueOffsetY);
                    Player.loadMoveSet(spriteOffsetX);
                    Player.loadMoveSet(pos);                    
                }
            }
            if(!dummy.hasMoveSet()){
                int choice = int(10*(((double)rand())/RAND_MAX));
                if (choice == 0) {
                    //Move Up
                    //------------------------------------------
                    startSpritePosX = 9 + 1*3*17;
                    startSpritePosY = 42;
                    vecX = 0;
                    vecY = -1;

                    //Set start sprite
                    startValueOffsetX = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetX, startSpritePosX);
                    startValueOffsetY = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetY, startSpritePosY);

                    //Sprite Movement
                    spriteOffsetX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, 1,3,17);

                    //Object Movement
                    pos = MoveRuleSet::Examples::Move::linear(vecX*160,vecY*160,10,10);

                    //Load
                    Player.update();
                    Player.loadMoveSet(startValueOffsetX);
                    Player.loadMoveSet(startValueOffsetY);
                    Player.loadMoveSet(spriteOffsetX);
                    Player.loadMoveSet(pos);
                }
                else if (choice == 1) {
                    //Move Left
                    //------------------------------------------
                    startSpritePosX = 9 + 2*3*17;
                    startSpritePosY = 42;
                    vecX = -1;
                    vecY = 0;

                    //Set start sprite
                    startValueOffsetX = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetX, startSpritePosX);
                    startValueOffsetY = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetY, startSpritePosY);

                    //Sprite Movement
                    spriteOffsetX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, 1,3,17);

                    //Object Movement
                    pos = MoveRuleSet::Examples::Move::linear(vecX*160,vecY*160,10,10);

                    //Load
                    Player.update();
                    Player.loadMoveSet(startValueOffsetX);
                    Player.loadMoveSet(startValueOffsetY);
                    Player.loadMoveSet(spriteOffsetX);
                    Player.loadMoveSet(pos);
                }
                else if (choice == 2) {
                    //Move Down
                    //------------------------------------------
                    startSpritePosX = 9 + 0*3*17;
                    startSpritePosY = 42;
                    vecX = 0;
                    vecY = 1;

                    //Set start sprite
                    startValueOffsetX = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetX, startSpritePosX);
                    startValueOffsetY = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetY, startSpritePosY);

                    //Sprite Movement
                    spriteOffsetX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, 1,3,17);

                    //Object Movement
                    pos = MoveRuleSet::Examples::Move::linear(vecX*160,vecY*160,10,10);

                    //Load
                    Player.update();
                    Player.loadMoveSet(startValueOffsetX);
                    Player.loadMoveSet(startValueOffsetY);
                    Player.loadMoveSet(spriteOffsetX);
                    Player.loadMoveSet(pos);                    
                }
                else if (choice == 3) {
                    //Move Right
                    //------------------------------------------
                    startSpritePosX = 9 + 3*3*17;
                    startSpritePosY = 42;
                    vecX = 1;
                    vecY = 0;

                    //Set start sprite
                    startValueOffsetX = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetX, startSpritePosX);
                    startValueOffsetY = MoveRuleSet::Examples::setValue(namenKonvention.renderObject.spritesheetOffsetY, startSpritePosY);

                    //Sprite Movement
                    spriteOffsetX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, 1,3,17);

                    //Object Movement
                    pos = MoveRuleSet::Examples::Move::linear(vecX*160,vecY*160,10,10);

                    //Load
                    Player.update();
                    Player.loadMoveSet(startValueOffsetX);
                    Player.loadMoveSet(startValueOffsetY);
                    Player.loadMoveSet(spriteOffsetX);
                    Player.loadMoveSet(pos);                    
                }
            }

            // Set Render Position
            int playerPosX = (int)Player.valueGet<double>(namenKonvention.renderObject.positionX) + (int)(Player.valueGet<double>(namenKonvention.renderObject.pixelSizeX)/2);
            int playerPosY = (int)Player.valueGet<double>(namenKonvention.renderObject.positionY) + (int)(Player.valueGet<double>(namenKonvention.renderObject.pixelSizeY)/2);
            Renderer.updatePosition(playerPosX,playerPosY,true);

            // Append test obj
            Renderer.append(Player);
            Renderer.append(background);
            Player.update();          //update obj1 separately

            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            // Purge
            Renderer.purgeObjects();
        }
    }
    //End of Program!
    Renderer.destroy();
}

void TestEnv::_RenderObject::testRuleSets() {
    Renderer Renderer;
    
    RenderObject obj[20];
    
    for (int i = 0; i < 20; i++) {

        obj[i].valueSet(namenKonvention.renderObject.positionX, 500);
        obj[i].valueSet(namenKonvention.renderObject.positionY, 500);

        obj[i].valueSet(namenKonvention.renderObject.pixelSizeX, 5);
        obj[i].valueSet(namenKonvention.renderObject.pixelSizeY, 5);

        obj[i].loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionX, 300, 20*i / 7, 0.01));
        obj[i].loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionY, 300, 20*i / 7 + 3.1415 / 2, 0.01));
        Renderer.append(obj[i]);
    }
    

    //Renderer.serializeObjects();

    while (true) {
        if (Renderer.timeToRender()) {
            //Render Current instances
            Renderer.renderFrame();

            //Render FPS
            Renderer.renderFPS();

            // Present the renderer
            Renderer.showFrame();

            //Update all visible
            Renderer.update();
        }
    }
    //End of Program!
    Renderer.destroy();
}

