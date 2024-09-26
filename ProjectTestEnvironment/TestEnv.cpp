#pragma once

#include "_TEMPLATE_CLASS.cpp"
#include "FileManagement.cpp"
#include "JSONHandler.cpp"
#include "Kreatur.cpp"
#include "Renderer.cpp"
#include "Time.cpp"
#include "MoveRuleSet.cpp"
#include "DsaDebug.cpp"
#include "StringHandler.cpp"

class TestEnv {
public:
    class _FileManagement {
    public:
        static void testMenue() {
            system("cls");

            int opt = 0;
            std::string options = "Show files in ressource folder\nOpen file in ressource folder\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_FileManagement::testFileCollector();
                    break;
                case 1:
                    TestEnv::_FileManagement::testFileCollectorMenue();
                    break;
                }
            }
        }

        //Shows all files in ressources folder
        static void testFileCollector(){
            FileManagement::FileTypeCollector ftc(std::string("Resources"),std::string(".bmp"));

            std::cout << StringHandler::parseArray(ftc.getFileDirectories(),"Entry %i :\t","\n");

            Time::wait(10000);
        }

        //Same as before, but with opening
        static void testFileCollectorMenue() {
            FileManagement::FileTypeCollector ftc(std::string("Resources"), std::string(".bmp"));

            std::string options = StringHandler::parseArray(ftc.getFileDirectories(), "Entry %i :\t", "\n");

            int i = DsaDebug::menueScreen(options, 0, "Files available:\n\n", "\n\nPlease select one.",false);

            std::cout << "\n\nOpening:\t" << ftc.getFileDirectories().at(i) << " ...";

            FileManagement::openFileWithDefaultProgram(ftc.getFileDirectories().at(i));

            Time::wait(10000);
        }
    };
	class _General {
    public:
        static void testMenue() {
            system("cls");

            int opt = 0;
            std::string options = "benchmarkStringConversion\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_General::benchmarkStringConversion();
                    break;
                }
            }
        }

        static void benchmarkStringConversion() {
            // Seed the random number generator with the current time
            std::srand(static_cast<unsigned>(std::time(nullptr)));

            // Define the size of the array
            const uint64_t arraySize = 1e8;

            // Create an array of strings to store the random numbers
            // Allocate an array of std::string on the heap
            std::unique_ptr<std::string[]> randomNumbers(new std::string[arraySize]);
            //std::string randomNumbers[arraySize];

            // Generate and store random numbers in the array
            for (uint64_t i = 0; i < arraySize; ++i) {
                int randomNumber = std::rand();
                randomNumbers[i] = std::to_string(randomNumber);
            }

            // Convert ints
            uint64_t start_int = Time::gettime();
            for (uint64_t i = 0; i < arraySize; ++i) {
                int intValue = std::stoi(randomNumbers[i]);
            }
            uint64_t end_int = Time::gettime();

            // Convert floats
            uint64_t start_float = Time::gettime();
            for (uint64_t i = 0; i < arraySize; ++i) {
                float floatValue = std::stof(randomNumbers[i]);
            }
            uint64_t end_float = Time::gettime();

            printf(" Task ints took \t%010i ms for %i instances\n", (int)(end_int - start_int), arraySize);
            printf(" Task floats took\t%010i ms for %i instances\n\n", (int)(end_float - start_float), arraySize);

            Time::wait(5000);
        }

	};
    class _InventarObjekt {
    public:
        static void testMenue() {
            system("cls");

            int opt = 0;
            std::string options = "serialize\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_InventarObjekt::serialize();
                    break;
                }
            }
        }


        static void serialize() {
            InventarObjekt tee;
            std::cout << tee.serialize();

            Time::wait(5000);
        }
    };
    class _JSONHandler {
    public:
        static void testMenue() {
            system("cls");

            int opt = 0;
            std::string options = "listOfKeys\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_JSONHandler::listOfKeys();
                    break;
                }
            }
        }


        static void speed(int loopAmount) {
            uint64_t starttime = Time::gettime();

            //--------------------------------------------
            //Test Variables
            int testInt = 1;
            float testFloat = 2.718;
            double testDouble = 3.141;
            char testString[20] = "testString";
            std::string testStdString = "testStdString";

            std::map<std::string, std::string> testMap;
            testMap["key1"] = "value1";
            testMap["key2"] = "value2";

            std::vector<double> testVector;
            testVector.push_back(1.0);
            testVector.push_back(2.0);

            std::pair<int, std::string> testPair = std::make_pair(0, "pair");

            //Creating doc
            rapidjson::Document doc;

            //Looping for speed test
            for (volatile int i = 0; i < loopAmount; i++) {
                if (doc.IsObject()) {
                    doc.RemoveAllMembers();
                }

                //--------------------------------------------
                //Adding to doc
                JSONHandler::Set::Any(doc, "Int", testInt);
                JSONHandler::Set::Any(doc, "Float", testFloat);
                JSONHandler::Set::Any(doc, "Double", testDouble);
                JSONHandler::Set::Any(doc, "String", testString);
                JSONHandler::Set::Any(doc, "StdString", testStdString);
                JSONHandler::Set::Any(doc, "Map", testMap);
                JSONHandler::Set::Any(doc, "Vector", testVector);
                JSONHandler::Set::Any(doc, "Pair", testPair);

                //--------------------------------------------
                //Adding doc intself to doc
                rapidjson::Document doc2;
                doc2.CopyFrom(doc, doc.GetAllocator());
                JSONHandler::Set::Any(doc, "JSON", &doc2);
            }

            //--------------------------------------------
            //Printing doc once
            std::cout << JSONHandler::serialize(doc);

            //--------------------------------------------
            //Show elapsed time
            printf("\n\nTime taken: %i ms\n", (int)(Time::gettime() - starttime));
        };

        static void setGet() {
            rapidjson::Document doc;

            JSONHandler::Set::Any(doc, namenKonvention.MyTemplate._self, namenKonvention.MyTemplate.bsp1);
            std::cout << JSONHandler::Get::Any<std::string>(doc, namenKonvention.MyTemplate._self, "");
        }

        static void KeyNesting() {
            rapidjson::Document mainDoc;
            
            //insert intop main doc
            JSONHandler::Set::Any<double>(mainDoc, std::string("level1-double"), 3.14);
            JSONHandler::Set::Any<int>(mainDoc, std::string("level1-level2-int"), 10);

            //first step: get
            std::cout << "Getting:\n-------------------------------------------------\n";
            std::cout << "Main Doc:\n\n" << JSONHandler::serialize(mainDoc) << "\n\n";
            std::cout << "level 2 int:" << JSONHandler::Get::Any<int>(mainDoc, std::string("level1-level2-int"),0);
            
            //second step, setting int inside level2 to another value
            std::cout << "\n\nSetting int to 11:\n-------------------------------------------------\n";
            JSONHandler::Set::Any<int>(mainDoc, std::string("level1-level2-int"),11);

            //Check if it's correct, show full doc and test get function again.
            std::cout << "Main Doc:\n\n" << JSONHandler::serialize(mainDoc) << "\n\n";
            std::cout << "level 2 int:" << JSONHandler::Get::Any<int>(mainDoc, std::string("level1-level2-int"), 0);

            //For viewing result
            Time::wait(20000);

        }

        static void listOfKeys() {
            rapidjson::Document doc;
            JSONHandler::Set::Any(doc, "test1", "lol");
            JSONHandler::Set::Any(doc, "test2", 123);
            JSONHandler::Set::Any(doc, "test3", 3.14);

            std::vector<std::string> keys;

            JSONHandler::Get::listOfKeys(doc, keys);

            for (const auto& key : keys) {
                std::cout << key << "\n";
            }

            Time::wait(5000);
        }

        class Memoryleak {
        public:
            //Sets a value of the doc for 'amount' many times. Value increases by 1 with each iteration
            //Return value should match amount on return!
            //Value type is double
            //Debug to see value each 100 iterations
            static double setAny(UINT64 amount, bool debug = false) {
                rapidjson::Document doc;
                double fromDoc = 0;
                for (UINT64 i = 0; i < amount; i++) {
                    JSONHandler::Set::Any<double>(doc, "tee", (double)i+1.0);
                    fromDoc = JSONHandler::Get::Any<double>(doc, "tee");
                    if (debug && !(i % 100)) {
                        std::cout << "Value after " << i << " iterations: " << fromDoc << "\n";
                    }
                }
                return fromDoc;
            }

            //Sets a value of the doc for 'amount' many times. Value in subdoc increases by 1 with each Iteration
            //Subdoc is rebuild from main doc with each iteration
            //Return string is entire doc
            //Debug to see full doc each 100 iterations
            static std::string addGetSubDoc(UINT64 amount, bool debug = false) {
                //main Doc with static Variable
                rapidjson::Document doc;
                JSONHandler::Set::Any<double>(doc, "staticVar", 3.14);

                //Subdoc
                rapidjson::Document subDoc;

                for (UINT64 i = 0; i < amount; i++) {
                    //Get back subdoc
                    JSONHandler::Get::subDoc(doc, "subDoc", subDoc);

                    //Set subdoc
                    JSONHandler::Set::Any<int>(subDoc, "subvar", (double)i+1.0);
                    
                    //Add to main doc
                    JSONHandler::Set::subDoc(doc, "subDoc", subDoc);    

                    if (debug && !(i % 100)) {
                        std::cout << "Value after " << i << " iterations: " << JSONHandler::serialize(doc) << "\n";
                    }
                }

                //Return entire doc
                return JSONHandler::serialize(doc);
            }

            //Full memory leak test
            static void fullTest(UINT64 amount = 10000000) {
                uint64_t starttime;

                std::cout << "Testing for memory leaks\n";
                
                std::cout << "Total memory used: " << DsaDebug::getMemoryUsagekB() << "kB\n";
                
                std::cout << "---------------------------\n\n";

                std::cout << "Test 1: setting double,        \t" << StringHandler::uint64ToStringWithGroupingAndPadding(amount, 12) << " Iterations...\n";
                starttime = Time::gettime();
                double checksum = setAny(amount);
                std::cout << "\tTime taken:" << Time::getruntime(starttime) << "\n";
                std::cout << "\tTotal memory used after test: " << DsaDebug::getMemoryUsagekB() << "kB\n";
                std::cout << "\tChecksum: " << checksum << "\n\n";

                std::cout << "Test 2: setting/getting subdoc,\t" << StringHandler::uint64ToStringWithGroupingAndPadding(amount,12) << " Iterations...\n";
                starttime = Time::gettime();
                std::string checkstring = addGetSubDoc(amount);
                std::cout << "\tTime taken:" << Time::getruntime(starttime) << "\n";
                std::cout << "\tTotal memory used after test: " << DsaDebug::getMemoryUsagekB() << "kB\n";
                std::cout << "\tCheckstring:\n";
                DsaDebug::printWithPrefix(checkstring, "\t");
                std::cout << "\n\n";
                
            }
        };

        class Retired {
            //Also good for benchmarking
            // Times in ms for 1 000 000 iterations:
            //[2024-02-14] : 2564
            static void testMemLeak() {
                //main doc
                rapidjson::Document doc;
                JSONHandler::Set::Any<double>(doc, "double", 3.14);

                rapidjson::Document loc;

                //Copy a subdoc into doc many times to check for leakage
                auto start = Time::gettime();


                for (UINT64 i = 0; i < 1000000; i++) {
                    //create subdoc, fill it with some stuff
                    //needs to be recreated each time, as JSONHandler::Set::subDoc will delete this one
                    rapidjson::Document subdoc;
                    JSONHandler::Set::Any<int>(subdoc, "subvar", 0);
                    JSONHandler::Set::Any<int>(subdoc, "subvar2", 0);

                    //DEBUG, for checking docs
                    if (false) {
                        std::cout << "DEBUG before set\n";
                        std::cout << "---------------------------\n";
                        std::cout << "SubDoc Template: \n" << JSONHandler::serialize(subdoc) << "\n\n";
                        std::cout << "Full doc: \n" << JSONHandler::serialize(doc) << "\n\n";
                        std::cout << "SubDoc Copy: \n" << JSONHandler::serialize(loc) << "\n\n";
                    }
                    JSONHandler::empty(loc);

                    //Add subdoc
                    JSONHandler::Set::subDoc(doc, "sd", subdoc);

                    //Get subdoc back
                    JSONHandler::Get::subDoc(doc, "sd", loc);
                }
                std::cout << Time::getruntime(start) << "\n\n\n";
                std::cout << JSONHandler::serialize(doc) << "\n";
                std::cout << JSONHandler::serialize(loc) << "\n";
                Time::wait(10000);                       //Just to wait some seconds as function ends
            }
        };
    };
    class _Kreatur {
    public:
        static void testMenue() {
            system("cls");

            int opt = 0;
            std::string options = "serialize\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_Kreatur::serialize();
                    break;
                }
            }
        }


        static void serialize() {
            Kreatur tee;
            std::cout << tee.serialize();
        }
    };
    class _MoveRuleSet {
    public:
        static void testMenue() {
            system("cls");

            int opt = 2;
            std::string options = "serialize\ntestMoveSet\ntest single obj\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_MoveRuleSet::serialize();
                    break;
                case 1:
                    TestEnv::_MoveRuleSet::testMoveSet();
                    break;
                case 2:
                    TestEnv::_MoveRuleSet::testSingleObject();
                    break;
                }
            }
        
        }


        static void serialize() {
            MoveRuleSet mrs;
            std::cout << mrs.serialize();
        }

        static void testMoveSet(int objCount = 32, int ringCount = 16) {
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
                }
            }
            //End of Program!
            Renderer.destroy();
        }

        static void testSingleObject() {
            //Renderer
            Renderer Renderer;
            Renderer.setFPS(60);

            //Add object
            RenderObject Obj;
            Obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionX, 100, 0, 0.01));
            Obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionY, 100, 0, 0.01));
            Obj.valueSet(namenKonvention.renderObject.positionX, 500);
            Obj.valueSet(namenKonvention.renderObject.positionY, 500);

            

            

            bool quit = false;
            int event = 0;
            while (!quit) {
                if (Renderer.timeToRender()) {
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

                    //Renderer.update();
                }
            }
            //End of Program!
            Renderer.destroy();
        }

    };
    class _RenderObject {
    public:
        static void testMenue() {
            system("cls");

            int opt = 1;
            std::string options = "Test Spritesheets\nTest Pokemon Sprite\ntestRuleSets\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_RenderObject::testSpriteSheets();
                    break;
                case 1:
                    TestEnv::_RenderObject::testPokemonSprites();
                    break;
                case 2:
                    TestEnv::_RenderObject::testRuleSets();
                    break;
                }
            }
        }

        static void testSpriteSheets() {
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
                    
                    system("cls");
                    std::cout << Renderer.serializeEnvironment();
                }
            }
            //End of Program!
            Renderer.destroy();
        }

        static void testPokemonSprites() {
            Renderer Renderer;
            RenderObject obj1;

            //Standard stuff
            obj1.valueSet(namenKonvention.renderObject.positionX, 200);
            obj1.valueSet(namenKonvention.renderObject.positionY, 200);
            obj1.valueSet(namenKonvention.renderObject.pixelSizeX, 160);
            obj1.valueSet(namenKonvention.renderObject.pixelSizeY, 240);

            //Spritesheet
            obj1.valueSet(namenKonvention.renderObject.imageLocation, "Resources/Sprites/Pokemon/Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png");
            obj1.valueSet(namenKonvention.renderObject.isSpritesheet, true);
            obj1.valueSet(namenKonvention.renderObject.spritesheetOffsetX, 9);
            obj1.valueSet(namenKonvention.renderObject.spritesheetOffsetY, 42);
            obj1.valueSet(namenKonvention.renderObject.spritesheetSizeX, 16);
            obj1.valueSet(namenKonvention.renderObject.spritesheetSizeY, 24);

            //Sprite Movement
            MoveRuleSet posX = MoveRuleSet::Examples::upAndDown(namenKonvention.renderObject.spritesheetOffsetX, 17 * 2, 17, true,3);
            obj1.loadMoveSet(posX);

            //DEBUG
            std::cout << "\nDST: ";
            std::cout << "\tx: " << obj1.getDstRect().x;
            std::cout << "\ty: " << obj1.getDstRect().y;
            std::cout << "\tw: " << obj1.getDstRect().w;
            std::cout << "\th: " << obj1.getDstRect().h;

            std::cout << "SRC: ";
            std::cout << "\tx: " << obj1.getSrcRect()->x;
            std::cout << "\ty: " << obj1.getSrcRect()->y;
            std::cout << "\tw: " << obj1.getSrcRect()->w;
            std::cout << "\th: " << obj1.getSrcRect()->h;

            std::cout << obj1.serialize();

            // Append test obj
            Renderer.append(obj1);

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

                    //system("cls");
                    //std::cout << Renderer.serializeEnvironment();
                    //std::cout << "\n";
                }
            }
            //End of Program!
            Renderer.destroy();
        }

        static void testRuleSets() {
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

            //std::cout << obj[0].serialize();
            

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
    };
    class _Renderer {
    public:
        static void testMenue() {
            system("cls");

            int opt = 4;
            std::string options = "randomImages\ncircleBenchmark\nThreadTest\ntestRendererMemLeak\nmovement\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                    
                case 0:
                    TestEnv::_Renderer::randomImages();
                    break;
                case 1:
                    TestEnv::_Renderer::circleBenchmark();
                    break;
                case 2:
                    TestEnv::_Renderer::ThreadTest();
                    break;
                case 3:
                    TestEnv::_Renderer::testRendererMemLeak();
                    break;
                case 4:
                    TestEnv::_Renderer::movement();
                    break;
                }
            }
        }

        static int randomImages(int objectCount = 128) {
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

        static int circleBenchmark(int objectCount = 128) {
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
            Renderer.updatePosition(0, 0);

            

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
                    event = Renderer.handleEvent();

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
        
        static void ThreadTest(int objCount = 1024, int ringCount = 16, int framesToRender = 30) {
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
                event = Renderer.handleEvent();

                //Render Current instances
                Renderer.renderFrame();
                Renderer.showFrame();
            }
        }

        static void testRendererMemLeak(int loopamount = 128, bool dump = true) {
            if (dump) {
                // Send all reports to STDOUT
                _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
                _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
                _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
                _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
                _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
                _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
            }

            for (int i = 0; i < loopamount; i++) {
                Renderer Renderer;
                createRandomObjects(Renderer,128,300);
                Renderer.renderFrame();
                Renderer.showFrame();
            }
            if (dump){
                _CrtDumpMemoryLeaks();
                //Time::wait(10000);
            }
        };

        static int movement() {

            //Renderer Object
            Renderer Renderer;
            Renderer.setFPS(60);

            //General Variables
            bool quit = false;
            int event = 0;

            RenderObject player;
            player.valueSet(namenKonvention.renderObject.positionX, 500);
            player.valueSet(namenKonvention.renderObject.positionY, 500);

            while (!quit) {
                //Event handling
                SDL_Event sdlEvent = Renderer.getEventHandle();

                if (sdlEvent.type == SDL_KEYDOWN) {
                    switch (sdlEvent.key.keysym.sym) {
                    case Renderer::SDL::KEY_W:
                        if (!player.hasMoveSet()) {
                            player.loadMoveSet(MoveRuleSet::Examples::Move::linear(0, -32, 5, 5));
                        }
                        break;
                    case Renderer::SDL::KEY_S:
                        if (!player.hasMoveSet()) {
                            player.loadMoveSet(MoveRuleSet::Examples::Move::linear(0, 32, 5, 5));
                        }
                        break;
                    case Renderer::SDL::KEY_A:
                        if (!player.hasMoveSet()) {
                            player.loadMoveSet(MoveRuleSet::Examples::Move::linear(-32, 0, 5, 5));
                        }
                        break;
                    case Renderer::SDL::KEY_D:
                        if (!player.hasMoveSet()) {
                            player.loadMoveSet(MoveRuleSet::Examples::Move::linear(32, 0, 5, 5));
                        }
                        break;
                    }

                }
                if (sdlEvent.type == SDL_QUIT) {
                    quit = true;
                }

                if (Renderer.timeToRender()) {
                    player.update();

                    Renderer.append(player);

                    //Render Current instances
                    Renderer.renderFrame();

                    //Render FPS
                    Renderer.renderFPS();

                    // Present the renderer
                    Renderer.showFrame();

                    //Delete objects
                    Renderer.purgeObjects();
                }
            }
            //End of Program!
            Renderer.destroy();

            return 0;

        }

    private:
        static void createRandomObjects(Renderer& Renderer, int amount, int radius) {

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

        static void createCircleOLD(Renderer& Renderer, int amount, int radius) {

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

        static void createCircle001P(Renderer& Renderer, int amount, int radius, int colour128, int _dX, int _dY) {

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

        static void createCircle100P(Renderer& Renderer, int amount, int radius, int colour128, int _dX, int _dY) {

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
        static UINT64 stressTest(int objCount = 512, int ringCount = 8, int threadSize = 1024, unsigned int framesToRender = 300, bool bypassThreads = false) {
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

                    obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionX, 540, (i * 2 * 3.141 / objCount), 1));
                    obj.loadMoveSet(MoveRuleSet::Examples::sin(namenKonvention.renderObject.positionY, 540, (i * 2 * 3.141 / objCount) + (j * 3.141 / 8) + 3.141 / 4, 0.8));
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

	};
    class _Talente {
    public:
        static void testMenue() {
            system("cls");

            int opt = 0;
            std::string options = "testTalentJson\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_Talente::testTalentJson();
                    break;
                }
            }
        }


        static void testTalentJson() {
            Talente talente;
            std::cout << talente.serialize();

            Time::wait(5000);
        }

    };
    class _Template {
    public:
        static void testMenue() {
            system("cls");

            int opt = 0;
            std::string options = "Template serialization test\nQuit";
            while (opt != std::ranges::count(options, '\n')) {

                opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");

                system("cls");

                switch (opt) {
                case 0:
                    TestEnv::_Template::testMyTemplate();
                    break;
                }
            }
        }


        static void testMyTemplate() {
            MyTemplate temp1;
            MyTemplate temp2;
            temp1.valueSet("testString", "zero");
            temp2.valueSet("testString2", "zero2");
            std::cout << "Obj1:\n";
            std::cout << temp1.serialize();
            std::cout << "\n";

            std::cout << "Obj2:\n";
            std::cout << temp2.serialize();
            std::cout << "\n";

            std::cout << "Obj2 copy to Obj1...\n";
            temp1 = temp2;
            std::cout << "Obj1:\n";
            std::cout << temp1.serialize();
            std::cout << "\n";
            std::cout << "Obj2:\n";
            std::cout << temp2.serialize();
            std::cout << "\n";

            Time::wait(5000);
        }

	};
};