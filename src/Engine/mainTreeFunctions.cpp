//------------------------------------------------
// Main Tree attached functions 
#include <deque>
#include "Renderer.h"

Renderer renderer;

std::deque<std::string> tasks;
uint64_t waitCounter = 0;

namespace mainTreeFunctions{


int envload(int argc, char* argv[]){
    if(argc > 0){
        std::cout << "Loading env: " << argv[0] << std::endl;
        renderer.deserialize(argv[0]);
        return 0;
    }
    else{
        std::cerr << "No env name provided!" << std::endl;
        return 1;
    }
}

int envdeload(int argc, char* argv[]){
    renderer.purgeObjects();
    renderer.purgeTextures();
    return 0;
}

int spawn(int argc, char* argv[]){
    if(argc>0){
        std::cout << "Spawning object: " << argv[0] << std::endl;
        RenderObject ro;
        ro.deserialize(argv[0]);
        renderer.append(ro);
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return 1;
    }
    return 0;
}

int exitProgram(int argc, char* argv[]){
    renderer.setQuit();
    std::cout << "Exiting program" << std::endl;
    return 0;
}

int save(int argc, char* argv[]){
    std::cerr << "Function save not implemented yet!" << std::endl;
    return 0;
}

int wait(int argc, char* argv[]){
    std::cout << "Setting new value for wait" << std::endl;
    std::istringstream iss(argv[0]);
    iss >> waitCounter;
    return 0;
}

int load(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return 0;
}

int loadTaskList(int argc, char* argv[]) {
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
            tasks.push_back(line);
        }
    }

    return 0;
}

int echo(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return 0;
}

int setResolution(int argc, char* argv[]){
    if(argc != 2){
        renderer.changeWindowSize(1000,1000);
    }
    else{
        int w = std::stoi(argv[0]);
        int h = std::stoi(argv[1]);
        renderer.changeWindowSize(w,h);
    }
    return 0;
}

int setFPS(int argc, char* argv[]){
    if(argc != 1){
        renderer.setFPS(60);
    }
    else{
        int fps = std::stoi(argv[0]);
        if(fps < 1) fps=1;
        if(fps > 10000) fps=10000;
        renderer.setFPS(fps);
    }
    return 0;
}

int serialize(int argc, char* argv[]){
    std::string serialized = renderer.serialize();
    if (argc>0){
        FileManagement::WriteFile(argv[0],serialized);
    }
    else{
        FileManagement::WriteFile("lastLevel.log.json",serialized);
    }
}



// END OF NAMESPACE
}

