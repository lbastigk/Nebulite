#include "DomainModule/GlobalSpace/GDM_General.h"
#include "Core/GlobalSpace.h"       // Global Space for Nebulite
#include "Interaction/Invoke.h"            // Invoke for parsing expressions

//------------------------------------------
// Update
void Nebulite::DomainModule::GlobalSpace::General::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// FuncTree-Bound Functions

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::eval(int argc, char* argv[]){
    // argc/argv to string for evaluation
    std::string args = "";
    for (int i = 0; i < argc; ++i) {
        args += argv[i];
        if (i < argc - 1) {
            args += " ";
        }
    }

    // eval all $(...)
    std::string args_evaled = domain->invoke->evaluateStandaloneExpression(args);

    // reparse
    return funcTree->parseStr(args_evaled);
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::exitProgram(int argc, char* argv[]){
    domain->getRenderer()->setQuit();
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::wait(int argc, char* argv[]){
    if(argc == 2){
        std::istringstream iss(argv[1]);
        iss >> domain->scriptWaitCounter;
        if (domain->scriptWaitCounter < 0){
            domain->scriptWaitCounter = 0;
        }
        return Nebulite::Constants::ERROR_TYPE::NONE;
    }
    else if(argc < 2){
       return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    else{
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::loadTaskList(int argc, char* argv[]) {
    std::cout << "Loading task list from file: " << (argc > 1 ? argv[1] : "none") << std::endl;

    if (argc < 2) {
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::string file = Nebulite::Utility::FileManagement::LoadFile(argv[1]);
    if (file.empty()) {
        std::cerr << "Error: "<< argv[0] <<" Could not open file '" << argv[1] << "'" << std::endl;
        return Nebulite::Constants::ERROR_TYPE::CRITICAL_INVALID_FILE;
    }

    std::vector<std::string> lines;

    // Split std::string file into lines and remove comments
    std::istringstream stream(file);
    std::string line;
    while (std::getline(stream, line)) {
        line = Nebulite::Utility::StringHandler::untilSpecialChar(line,'#');   // Remove comments. TODO: Doesnt remove lines that are purely "#"
        line = Nebulite::Utility::StringHandler::lstrip(line,' ');             // Remove whitespaces at start
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
        domain->tasks.script.taskList.push_front(line);
    }

    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::forLoop(int argc, char* argv[]){
    std::string funcName = argv[0];
    if(argc > 4){
        std::string varName = argv[1];

        int iStart = std::stoi(domain->invoke->evaluateStandaloneExpression(argv[2]));
        int iEnd   = std::stoi(domain->invoke->evaluateStandaloneExpression(argv[3]));

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
            args_replaced = funcName + " " + Nebulite::Utility::StringHandler::replaceAll(args, '$' + varName, std::to_string(i));
            funcTree->parseStr(args_replaced);
        }
    }
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::ifCondition(int argc, char* argv[]) {
    if (argc < 3) {
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }

    std::string result = domain->invoke->evaluateStandaloneExpression(argv[1]);
    double condition_potentially_nan = std::stod(result);

    bool condition = !isnan(condition_potentially_nan) && (condition_potentially_nan != 0);

    if (!condition) {
        // If the condition is false, skip the following commands
        return Nebulite::Constants::ERROR_TYPE::NONE;
    }

    // Build the command string from rest
    std::string commands = "";
    for (int i = 2; i < argc; i++) {
        commands += argv[i];
        if (i < argc - 1) {
            commands += " ";
        }
    }
    commands = "Nebulite::DomainModule::GlobalSpace::General::ifCondition " + commands;
    return funcTree->parseStr(commands);
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::func_assert(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }

    if (argc > 2) {
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::string condition = argv[1];

    if(!std::stod(domain->invoke->evaluateStandaloneExpression(condition))){
        return Nebulite::Constants::ERROR_TYPE::CRITICAL_CUSTOM_ASSERT;
    }
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::func_return(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }
    return (Nebulite::Constants::ERROR_TYPE)std::stoi(argv[1]);
}

//------------------------------------------
// To move

// 1.) To GDM_Debug

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::error(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cerr << argv[i];
        if (i < argc - 1) {
            std::cerr << " ";
        }
    }
    std::cerr << std::endl;
    return Nebulite::Constants::ERROR_TYPE::CUSTOM_ERROR;
}

// 2.) To GDM_StateManagement

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::stateLoad(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::stateSave(int argc, char* argv[]){
    // <stateName>
    // Change std::string Nebulite::stateName to name
    // Check if dir ./States/stateName exists

    // If any env is deloaded, save in stateName

    // Every load of any file must be linked to state! If file exists in state load from there
    // if not, load from usual path

    std::cerr << "Function save not implemented yet!" << std::endl;
    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}

// 3.) To GDM_InputMapping

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::forceGlobal(int argc, char* argv[]) {
    if (argc < 3) {
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 3) {
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }

    std::string key = argv[1];
    std::string value = argv[2];
    domain->getRenderer()->setForcedGlobalValue(key, value);
    return Nebulite::Constants::ERROR_TYPE::NONE;
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::General::clearForceGlobal(int argc, char* argv[]) {
    domain->getRenderer()->clearForcedGlobalValues();
    return Nebulite::Constants::ERROR_TYPE::NONE;
}