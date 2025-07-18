#include "FuncTrees/MainFuncTree.h"

#include "Nebulite.h"
// TODO: Add depth to mainTree:
/*



// Functions kept in maintree for ease of use:
eval
spawn
for
wait
always

// Splitting into a tree:
system      echo  
            error
debug       print
            log
global      set
            print
            log

state       load
            set
            print
            log

renderer    set-fps
            cam-set
            cam-move

debug       standard-render-object
*/

// Helper function
/*
template<typename TClass, typename TEnum>
void bind(FuncTree<TEnum>& tree,
                          TClass* instance,
                          TEnum (TClass::*method)(int, char**),
                          const std::string& name,
                          const std::string& description) {
    tree.attachFunction(
        [instance, method](int argc, char** argv) {
            return (instance->*method)(argc, argv);
        },
        name,
        description
    );
}
*/


Nebulite::MainFuncTree::MainFuncTree(Nebulite::Invoke* invoke)
    : FuncTreeWrapper("MainFuncTree", Nebulite::ERROR_TYPE::NONE, Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID) {
    
    invoke_ptr = invoke;
    
    // General
    bind(funcTree, this, &MainFuncTree::eval,            "eval",         "Evaluate all $(...) after this keyword, parse rest as usual");
    bind(funcTree, this, &MainFuncTree::setGlobal,       "set-global",   "Set any global variable: [key] [value]");
    bind(funcTree, this, &MainFuncTree::envload,         "env-load",     "Loads an environment");
    bind(funcTree, this, &MainFuncTree::envdeload,       "env-deload",   "Deloads an environment");
    bind(funcTree, this, &MainFuncTree::spawn,           "spawn",        "Spawn a renderobject");
    bind(funcTree, this, &MainFuncTree::exitProgram,     "exit",         "exits the program");
    bind(funcTree, this, &MainFuncTree::stateSave,       "state-save",   "Saves the state");
    bind(funcTree, this, &MainFuncTree::stateLoad,       "state-load",   "Loads a state");
    bind(funcTree, this, &MainFuncTree::loadTaskList,    "task",         "Loads a txt file of tasks");
    bind(funcTree, this, &MainFuncTree::wait,            "wait",         "Halt all commands for a set amount of frames");
    bind(funcTree, this, &MainFuncTree::forLoop,         "for",          "Start for-loop. Usage: for var <iStart> <iEnd> command $var");
    bind(funcTree, this, &MainFuncTree::func_assert,     "assert",       "Force a certain return value");
    bind(funcTree, this, &MainFuncTree::func_return,     "return",       "Returns an assert value, stopping program");
    
    // Renderer Settings
    bind(funcTree, this, &MainFuncTree::setFPS,          "set-fps",      "Sets FPS to an integer between 1 and 10000. 60 if no arg is provided");
    bind(funcTree, this, &MainFuncTree::setResolution,   "set-res",      "Sets resolution size: [w] [h]");
    bind(funcTree, this, &MainFuncTree::setCam,          "cam-set",      "Sets Camera position [x] [y] <c>");
    bind(funcTree, this, &MainFuncTree::moveCam,         "cam-move",     "Moves Camera position [dx] [dy]");

    // Debug
    bind(funcTree, this, &MainFuncTree::echo,            "echo",         "Echos all args provided to cout");
    bind(funcTree, this, &MainFuncTree::error,           "error",        "Echos all args provided to cerr");
    bind(funcTree, this, &MainFuncTree::printGlobal,     "print-global", "Prints global doc to cout");
    bind(funcTree, this, &MainFuncTree::printState,      "print-state",  "Prints state to cout");
    bind(funcTree, this, &MainFuncTree::logGlobal,       "log-global",   "Logs global doc to file");
    bind(funcTree, this, &MainFuncTree::logState,        "log-state",    "Logs state to file");
    bind(funcTree, this, &MainFuncTree::errorlog,        "log",          "Activate/Deactivate error log");
    bind(funcTree, this, &MainFuncTree::always,          "always",       "Attach functioncall that is executed on each tick");
    bind(funcTree, this, &MainFuncTree::alwaysClear,     "always-clear", "Clear all always-functioncalls");

    // Helper
    bind(funcTree, this, &MainFuncTree::render_object,   "standard-render-object",  "Serializes standard renderobject to ./Resources/Renderobjects/standard.json");

    // Internal Tests
    bind(funcTree, this, &MainFuncTree::printVar,        "print-var",   "Prints the value of the test variable");
    attachVariable(&testVar, "testVar", "Test variable for printing");
}


//---------------------------------------------------------------
// Main Tree Functions

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::eval(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::setGlobal(int argc, char* argv[]){
    if(argc == 3){
        std::string key = argv[1];
        std::string value = argv[2];
        Nebulite::getRenderer()->getGlobal().set<std::string>(key.c_str(),value);
        return Nebulite::ERROR_TYPE::NONE;
    }
    if(argc == 2){
        std::string key = argv[1];
        std::string value = "0";
        Nebulite::getRenderer()->getGlobal().set<std::string>(key.c_str(),value);
        return Nebulite::ERROR_TYPE::NONE;
    }
    if(argc < 2){
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    else{
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::envload(int argc, char* argv[]){
    if(argc > 1){
        Nebulite::getRenderer()->deserialize(argv[1]);
        return Nebulite::ERROR_TYPE::NONE;
    }
    else{
        // no name provided, load empty env
        Nebulite::getRenderer()->deserialize("{}");
        return Nebulite::ERROR_TYPE::NONE;
    }
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::envdeload(int argc, char* argv[]){
    Nebulite::getRenderer()->purgeObjects();
    Nebulite::getRenderer()->purgeTextures();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::spawn(int argc, char* argv[]){
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
        Nebulite::getRenderer()->append(ro);
    }
    else{
        std::cerr << "No renderobject name provided!" << std::endl;
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::exitProgram(int argc, char* argv[]){
    Nebulite::getRenderer()->setQuit();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::stateLoad(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::stateSave(int argc, char* argv[]){
    // <stateName>
    // Change std::string Nebulite::stateName to name
    // Check if dir ./States/stateName exists

    // If any env is deloaded, save in stateName

    // Every load of any file must be linked to state! If file exists in state load from there
    // if not, load from usual path

    std::cerr << "Function save not implemented yet!" << std::endl;
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::wait(int argc, char* argv[]){
    if(argc == 2){
        std::istringstream iss(argv[1]);
        iss >> tasks_script.waitCounter;
        if (tasks_script.waitCounter < 0){
            tasks_script.waitCounter = 0;
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

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::loadTaskList(int argc, char* argv[]) {
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
            tasks_script.taskList.push_back(line);
        }
    }

    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::forLoop(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::error(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cerr << argv[i];
        if (i < argc - 1) {
            std::cerr << " ";
        }
    }
    std::cerr << std::endl;
    return Nebulite::ERROR_TYPE::CUSTOM_ERROR;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::setResolution(int argc, char* argv[]){
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
    Nebulite::getRenderer()->changeWindowSize(w,h,scalar);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::setFPS(int argc, char* argv[]){
    if(argc != 2){
        Nebulite::getRenderer()->setFPS(60);
    }
    else{
        int fps = std::stoi(argv[1]);
        if(fps < 1) fps=1;
        if(fps > 10000) fps=10000;
        Nebulite::getRenderer()->setFPS(fps);
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::moveCam(int argc, char* argv[]){
    if (argc < 3) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 3) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }

    int dx = floor(std::stod(argv[1]));
    int dy = floor(std::stod(argv[2]));
    Nebulite::getRenderer()->moveCam(dx,dy);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::setCam(int argc, char* argv[]){
    if(argc == 3){
        int x = floor(std::stod(argv[1]));
        int y = floor(std::stod(argv[2]));
        Nebulite::getRenderer()->setCam(x,y);
        return Nebulite::ERROR_TYPE::NONE;
    }
    if(argc == 4){
        if(!strcmp(argv[3], "c")){
            int x = std::stoi(argv[1]);
            int y = std::stoi(argv[2]);
            Nebulite::getRenderer()->setCam(x,y,true);
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

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::printGlobal(int argc, char* argv[]){
    std::cout << Nebulite::getRenderer()->serializeGlobal() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::printState(int argc, char* argv[]){
    std::cout << Nebulite::getRenderer()->serialize() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::logGlobal(int argc, char* argv[]){
    std::string serialized = Nebulite::getRenderer()->serializeGlobal();
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

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::logState(int argc, char* argv[]){
    std::string serialized = Nebulite::getRenderer()->serialize();
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

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::render_object(int argc, char** argv){
    RenderObject ro;
    FileManagement::WriteFile("./Resources/Renderobjects/standard.json",ro.serialize());
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::errorlog(int argc, char* argv[]){
    if(argc == 2){
        if(!strcmp(argv[1], "on")){
            if(!Nebulite::errorLogStatus){
                // Log errors in separate file
                Nebulite::errorFile.open("errors.log");
                if (!Nebulite::errorFile) {
                    std::cerr << "Failed to open error file." << std::endl;
                    return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
                }
                Nebulite::originalCerrBuf = std::cerr.rdbuf(); // Store the original cerr buffer
                std::cerr.rdbuf(Nebulite::errorFile.rdbuf());
                Nebulite::errorLogStatus = true;
            }
        }
        else if (!strcmp(argv[1], "off")){
            if(Nebulite::errorLogStatus){
                // Close error log
                std::cerr.flush();                              // Explicitly flush std::cerr before closing the file stream. Ensures everything is written to the file
                std::cerr.rdbuf(Nebulite::originalCerrBuf);     // Restore the original buffer to std::cerr (important for cleanup)
                Nebulite::errorFile.close();
                Nebulite::errorLogStatus = false;
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

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::always(int argc, char* argv[]){
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
                Nebulite::tasks_always.taskList.push_back(command);
            }
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::alwaysClear(int argc, char* argv[]){
    Nebulite::tasks_always.taskList.clear();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::func_assert(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_CUSTOM_ASSERT;
}

Nebulite::ERROR_TYPE Nebulite::MainFuncTree::func_return(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
    return (Nebulite::ERROR_TYPE)std::stoi(argv[1]);
}


Nebulite::ERROR_TYPE Nebulite::MainFuncTree::printVar(int argc, char** argv){
    std::cout << testVar << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}