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
        mainTree.attachFunction(Nebulite::mainTreeFunctions::save,            "state-save",   "Saves the state");
        mainTree.attachFunction(Nebulite::mainTreeFunctions::load,            "state-load",   "Loads a state");
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

        // Tests
        mainTree.attachFunction(Nebulite::mainTreeFunctions::json_test,"json-test","Testing new json wrapper");
    }


    Renderer* getRenderer() {
        if (!renderer) {
            renderer = std::make_unique<Renderer>(invoke,*global);
            renderer->setFPS(60);
        }
        return renderer.get();
    }
}
/*
void Nebulite::convertStrToArgcArgv(const std::string& cmd, int& argc, char**& argv) {
    // Free previous buffer if any
    if (argvBuffer) {
        delete[] argvBuffer;
        argvBuffer = nullptr;
        argvCapacity = 0;
    }

    // Allocate new buffer
    argvCapacity = static_cast<int>(cmd.size()) + 1;
    argvBuffer = new char[argvCapacity];
    std::memcpy(argvBuffer, cmd.c_str(), argvCapacity);

    // Tokenize
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

    // Allocate argv (deep copy of argvVec)
    argv = new char*[argc + 1];
    for (int i = 0; i < argc; ++i) {
        argv[i] = argvVec[i];
    }
    argv[argc] = nullptr; // Null terminator
}

*/


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
        std::cout << "Loading env: " << argv[0] << std::endl;
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
        //std::cout << "Spawning object: " << argv[0] << std::endl;
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


int Nebulite::mainTreeFunctions::json_test(int argc, char** argv){
    uint64_t start;
    //*
    uint64_t count = 10000000;

    std::cout << "Testing JSON performance in setting values." << std::endl;
    std::cout << "Set count is: " << count << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    // Version 1: old wrapper
    rapidjson::Document doc;
    JSONHandler::Set::Any<double>(doc,"global.time.t",1.2345);

    // Version 2: new wrapper with cache
    Nebulite::JSON json;
    json.set<double>("global.time.t",1.2345);

    //------------------------------------------------------------------------
    std::cout << std::endl;
    std::cout << "Test 1: Setting a nested double value from old doc" << std::endl;
    start = Time::gettime();
    for(volatile uint64_t i = 0; i < count; i++){
        (void) JSONHandler::Set::Any<double>(doc,"global.time.t",1.2345);
    }
    std::cout << "\t Took " << Time::getruntime(start) << std::endl;

    std::cout << std::endl;
    std::cout << "Test 2: Setting a nested double value from new wrapper with caching" << std::endl;
    start = Time::gettime();
    for(volatile uint64_t i = 0; i < count; i++){
        (void) json.set<double>("global.time.t",1.2345);
    }
    std::cout << "\t Took " << Time::getruntime(start) << std::endl;

    //------------------------------------------------------------------------
    std::cout << std::endl;
    std::cout << "Test 3: Getting a nested double value from old doc" << std::endl;
    start = Time::gettime();
    for(volatile uint64_t i = 0; i < count; i++){
        (void) JSONHandler::Get::Any<double>(doc,"global.time.t",1.2345);
    }
    std::cout << "\t Took " << Time::getruntime(start) << std::endl;

    std::cout << std::endl;
    std::cout << "Test 4: Getting a nested double value from new wrapper with caching" << std::endl;
    start = Time::gettime();
    for(volatile uint64_t i = 0; i < count; i++){
        (void) json.get<double>("global.time.t",0);
    }
    std::cout << "\t Took " << Time::getruntime(start) << std::endl;

    std::string doc1Str = JSONHandler::serialize(doc);
    std::string doc2Str = json.serialize();
    std::cout << std::endl;
    std::cout << "Comparing docs:" << std::endl;
    if (doc1Str == doc2Str) {
        std::cout << "✅ JSON documents match!" << std::endl;
    } else {
        std::cout << "❌ JSON documents differ!" << std::endl;
        std::cout << doc1Str << std::endl;
        std::cout << doc2Str << std::endl;
    }
    //*/

    std::cout << std::endl;
    std::cout << "Test 5: Array setting and getting" << std::endl;
    Nebulite::JSON json2;
    json2.set<double>("global.value1",1337);
    json2.set<double>("global.arr[2]",1.2345);
    std::cout << json2.serialize() << std::endl; 
    std::cout << "global.value1 = " << json2.get<double>("global.value1",0.0) << std::endl;
    std::cout << "global.arr[0] = " << json2.get<double>("global.arr[0]",0.0) << std::endl;
    std::cout << "global.arr[1] = " << json2.get<double>("global.arr[1]",0.0) << std::endl;
    std::cout << "global.arr[2] = " << json2.get<double>("global.arr[2]",0.0) << std::endl;

    std::cout << std::endl;
    std::cout << "Test 6: Setting and getting values from RenderObject" << std::endl;
    RenderObject obj;
    std::cout << "\tValues in Cache: " << obj.getDoc()->size_cache() << std::endl;
    for (int i = 0; i < 10; i++){
        obj.valueSet<double>(namenKonvention.renderObject.positionX.c_str(),(double)i * 0.1);
        std::cout << "\t" << namenKonvention.renderObject.positionX << " = " << obj.valueGet<double>(namenKonvention.renderObject.positionX.c_str()) << std::endl;
    }
    std::cout << "\tValues in Cache: " << obj.getDoc()->size_cache() << std::endl;

    return 0;
}