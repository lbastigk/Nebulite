//------------------------------------------------
// Main Tree attached functions 
#pragma once

#include <deque>
#include <memory>
#include "Renderer.h"
#include "FuncTree.h"


#include "JSON.h"


// Nebulite instances
namespace Nebulite {
    

    struct taskQueue {
        std::deque<std::string> taskList;
        uint64_t waitCounter = 0;
    };

    // Declare global instances
    extern taskQueue tasks_script;
    extern taskQueue tasks_internal;
    extern std::unique_ptr<Renderer> renderer;
    extern Invoke invoke;
    extern FuncTree mainTree;
    extern std::unique_ptr<Nebulite::JSON> global;
    
    // Function to init variables
    void init();

    // Function to init nebulite arg-bounded functions
    void init_functions();

    // Function to lazily initialize the renderer
    Renderer* getRenderer();

    // Converting string cmd to argc/argv
    void convertStrToArgcArgv(const std::string& cmd, int& argc, char**& argv);

    namespace mainTreeFunctions{

        int eval(int argc, char* argv[]);
        
        int envload(int argc, char* argv[]);

        int envdeload(int argc, char* argv[]);

        int spawn(int argc, char* argv[]);

        int exitProgram(int argc, char* argv[]);

        int save(int argc, char* argv[]);

        int wait(int argc, char* argv[]);

        int load(int argc, char* argv[]);

        int loadTaskList(int argc, char* argv[]);

        int echo(int argc, char* argv[]);

        int forLoop(int argc, char* argv[]);

        int error(int argc, char* argv[]);

        int setResolution(int argc, char* argv[]);

        int setFPS(int argc, char* argv[]);

        int serialize(int argc, char* argv[]);

        int moveCam(int argc, char* argv[]);

        int setCam(int argc, char* argv[]);

        int printGlobal(int argc, char* argv[]);

        int printState(int argc, char* argv[]);

        int setGlobal(int argc, char* argv[]);

        int json_test(int argc, char* argv[]);
    }

}