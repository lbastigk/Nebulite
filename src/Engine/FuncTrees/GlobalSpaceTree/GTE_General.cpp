#include "GTE_General.h"
#include "GlobalSpace.h"       // Global Space for Nebulite
#include "Invoke.h"            // Invoke for parsing expressions

//-------------------------------
// Update
void Nebulite::GlobalSpaceTreeExpansion::General::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//-------------------------------
// FuncTree-Bound Functions

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::eval(int argc, char* argv[]){
    // argc/argv to string for evaluation
    std::string args = "";
    for (int i = 0; i < argc; ++i) {
        args += argv[i];
        if (i < argc - 1) {
            args += " ";
        }
    }

    // eval all $(...)
    std::string args_evaled = self->invoke->evaluateStandaloneExpression(args);

    // reparse
    return funcTree->parseStr(args_evaled);
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::exitProgram(int argc, char* argv[]){
    self->getRenderer()->setQuit();
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::stateLoad(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::stateSave(int argc, char* argv[]){
    // <stateName>
    // Change std::string Nebulite::stateName to name
    // Check if dir ./States/stateName exists

    // If any env is deloaded, save in stateName

    // Every load of any file must be linked to state! If file exists in state load from there
    // if not, load from usual path

    std::cerr << "Function save not implemented yet!" << std::endl;
    return Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::wait(int argc, char* argv[]){
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

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::loadTaskList(int argc, char* argv[]) {
    std::cout << "Loading task list from file: " << (argc > 1 ? argv[1] : "none") << std::endl;

    if (argc < 2) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::string file = FileManagement::LoadFile(argv[1]);
    if (file.empty()) {
        std::cerr << "Error: "<< argv[0] <<" Could not open file '" << argv[1] << "'" << std::endl;
        return Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE;
    }

    std::vector<std::string> lines;

    // Split std::string file into lines and remove comments
    std::istringstream stream(file);
    std::string line;
    while (std::getline(stream, line)) {
        line = StringHandler::untilSpecialChar(line,'#');   // Remove comments. TODO: Doesnt remove lines that are purely "#"
        line = StringHandler::lstrip(line,' ');             // Remove whitespaces at start
        if(line.length() == 0){
            // line is empty
            continue;
        }
        else{
            // Insert line backwards, so we can process them in the order they were written later on:
            lines.insert(lines.begin(), line);
        }
    }

    // Now insert all lines into the task queue
    for (const auto& line : lines){
        self->tasks_script.taskList.push_front(line);
    }

    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::forLoop(int argc, char* argv[]){
    std::string funcName = argv[0];
    if(argc > 4){
        std::string varName = argv[1];

        int iStart = std::stoi(self->invoke->evaluateStandaloneExpression(argv[2]));
        int iEnd   = std::stoi(self->invoke->evaluateStandaloneExpression(argv[3]));

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
            funcTree->parseStr(args_replaced);
        }
    }
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::ifCondition(int argc, char* argv[]) {
    if (argc < 3) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }

    std::string result = self->invoke->evaluateStandaloneExpression(argv[1]);
    double condition_potentially_nan = std::stod(result);

    bool condition = !isnan(condition_potentially_nan) && (condition_potentially_nan != 0);

    if (!condition) {
        // If the condition is false, skip the following commands
        return Nebulite::ERROR_TYPE::NONE;
    }

    // Build the command string from rest
    std::string commands = "";
    for (int i = 2; i < argc; i++) {
        commands += argv[i];
        if (i < argc - 1) {
            commands += " ";
        }
    }
    commands = "Nebulite::GlobalSpaceTreeExpansion::General::ifCondition " + commands;
    return funcTree->parseStr(commands);
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::error(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cerr << argv[i];
        if (i < argc - 1) {
            std::cerr << " ";
        }
    }
    std::cerr << std::endl;
    return Nebulite::ERROR_TYPE::CUSTOM_ERROR;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::func_assert(int argc, char* argv[]){
    return Nebulite::ERROR_TYPE::CRITICAL_CUSTOM_ASSERT;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::func_return(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }
    return (Nebulite::ERROR_TYPE)std::stoi(argv[1]);
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::forceGlobal(int argc, char* argv[]) {
    if (argc < 3) {
        return Nebulite::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 3) {
        return Nebulite::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::string key = argv[1];
    std::string value = argv[2];
    self->getRenderer()->setForcedGlobalValue(key, value);
    return Nebulite::ERROR_TYPE::NONE;
}

Nebulite::ERROR_TYPE Nebulite::GlobalSpaceTreeExpansion::General::clearForceGlobal(int argc, char* argv[]) {
    self->getRenderer()->clearForcedGlobalValues();
    return Nebulite::ERROR_TYPE::NONE;
}