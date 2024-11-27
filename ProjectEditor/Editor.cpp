#include "Editor.h"

Editor::Editor() {
        directory.levels = FileManagement::CombinePaths(FileManagement::CombinePaths(FileManagement::currentDir(),std::string("Resources")), std::string("Levels"));
    }

void Editor::LoadLevel() {
    //Check files in directory "Levels"
    FileManagement::FileTypeCollector ftc(directory.levels, std::string(".json"), false);
    std::vector<std::string> files = ftc.getFileDirectories();

    //Create options string
    std::string options;
    for (const auto& file : files) {
        options += file + "\n";
    }
    options += "New File\nQuit";

    //Create options menue
    Platform::clearScreen();
    int opt = 0;
    while (opt != std::ranges::count(options, '\n')) {
        opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");
        Platform::clearScreen();

        //New file
        std::string levelname;
        if (opt == files.size()) {
            //Get file name
            Platform::clearScreen();
            std::cout << "File name: ";
            std::string name;
            std::cin >> name;

            //Add file ending
            name += ".json";

            //Create dummy environment and save it
            Environment env;
            name = FileManagement::CombinePaths(directory.levels, name);
            FileManagement::WriteFile(name.c_str(), env.serialize());

            //Set file name to Editor
            levelname = name;
            
        }
        //Set File name
        else {
            levelname = files.at(opt);
        }

        LevelEditor LevelEditor;
        LevelEditor.loadEnvironment(levelname);
        
        //Start editor loop
        while (LevelEditor.status()) {
            LevelEditor.update();
        }
    }
}

void Editor::CreateRenderobject() {
    Platform::clearScreen();
    RenderObject ro;
    ro.valueSet(namenKonvention.renderObject.positionX, 200);
    ro.valueSet(namenKonvention.renderObject.positionY, 200);

    std::vector<std::string> opts;
    std::vector<std::string> opts_key;

    //-----------------------------------------------------
    //List of options

    

    //Strings
    int listStartOfString = opts.size();
    opts.push_back("Texture Image Location");
    opts_key.push_back(namenKonvention.renderObject.imageLocation);

    //Bools
    int listStartOfBool = opts.size();
    opts.push_back("Is Overlay");
    opts_key.push_back(namenKonvention.renderObject.isOverlay);
    opts.push_back("Is Spritesheet");
    opts_key.push_back(namenKonvention.renderObject.isSpritesheet);

    //Ints
    int listStartOfInt = opts.size();
    opts.push_back("Layer");
    opts_key.push_back(namenKonvention.renderObject.layer);
    opts.push_back("Pixel Size X");
    opts_key.push_back(namenKonvention.renderObject.pixelSizeX);
    opts.push_back("Pixel Size Y");
    opts_key.push_back(namenKonvention.renderObject.pixelSizeY);
    opts.push_back("Position X");
    opts_key.push_back(namenKonvention.renderObject.positionX);
    opts.push_back("Position Y");
    opts_key.push_back(namenKonvention.renderObject.positionY);
    opts.push_back("X Offset of Sprite sheet");
    opts_key.push_back(namenKonvention.renderObject.spritesheetOffsetX);
    opts.push_back("Y Offset of Sprite sheet");
    opts_key.push_back(namenKonvention.renderObject.spritesheetOffsetY);
    opts.push_back("X Size of Sprite sheet");
    opts_key.push_back(namenKonvention.renderObject.spritesheetSizeX);
    opts.push_back("Y Size of Sprite sheet");
    opts_key.push_back(namenKonvention.renderObject.spritesheetSizeY);

    //Other opts
    opts.push_back("Serialize");
    opts.push_back("Save");

    std::string stringOfOpts;
    for (auto opt : opts) {
        stringOfOpts += opt + "\n";
    }
    stringOfOpts += "Exit";

    // Renderer
    Renderer Renderer;
    int event = 0;
    

    // Options Menue
    int lastOptionChoosen = 0;
    while (lastOptionChoosen != opts.size()) {
        //Event handling
        event = Renderer.handleEvent();

        // Append object
        Renderer.append(ro);
        Renderer.update();      //Important after appending, for rect update!

        //Render Current instances
        Renderer.renderFrame();

        //Render FPS
        Renderer.renderFPS();

        // Present the renderer
        Renderer.showFrame();

        // Delete Objects
        Renderer.purgeObjects();

        lastOptionChoosen = DsaDebug::menueScreen(stringOfOpts, lastOptionChoosen, "Choose:\n\n", "", true);
        if (lastOptionChoosen < opts_key.size()) {
            std::cout << "\n\nCurrent Value is: " << ro.valueGet<std::string>(opts_key.at(lastOptionChoosen)) << "\nSet new Value:    ";
            char buffer[200];
            std::cin.getline(buffer, sizeof(buffer));
            std::string newVal(buffer);

            if (lastOptionChoosen < listStartOfBool) {
                // String
                ro.valueSet(opts_key.at(lastOptionChoosen), newVal);
            }
            else if (lastOptionChoosen < listStartOfInt) {
                // Bool
                ro.valueSet(opts_key.at(lastOptionChoosen), newVal=="true"?true:false);
            }
            else {
                // Int
                ro.valueSet(opts_key.at(lastOptionChoosen), stoi(newVal));
            }
        }
        else if (lastOptionChoosen < opts.size()){
            if (opts.at(lastOptionChoosen) == "Serialize") {
                std::cout << ro.serialize();
                Platform::getCharacter();
            }
            if(opts.at(lastOptionChoosen) == "Save"){
                std::cout << "\n\nEnter file name: ./Resources/Renderobjects/";
                char buffer[200];
                std::cin.getline(buffer, sizeof(buffer));
                std::string newVal(buffer);
                newVal = "./Resources/Renderobjects/"+newVal;
                FileManagement::WriteFile(newVal.c_str(),ro.serialize());
            }
        }
    }
    
    //End of Program!
    Renderer.destroy();

    
}

void Editor::CreateMoveRuleSet() {

}
