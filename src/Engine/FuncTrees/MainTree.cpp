#include "MainTree.h"
#include "GlobalSpace.h"

// TODO: Add depth to mainTree:
/*
Example:
MainTree
    - system
        - echo
        - error
        - assert
        - return
    - debug
        - print
        - log
        - standard-render-object
        - always
        - always-clear
    - global
        - set
        - print
        - log
    - state
        - load
        - set
        - print
        - log
    - renderer
        - set-fps
        - cam-set
        - cam-move
*/



Nebulite::MainTree::MainTree(Nebulite::Invoke* invoke, Nebulite::GlobalSpace* globalSpace)
    : FuncTreeWrapper("Nebulite", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID) {

    invoke_ptr = invoke;
    self = globalSpace;

    // General
    bindFunction(funcTree, this, &MainTree::eval,            "eval",         "Evaluate all $(...) after this keyword, parse rest as usual");
    bindFunction(funcTree, this, &MainTree::setGlobal,       "set-global",   "Set any global variable: [key] [value]");
    bindFunction(funcTree, this, &MainTree::envload,         "env-load",     "Loads an environment");
    bindFunction(funcTree, this, &MainTree::envdeload,       "env-deload",   "Deloads an environment");
    bindFunction(funcTree, this, &MainTree::spawn,           "spawn",        "Spawn a renderobject");
    bindFunction(funcTree, this, &MainTree::exitProgram,     "exit",         "exits the program");
    bindFunction(funcTree, this, &MainTree::stateSave,       "state-save",   "Saves the state");
    bindFunction(funcTree, this, &MainTree::stateLoad,       "state-load",   "Loads a state");
    bindFunction(funcTree, this, &MainTree::loadTaskList,    "task",         "Loads a txt file of tasks");
    bindFunction(funcTree, this, &MainTree::wait,            "wait",         "Halt all commands for a set amount of frames");
    bindFunction(funcTree, this, &MainTree::forLoop,         "for",          "Start for-loop. Usage: for var <iStart> <iEnd> command $var");
    bindFunction(funcTree, this, &MainTree::func_assert,     "assert",       "Force a certain return value");
    bindFunction(funcTree, this, &MainTree::func_return,     "return",       "Returns an assert value, stopping program");
    
    // Renderer Settings
    bindFunction(funcTree, this, &MainTree::setFPS,          "set-fps",      "Sets FPS to an integer between 1 and 10000. 60 if no arg is provided");
    bindFunction(funcTree, this, &MainTree::setResolution,   "set-res",      "Sets resolution size: [w] [h]");
    bindFunction(funcTree, this, &MainTree::setCam,          "cam-set",      "Sets Camera position [x] [y] <c>");
    bindFunction(funcTree, this, &MainTree::moveCam,         "cam-move",     "Moves Camera position [dx] [dy]");

    // Debug
    bindFunction(funcTree, this, &MainTree::echo,            "echo",         "Echos all args provided to cout");
    bindFunction(funcTree, this, &MainTree::error,           "error",        "Echos all args provided to cerr");
    bindFunction(funcTree, this, &MainTree::printGlobal,     "print-global", "Prints global doc to cout");
    bindFunction(funcTree, this, &MainTree::printState,      "print-state",  "Prints state to cout");
    bindFunction(funcTree, this, &MainTree::logGlobal,       "log-global",   "Logs global doc to file");
    bindFunction(funcTree, this, &MainTree::logState,        "log-state",    "Logs state to file");
    bindFunction(funcTree, this, &MainTree::errorlog,        "log",          "Activate/Deactivate error log");
    bindFunction(funcTree, this, &MainTree::always,          "always",       "Attach functioncall that is executed on each tick");
    bindFunction(funcTree, this, &MainTree::alwaysClear,     "always-clear", "Clear all always-functioncalls");

    // Helper
    bindFunction(funcTree, this, &MainTree::render_object,   "standard-render-object",  "Serializes standard renderobject to ./Resources/Renderobjects/standard.json");

    // Internal Tests
    bindFunction(funcTree, this, &MainTree::printVar,        "print-var",   "Prints the value of the test variable");
    bindVariable(&testVar, "testVar", "Test variable for printing");
}


//---------------------------------------------------------------
// Main Tree Functions

Nebulite::ERROR_TYPE Nebulite::MainTree::eval(int argc, char* argv[]){
    // argc/argv to string for evaluation
    std::string args = "";
    for (int i = 0; i < argc; ++i) {
        args += argv[i];
        if (i < argc - 1) {
            args += " ";
        }
    }

    // eval all $(...)
    std::string args_evaled = invoke_ptr->resolveGlobalVars(args);

    // reparse
    return funcTree.parseStr(args_evaled);
}

Nebulite::ERROR_TYPE Nebulite::MainTree::setGlobal(int argc, char* argv[]){
    if(argc == 3){
        std::string key = argv[1];
        std::string value = argv[2];
        self->getRenderer()->getGlobal().set<std::string>(key.c_str(),value);
        return Nebulite::ERROR_TYPE::NONE;
    }
    if(argc == 2){
        std::string key = argv[1];
        std::string value = "0";
        self->getRenderer()->getGlobal().set<std::string>(key.c_str(),value);
        return Nebulite::ERROR_TYPE::NONE;
    }
    if(argc < 2){
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    else{
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
}

Nebulite::ERROR_TYPE Nebulite::MainTree::envload(int argc, char* argv[]){
    if(argc > 1){
        self->getRenderer()->deserialize(argv[1]);
        return Nebulite::ERROR_TYPE::NONE;
    }
    else{
        // no name provided, load empty env
        self->getRenderer()->deserialize("{}");
        return Nebulite::ERROR_TYPE::NONE;
    }
}

Nebulite::ERROR_TYPE Nebulite::MainTree::envdeload(int argc, char* argv[]){
    self->getRenderer()->purgeObjects();
    self->getRenderer()->purgeTextures();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::spawn(int argc, char* argv[]){
    if(argc>1){
        std::string linkOrObject = argv[1];

        // Check if argv1 provided is an object
        if(linkOrObject.starts_with('{')){
            std::cerr << "Object-passing to spawn is currently not supported" << std::endl;
            return Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED;
        }

        // [TODO] Add standard-directories to find files in:
        // spawn Planets/sun.json -> spawn ./Resources/Renderobjects/Planets/sun.json
        // Note that the link cant be turned into a serial here, 
        // due to additional passings like |posX=100
        // that are resolved in Renderobject::deserialize / JSON::deserialize

        // Create object
        RenderObject* ro = new RenderObject;
        ro->deserialize(linkOrObject);
        self->getRenderer()->append(ro);
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::exitProgram(int argc, char* argv[]){
    self->getRenderer()->setQuit();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::stateLoad(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::stateSave(int argc, char* argv[]){
    // <stateName>
    // Change std::string Nebulite::stateName to name
    // Check if dir ./States/stateName exists

    // If any env is deloaded, save in stateName

    // Every load of any file must be linked to state! If file exists in state load from there
    // if not, load from usual path

    std::cerr << "Function save not implemented yet!" << std::endl;
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::wait(int argc, char* argv[]){
    if(argc == 2){
        std::istringstream iss(argv[1]);
        iss >> self->tasks_script.waitCounter;
        if (self->tasks_script.waitCounter < 0){
            self->tasks_script.waitCounter = 0;
        }
        return Nebulite::ERROR_TYPE::NONE;
    }
    else if(argc < 2){
       return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    else{
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
}

// TODO: Tasks should be added at the current queue position, not at the end!
// Adding line backwards to front of the queue should fix this,
// but we need to make sure that the queue is processed in a way that allows for manipulation
// of the queue while processing it, especially at the front.
Nebulite::ERROR_TYPE Nebulite::MainTree::loadTaskList(int argc, char* argv[]) {
    if (argc < 2) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::ifstream infile(argv[1]);
    if (!infile) {
        std::cerr << "Error: "<< argv[0] <<" Could not open file '" << argv[1] << "'" << std::endl;
        return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
    }
    
    std::string line;
    while (std::getline(infile, line)) {
        line = StringHandler::untilSpecialChar(line,'#');   // Remove comments
        line = StringHandler::lstrip(line,' ');             // Remove whitespaces at start
        if(line.length() == 0){
            // line is empty
            continue;
        }
        else{
            self->tasks_script.taskList.push_back(line);
        }
    }

    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::forLoop(int argc, char* argv[]){
    std::string funcName = argv[0];
    if(argc > 4){
        std::string varName = argv[1];
        int iStart = std::stoi(argv[2]);
        int iEnd   = std::stoi(argv[3]);

        std::string args = "";
        for (int i = 4; i < argc; ++i) {
            args += argv[i];
            if (i < argc - 1) {
                args += " ";
            }
        }
        std::string args_replaced;
        for(int i = iStart; i <= iEnd; i++){
            // for + args
            args_replaced = funcName + " " + StringHandler::replaceAll(args, '$' + varName, std::to_string(i));
            funcTree.parseStr(args_replaced);
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::error(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cerr << argv[i];
        if (i < argc - 1) {
            std::cerr << " ";
        }
    }
    std::cerr << std::endl;
    return Nebulite::ERROR_TYPE::CUSTOM_ERROR;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::setResolution(int argc, char* argv[]){
    int w,h,scalar;
    w = 1000;
    h = 1000;
    scalar = 1;
    if(argc > 1){
        w = std::stoi(argv[1]);
    }
    if(argc > 2){
        h = std::stoi(argv[2]);
    }
    if(argc > 3){
        scalar = std::stoi(argv[3]);
    }
    self->getRenderer()->changeWindowSize(w,h,scalar);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::setFPS(int argc, char* argv[]){
    if(argc != 2){
        self->getRenderer()->setFPS(60);
    }
    else{
        int fps = std::stoi(argv[1]);
        if(fps < 1) fps=1;
        if(fps > 10000) fps=10000;
        self->getRenderer()->setFPS(fps);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::moveCam(int argc, char* argv[]){
    if (argc < 3) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 3) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }

    int dx = floor(std::stod(argv[1]));
    int dy = floor(std::stod(argv[2]));
    self->getRenderer()->moveCam(dx,dy);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::setCam(int argc, char* argv[]){
    if(argc == 3){
        int x = floor(std::stod(argv[1]));
        int y = floor(std::stod(argv[2]));
        self->getRenderer()->setCam(x,y);
        return Nebulite::ERROR_TYPE::NONE;
    }
    if(argc == 4){
        if(!strcmp(argv[3], "c")){
            int x = std::stoi(argv[1]);
            int y = std::stoi(argv[2]);
            self->getRenderer()->setCam(x,y,true);
            return Nebulite::ERROR_TYPE::NONE;
        }
        else{
            // unknown arg
            return Nebulite::ERROR_TYPE::UNKNOWN_ARG;
        }
    }
    else if(argc > 4){
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
    return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::printGlobal(int argc, char* argv[]){
    std::cout << self->getRenderer()->serializeGlobal() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::printState(int argc, char* argv[]){
    std::cout << self->getRenderer()->serialize() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::logGlobal(int argc, char* argv[]){
    std::string serialized = self->getRenderer()->serializeGlobal();
    if (argc>1){
        for(int i=1; i < argc; i++){
            FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        FileManagement::WriteFile("global.log.json",serialized);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::logState(int argc, char* argv[]){
    std::string serialized = self->getRenderer()->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        FileManagement::WriteFile("state.log.json",serialized);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::render_object(int argc, char** argv){
    RenderObject ro;
    FileManagement::WriteFile("./Resources/Renderobjects/standard.json",ro.serialize());
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::errorlog(int argc, char* argv[]){
    if(argc == 2){
        if(!strcmp(argv[1], "on")){
            if(!self->errorLogStatus){
                try {
                    // Create ofstream only when needed (lazy initialization)
                    if (!self->errorFile) {
                        self->errorFile = std::make_unique<std::ofstream>();
                    }
                    
                    // Log errors in separate file
                    self->errorFile->open("errors.log");
                    if (!(*self->errorFile)) {
                        std::cerr << "Failed to open error file." << std::endl;
                        return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
                    }
                    
                    self->originalCerrBuf = std::cerr.rdbuf(); // Store the original cerr buffer
                    std::cerr.rdbuf(self->errorFile->rdbuf()); // Redirect to file
                    self->errorLogStatus = true;
                    
                } catch (const std::exception& e) {
                    std::cerr << "Failed to create error log: " << e.what() << std::endl;
                    return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
                } catch (...) {
                    std::cerr << "Failed to create error log: unknown error" << std::endl;
                    return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
                }
            }
        }
        else if (!strcmp(argv[1], "off")){
            if(self->errorLogStatus){
                // Close error log
                std::cerr.flush();                           // Flush before restoring
                std::cerr.rdbuf(self->originalCerrBuf);     // Restore the original buffer
                
                if (self->errorFile) {
                    self->errorFile->close();
                    // Keep the unique_ptr for potential reuse
                }
                
                self->errorLogStatus = false;
            }
        } 
    }
    else{
        if(argc > 2){
            return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
        }
        else{
            return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::always(int argc, char* argv[]){
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string argStr = oss.str();
        std::stringstream ss(argStr);
        std::string command;

        while (std::getline(ss, command, ';')) {
            // Trim whitespace from each command
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);
            if (!command.empty()) {
                self->tasks_always.taskList.push_back(command);
            }
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::alwaysClear(int argc, char* argv[]){
    self->tasks_always.taskList.clear();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::func_assert(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_CUSTOM_ASSERT;
}

Nebulite::ERROR_TYPE Nebulite::MainTree::func_return(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
    return (Nebulite::ERROR_TYPE)std::stoi(argv[1]);
}


Nebulite::ERROR_TYPE Nebulite::MainTree::printVar(int argc, char** argv){
    std::cout << testVar << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}