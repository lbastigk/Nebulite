//------------------------------------------------
// Main Tree attached functions 
#include <deque>
#include "Renderer.h"

Renderer renderer;

std::deque<std::string> tasks;

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
    renderer.clear();
    return 0;
}

int spawn(int argc, char* argv[]){
    if(argc>0){
        std::string obj = FileManagement::LoadFile(argv[0]);
        if(obj.size()){
            RenderObject ro;
            ro.deserialize(obj.c_str());
            renderer.append(ro);
        }
        else{
            std::cerr << "File could not be opened!" << std::endl;
            return 1;
        }
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return 1;
    }
    return 0;
}

int exitProgram(int argc, char* argv[]){
    renderer.setQuit();
    return 0;
}

int save(int argc, char* argv[]){
    std::cerr << "Function save not implemented yet!" << std::endl;
    return 0;
}

int wait(int argc, char* argv[]){
    std::cerr << "Function wait not implemented yet!" << std::endl;
    return 0;
}

int load(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return 0;
}

int task(int argc, char* argv[]) {
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
        tasks.push_back(line);
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