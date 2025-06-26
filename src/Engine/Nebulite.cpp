//------------------------------------------------
// Main Tree attached functions 
#include "Nebulite.h"

// Separate queues for script and internal
// Otherwise, a wait from a script can halt the entire game logic
// All wait calls influence script queue for now
namespace Nebulite{
    taskQueue tasks_script;
    taskQueue tasks_internal;
    std::unique_ptr<Nebulite::Renderer> renderer = nullptr;
    Invoke invoke;
    FuncTree mainTree("Nebulite");
    std::unique_ptr<Nebulite::JSON> global = nullptr;

    // used by convertStrToArgcArgv
    char* argvBuffer = nullptr;
    int argvCapacity = 0;
    
    // init variables
    void init(){
        global = std::make_unique<Nebulite::JSON>();
        invoke.linkGlobal(*global);
	    invoke.linkQueue(tasks_internal.taskList);
        std::string stateName = "";
    }

    // Init nebulite functions
    void init_functions(){
        
        // General
        mainTree.attachFunction(Nebulite::mainTreeFunctions::eval,            "eval",         "Evaluate all $(...) after this keyword, parse rest as usual");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::setGlobal,       "set-global",   "Set any global variable: [key] [value]");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::envload,         "env-load",     "Loads an environment");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::envdeload,       "env-deload",   "Deloads an environment");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::spawn,           "spawn",        "Spawn a renderobject");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::exitProgram,     "exit",         "exits the program");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::stateSave,       "state-save",   "Saves the state");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::stateLoad,       "state-load",   "Loads a state");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::loadTaskList,    "task",         "Loads a txt file of tasks");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::wait,            "wait",         "Halt all commands for a set amount of frames");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::forLoop,         "for",          "Start for-loop. Usage: for var <iStart> <iEnd> command $var");
        
        // Renderer Settings
        mainTree.attachFunction(Nebulite::mainTreeFunctions::setFPS,          "set-fps",      "Sets FPS to an integer between 1 and 10000. 60 if no arg is provided");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::setResolution,   "set-res",      "Sets resolution size: [w] [h]");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::setCam,          "cam-set",      "Sets Camera position [x] [y] <c>");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::moveCam,         "cam-move",     "Moves Camera position [dx] [dy]");

        // Debug
        mainTree.attachFunction(Nebulite::mainTreeFunctions::serialize,       "serialize",    "Serialize current State to file");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::echo,            "echo",         "Echos all args provided to cout");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::error,           "error",        "Echos all args provided to cerr");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::printGlobal,     "print-global", "Prints global doc to cout");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::printState,      "print-state",  "Prints state doc to cout");

        // Helper
        mainTree.attachFunction(Nebulite::mainTreeFunctions::render_object,    "standard-render-object",  "Serializes standard renderobject to ./Resources/Renderobjects/standard.json");

        // Internal Tests
        // None atm
    }


    Renderer* getRenderer() {
        if (!renderer) {
            renderer = std::make_unique<Nebulite::Renderer>(invoke,*global);
            renderer->setFPS(60);
        }
        return renderer.get();
    }


}

int Nebulite::resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree){
    int result = 0;
    while (!tq.taskList.empty() && (counter == nullptr || *counter == 0)) {
        // Get task
        std::string argStr = tq.taskList.front();
        tq.taskList.pop_front();  // remove the used task

        // Convert std::string to argc,argv
        *argc_mainTree = 0;
        *argv_mainTree = nullptr;
        Nebulite::convertStrToArgcArgv(argStr, *argc_mainTree, *argv_mainTree);

        if (*argv_mainTree != nullptr && argStr.size()) {
            result = Nebulite::mainTree.parse(*argc_mainTree, *argv_mainTree);
        }
        else{
            result = 0;
        }
    }
    return result;
}

void Nebulite::convertStrToArgcArgv(const std::string& cmd, int& argc, char**& argv) {
    // Free previous buffer if any
    if (argvBuffer) {
        delete[] argvBuffer;
        argvBuffer = nullptr;
        argvCapacity = 0;
    }

    argvCapacity = static_cast<int>(cmd.size()) + 1;
    argvBuffer = new char[argvCapacity];
    std::memcpy(argvBuffer, cmd.c_str(), argvCapacity);

    std::vector<char*> argvVec;
    argc = 0;
    bool inToken = false;
    for (int i = 0; i < argvCapacity; ++i) {
        if (argvBuffer[i] == ' ' || argvBuffer[i] == '\t') {
            argvBuffer[i] = '\0';
            inToken = false;
        } else if (!inToken) {
            argvVec.push_back(&argvBuffer[i]);
            argc++;
            inToken = true;
        }
    }

    // Allocate argv and copy pointers
    argv = new char*[argc + 1];
    for (int i = 0; i < argc; ++i) {
        argv[i] = argvVec[i];
    }
    argv[argc] = nullptr;
}

int Nebulite::mainTreeFunctions::eval(int argc, char* argv[]){
    // argc/argv to string for evaluation
    std::string args = "";
    for (int i = 0; i < argc; ++i) {
        args += argv[i];
        if (i < argc - 1) {
            args += " ";
        }
    }

    // eval all $(...)
    std::string args_evaled = Nebulite::invoke.resolveGlobalVars(args);

    // convert back to argc/argv
    int argc_new = 0;
    char** argv_new = nullptr;
    Nebulite::convertStrToArgcArgv(args_evaled, argc_new, argv_new);

    // reparse
    return mainTree.parse(argc_new, argv_new);
}

int Nebulite::mainTreeFunctions::setGlobal(int argc, char* argv[]){
    if(argc == 2){
        std::string key = argv[0];
        std::string value = argv[1];
        Nebulite::getRenderer()->getGlobal().set<std::string>(key.c_str(),value);
        return 0;
    }
    if(argc == 1){
        std::string key = argv[0];
        std::string value = "0";
        Nebulite::getRenderer()->getGlobal().set<std::string>(key.c_str(),value);
        return 0;
    }
    return 1;
}

int Nebulite::mainTreeFunctions::envload(int argc, char* argv[]){
    if(argc > 0){
        Nebulite::getRenderer()->deserialize(argv[0]);
        return 0;
    }
    else{
        // no name provided, load empty env
        Nebulite::getRenderer()->deserialize("{}");
        return 0;
    }
}

int Nebulite::mainTreeFunctions::envdeload(int argc, char* argv[]){
    Nebulite::getRenderer()->purgeObjects();
    Nebulite::getRenderer()->purgeTextures();
    return 0;
}

int Nebulite::mainTreeFunctions::spawn(int argc, char* argv[]){
    if(argc>0){
        RenderObject ro;
        ro.deserialize(argv[0]);
        auto ptr = std::make_shared<RenderObject>(std::move(ro));
        Nebulite::getRenderer()->append(ptr);
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return 1;
    }
    return 0;
}

int Nebulite::mainTreeFunctions::exitProgram(int argc, char* argv[]){
    Nebulite::getRenderer()->setQuit();
    return 0;
}

int Nebulite::mainTreeFunctions::stateLoad(int argc, char* argv[]){ 

    std::cerr << "Function load not implemented yet!" << std::endl;
    return 0;
}

int Nebulite::mainTreeFunctions::stateSave(int argc, char* argv[]){
    // <stateName>
    // Change std::string Nebulite::stateName to name
    // Check if dir ./States/stateName exists

    // If any env is deloaded, save in stateName

    // Every load of any file must be linked to state! If file exists in state load from there
    // if not, load from usual path

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

int Nebulite::mainTreeFunctions::loadTaskList(int argc, char* argv[]) {
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

int Nebulite::mainTreeFunctions::forLoop(int argc, char* argv[]){
    if(argc > 3){
        std::string varName = argv[0];
        int iStart = std::stoi(argv[1]);
        int iEnd   = std::stoi(argv[2]);

        std::string args = "";
        for (int i = 3; i < argc; ++i) {
            args += argv[i];
            if (i < argc - 1) {
                args += " ";
            }
        }

        std::string args_replaced;
        for(int i = iStart; i <= iEnd; i++){
            args_replaced = StringHandler::replaceAll(args, '$' + varName, std::to_string(i));

            int argc_new = 0;
            char** argv_new = nullptr;
            Nebulite::convertStrToArgcArgv(args_replaced, argc_new, argv_new);

            mainTree.parse(argc_new, argv_new);

            // Free only argv_new (argv_new[i] still points to argvBuffer, no need to delete[])
            delete[] argv_new;
        }
    }

    return 0;
}

int Nebulite::mainTreeFunctions::error(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {
        std::cerr << argv[i];
        if (i < argc - 1) {
            std::cerr << " ";
        }
    }
    std::cerr << std::endl;
    return 1;
}

int Nebulite::mainTreeFunctions::setResolution(int argc, char* argv[]){
    int w,h,scalar;
    w = 1000;
    h = 1000;
    scalar = 1;
    if(argc > 0){
        w = std::stoi(argv[0]);
    }
    if(argc > 1){
        h = std::stoi(argv[1]);
    }
    if(argc > 2){
        scalar = std::stoi(argv[2]);
    }
    Nebulite::getRenderer()->changeWindowSize(w,h,scalar);
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
        
        int dx = floor(std::stod(argv[0]));
        int dy = floor(std::stod(argv[1]));
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
        int x = floor(std::stod(argv[0]));
        int y = floor(std::stod(argv[1]));
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

int Nebulite::mainTreeFunctions::render_object(int argc, char** argv){
    RenderObject ro;
    FileManagement::WriteFile("./Resources/Renderobjects/standard.json",ro.serialize());
    return 0;
}