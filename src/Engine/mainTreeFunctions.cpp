//------------------------------------------------
// Main Tree attached functions 
#include "mainTreeFunctions.h"

// Separate queues for script and internal
// Otherwise, a wait from a script can halt the entire game logic
// All wait calls influence script queue for now
namespace Nebulite{
    taskQueue tasks_script;
    taskQueue tasks_internal;
    std::unique_ptr<Renderer> renderer = nullptr;

    Renderer* getRenderer() {
        if (!renderer) {
            renderer = std::make_unique<Renderer>(tasks_internal.taskList);
            renderer->setFPS(60);
        }
        return renderer.get();
    }
}


int Nebulite::mainTreeFunctions::envload(int argc, char* argv[]){
    if(argc > 0){
        std::cout << "Loading env: " << argv[0] << std::endl;
        Nebulite::getRenderer()->deserialize(argv[0]);
        return 0;
    }
    else{
        std::cerr << "No env name provided!" << std::endl;
        return 1;
    }
}

int Nebulite::mainTreeFunctions::envdeload(int argc, char* argv[]){
    Nebulite::getRenderer()->purgeObjects();
    Nebulite::getRenderer()->purgeTextures();
    return 0;
}

int Nebulite::mainTreeFunctions::spawn(int argc, char* argv[]){
    if(argc>0){
        std::cout << "Spawning object: " << argv[0] << std::endl;
        RenderObject ro;
        ro.deserialize(argv[0]);

        auto ptr = std::make_shared<RenderObject>(std::move(ro));
        Nebulite::getRenderer()->append(ptr);

        // DEBUG: Check object after appending:
        //std::cout << ptr.get()->serialize() << std::endl;
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return 1;
    }
    return 0;
}

int Nebulite::mainTreeFunctions::exitProgram(int argc, char* argv[]){
    Nebulite::getRenderer()->setQuit();
    std::cout << "Exiting program" << std::endl;
    return 0;
}

int Nebulite::mainTreeFunctions::save(int argc, char* argv[]){
    std::cerr << "Function save not implemented yet!" << std::endl;
    return 0;
}

int Nebulite::mainTreeFunctions::wait(int argc, char* argv[]){
    std::istringstream iss(argv[0]);
    iss >> tasks_script.waitCounter;
    if (tasks_script.waitCounter < 0){
        tasks_script.waitCounter = 0;
    }
    return 0;
}

int Nebulite::mainTreeFunctions::load(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return 0;
}

int Nebulite::mainTreeFunctions::loadTaskList(int argc, char* argv[]) {
    std::cout << "Loading tasks!" << std::endl;

    if (argc < 1) {
        std::cout << "Usage: task <filename>" << std::endl;
        return 1;
    }

    std::ifstream infile(argv[0]);
    if (!infile) {
        std::cerr << "Error: Could not open file '" << argv[0] << "'" << std::endl;
        return 1;
    }
    
    std::string line;
    while (std::getline(infile, line)) {
        line = StringHandler::untilSpecialChar(line,'#');   // Remove comment
        line = StringHandler::lstrip(line,' ');             // Remove whitespaces at start
        if(line.length() == 0){
            // line is empty
            continue;
        }
        else{
            tasks_script.taskList.push_back(line);
        }
    }

    return 0;
}

int Nebulite::mainTreeFunctions::echo(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return 0;
}

int Nebulite::mainTreeFunctions::setResolution(int argc, char* argv[]){
    if(argc != 2){
        Nebulite::getRenderer()->changeWindowSize(1000,1000);
    }
    else{
        int w = std::stoi(argv[0]);
        int h = std::stoi(argv[1]);
        Nebulite::getRenderer()->changeWindowSize(w,h);
    }
    return 0;
}

int Nebulite::mainTreeFunctions::setFPS(int argc, char* argv[]){
    if(argc != 1){
        Nebulite::getRenderer()->setFPS(60);
    }
    else{
        int fps = std::stoi(argv[0]);
        if(fps < 1) fps=1;
        if(fps > 10000) fps=10000;
        Nebulite::getRenderer()->setFPS(fps);
    }
    return 0;
}

int Nebulite::mainTreeFunctions::serialize(int argc, char* argv[]){
    std::string serialized = Nebulite::getRenderer()->serialize();
    if (argc>0){
        FileManagement::WriteFile(argv[0],serialized);
    }
    else{
        FileManagement::WriteFile("last_state.log.json",serialized);
    }
    return 0;
}

int Nebulite::mainTreeFunctions::moveCam(int argc, char* argv[]){
    if(argc == 2){
        int dx = std::stoi(argv[0]);
        int dy = std::stoi(argv[1]);
        Nebulite::getRenderer()->moveCam(dx,dy);
        return 0;
    }
    else{
        std::cerr << "Expected 2 args, provided " << argc << std::endl;
        for(int i = 0; i < argc ; i++){
            std::cerr << "   " << argv[i] << std::endl;
        }
        return 1;
    }
}

int Nebulite::mainTreeFunctions::setCam(int argc, char* argv[]){
    if(argc == 2){
        int x = std::stoi(argv[0]);
        int y = std::stoi(argv[1]);
        Nebulite::getRenderer()->setCam(x,y);
        return 0;
    }
    if(argc == 3){
        if(!strcmp(argv[2], "c")){
            int x = std::stoi(argv[0]);
            int y = std::stoi(argv[1]);
            Nebulite::getRenderer()->setCam(x,y,true);
            return 0;
        }
        else{
            // unknown arg
            return 1;
        }
    }
    else{
        return 1;
    }
}

int Nebulite::mainTreeFunctions::printGlobal(int argc, char* argv[]){
    std::cout << Nebulite::getRenderer()->serializeGlobal() << std::endl;
    return 0;
}

int Nebulite::mainTreeFunctions::printState(int argc, char* argv[]){
    std::cout << Nebulite::getRenderer()->serialize() << std::endl;
    return 0;
}


