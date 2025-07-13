//------------------------------------------------
// Main Tree attached functions 
#include "Nebulite.h"

// Separate queues for script and internal
// Otherwise, a wait from a script can halt the entire game logic
// All wait calls influence script queue for now
namespace Nebulite{
    taskQueue tasks_script;
    taskQueue tasks_internal;
    taskQueue tasks_always;
    std::unique_ptr<Nebulite::Renderer> renderer = nullptr;
    Invoke invoke;

    // Create funcTree, parse special return values for usage
    // As functree needs to know what to return if:
    // - An invalid functioncall was made
    // - ... potentially more types are needed if program becomes more complex (extended to none might be a good idea)
    std::string treeName = "Nebulite";
    Nebulite::ERROR_TYPE returnValueOnInvalidFunctioncall = Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID;
    Nebulite::ERROR_TYPE returnValueOnNoError = Nebulite::ERROR_TYPE::NONE;
    FuncTree<Nebulite::ERROR_TYPE> mainTree(treeName,
        returnValueOnNoError,
        returnValueOnInvalidFunctioncall
    );


    std::unique_ptr<Nebulite::JSON> global = nullptr;
    std::string stateName;
    std::string binName;

    // used by convertStrToArgcArgv
    char* argvBuffer = nullptr;
    int argvCapacity = 0;

    // Error log
    bool errorLogStatus = false;
    std::ofstream errorFile;
    std::streambuf* originalCerrBuf = nullptr;
    
    // init variables
    void init(){
        global = std::make_unique<Nebulite::JSON>();
        invoke.linkGlobal(*global);
	    invoke.linkQueue(tasks_internal.taskList);
        stateName = "";

        tasks_always.clearAfterResolving = false;
    }

    // Init nebulite functions
    void init_functions(){

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
        mainTree.attachFunction(Nebulite::mainTreeFunctions::func_assert,     "assert",       "Force a certain return value");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::func_return,     "return",       "Returns an assert value, stopping program");
        
        // Renderer Settings
        mainTree.attachFunction(Nebulite::mainTreeFunctions::setFPS,          "set-fps",      "Sets FPS to an integer between 1 and 10000. 60 if no arg is provided");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::setResolution,   "set-res",      "Sets resolution size: [w] [h]");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::setCam,          "cam-set",      "Sets Camera position [x] [y] <c>");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::moveCam,         "cam-move",     "Moves Camera position [dx] [dy]");

        // Debug
        mainTree.attachFunction(Nebulite::mainTreeFunctions::echo,            "echo",         "Echos all args provided to cout");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::error,           "error",        "Echos all args provided to cerr");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::printGlobal,     "print-global", "Prints global doc to cout");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::printState,      "print-state",  "Prints state to cout");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::logGlobal,       "log-global",   "Logs global doc to file");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::logState,        "log-state",    "Logs state to file");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::errorlog,        "log",          "Activate/Deactivate error log");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::always,          "always",       "Attach functioncall that is executed on each tick");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::alwaysClear,     "always-clear", "Clear all always-functioncalls");

        // Helper
        mainTree.attachFunction(Nebulite::mainTreeFunctions::render_object,   "standard-render-object",  "Serializes standard renderobject to ./Resources/Renderobjects/standard.json");

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

Nebulite::taskQueueResult Nebulite::resolveTaskQueue(Nebulite::taskQueue& tq, uint64_t* counter, int* argc_mainTree, char*** argv_mainTree){
    Nebulite::ERROR_TYPE currentResult = Nebulite::ERROR_TYPE::NONE;
    Nebulite::taskQueueResult result;

    bool processedPersistentTask = false;
    while (!tq.taskList.empty() && (counter == nullptr || *counter == 0) && !result.stoppedAtCriticalResult) {
        // Get task
        std::string argStr = tq.taskList.front();

        // Pop only if configured to clear
        if (tq.clearAfterResolving) {
            tq.taskList.pop_front();
        } else if (processedPersistentTask) {
            break;  // Avoid infinite loop
        }
        processedPersistentTask = true;

        // Convert std::string to argc, argv
        *argc_mainTree = 0;
        *argv_mainTree = nullptr;

        if (!argStr.starts_with(Nebulite::binName + " ")) {
            argStr = Nebulite::binName + " " + argStr;
        }

        Nebulite::convertStrToArgcArgv(argStr, *argc_mainTree, *argv_mainTree);

        if (*argv_mainTree != nullptr && argStr.size()) {
            currentResult = Nebulite::mainTree.parse(*argc_mainTree, *argv_mainTree);
        } else {
            currentResult = Nebulite::ERROR_TYPE::CRITICAL_INVALID_ARGC_ARGV_PARSING;
        }

        if(currentResult < Nebulite::ERROR_TYPE::NONE){
            result.stoppedAtCriticalResult = true;
        }
        result.errors.push_back(currentResult);
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::eval(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::setGlobal(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::envload(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::envdeload(int argc, char* argv[]){
    Nebulite::getRenderer()->purgeObjects();
    Nebulite::getRenderer()->purgeTextures();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::spawn(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::exitProgram(int argc, char* argv[]){
    Nebulite::getRenderer()->setQuit();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::stateLoad(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::stateSave(int argc, char* argv[]){
    // <stateName>
    // Change std::string Nebulite::stateName to name
    // Check if dir ./States/stateName exists

    // If any env is deloaded, save in stateName

    // Every load of any file must be linked to state! If file exists in state load from there
    // if not, load from usual path

    std::cerr << "Function save not implemented yet!" << std::endl;
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::wait(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::loadTaskList(int argc, char* argv[]) {
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::forLoop(int argc, char* argv[]){
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
            int argc_new = 0;
            char** argv_new = nullptr;
            Nebulite::convertStrToArgcArgv(args_replaced, argc_new, argv_new);
            mainTree.parse(argc_new, argv_new);
            // Free only argv_new (argv_new[i] still points to argvBuffer, no need to delete[])
            delete[] argv_new;
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::error(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cerr << argv[i];
        if (i < argc - 1) {
            std::cerr << " ";
        }
    }
    std::cerr << std::endl;
    return Nebulite::ERROR_TYPE::CUSTOM_ERROR;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::setResolution(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::setFPS(int argc, char* argv[]){
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


Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::moveCam(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::setCam(int argc, char* argv[]){
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
    else{
        Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::printGlobal(int argc, char* argv[]){
    std::cout << Nebulite::getRenderer()->serializeGlobal() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::printState(int argc, char* argv[]){
    std::cout << Nebulite::getRenderer()->serialize() << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::logGlobal(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::logState(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::render_object(int argc, char** argv){
    RenderObject ro;
    FileManagement::WriteFile("./Resources/Renderobjects/standard.json",ro.serialize());
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::errorlog(int argc, char* argv[]){
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

// Attaches functioncall that is executed on each tick
Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::always(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::alwaysClear(int argc, char* argv[]){
    Nebulite::tasks_always.taskList.clear();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::func_assert(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_CUSTOM_ASSERT;
}

Nebulite::ERROR_TYPE Nebulite::mainTreeFunctions::func_return(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
    return (Nebulite::ERROR_TYPE)std::stoi(argv[1]);
}