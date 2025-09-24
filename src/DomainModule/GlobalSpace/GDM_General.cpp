#include "DomainModule/GlobalSpace/GDM_General.hpp"
#include "Core/GlobalSpace.hpp"       // Global Space for Nebulite
#include "Interaction/Invoke.hpp"            // Invoke for parsing expressions

//------------------------------------------
// Update
void Nebulite::DomainModule::GlobalSpace::General::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// FuncTree-Bound Functions

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::eval(int argc, char* argv[]){
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

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::exitProgram(int argc, char* argv[]){
    // Clear all task queues to prevent further execution
    domain->tasks.script.taskList.clear();
    domain->tasks.internal.taskList.clear();
    domain->tasks.always.taskList.clear();

    // Set the renderer to quit
    domain->getRenderer()->setQuit();
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::wait(int argc, char* argv[]){
    if(argc == 2){
        std::istringstream iss(argv[1]);
        iss >> domain->scriptWaitCounter;
        if (domain->scriptWaitCounter < 0){
            domain->scriptWaitCounter = 0;
        }
        return Nebulite::Constants::ErrorTable::NONE();
    }
    else if(argc < 2){
       return Nebulite::Constants::ErrorTable::FUNCTIONALL::TOO_FEW_ARGS();
    }
    else{
        return Nebulite::Constants::ErrorTable::FUNCTIONALL::TOO_MANY_ARGS();
    }
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::loadTaskList(int argc, char* argv[]) {
    std::cout << "Loading task list from file: " << (argc > 1 ? argv[1] : "none") << std::endl;

    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONALL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONALL::TOO_MANY_ARGS();
    }

    std::string file = Nebulite::Utility::FileManagement::LoadFile(argv[1]);
    if (file.empty()) {
        std::cerr << "Error: "<< argv[0] <<" Could not open file '" << argv[1] << "'" << std::endl;
        return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
    }

    std::vector<std::string> lines;

    // Split std::string file into lines and remove comments
    std::istringstream stream(file);
    std::string line;
    while (std::getline(stream, line)) {
        line = Nebulite::Utility::StringHandler::untilSpecialChar(line,'#');   // Remove comments.
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
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cout << argv[i];
        if (i < argc - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::forLoop(int argc, char* argv[]){
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
            args_replaced = funcName + " " + Nebulite::Utility::StringHandler::replaceAll(args, '{' + varName + '}', std::to_string(i));
            funcTree->parseStr(args_replaced);
        }
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::ifCondition(int argc, char* argv[]) {
    if (argc < 3) {
        return Nebulite::Constants::ErrorTable::FUNCTIONALL::TOO_FEW_ARGS();
    }

    std::string result = domain->invoke->evaluateStandaloneExpression(argv[1]);
    double condition_potentially_nan = std::stod(result);

    bool condition = !isnan(condition_potentially_nan) && (condition_potentially_nan != 0);

    if (!condition) {
        // If the condition is false, skip the following commands
        return Nebulite::Constants::ErrorTable::NONE();
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

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::func_assert(int argc, char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONALL::TOO_FEW_ARGS();
    }

    if (argc > 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONALL::TOO_MANY_ARGS();
    }

    std::string condition = argv[1];

    // condition must start with $( and end with )
    if (condition.front() != '$' || condition[1] != '(' || condition.back() != ')') {
        return Nebulite::Constants::ErrorTable::FUNCTIONALL::UNKNOWN_ARG();
    }

    // Evaluate condition
    if(!std::stod(domain->invoke->evaluateStandaloneExpression(condition))){
        return Nebulite::Constants::ErrorTable::CRITICAL_CUSTOM_ASSERT();
    }

    // All good
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::func_return(int argc, char* argv[]){
    std::string str = "";
    for(int i = 1; i < argc; ++i){
        str += argv[i];
        if(i < argc - 1){
            str += " ";
        }
    }
    return Nebulite::Constants::ErrorTable::addError(str, Nebulite::Constants::Error::CRITICAL);
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::always(int argc, char* argv[]){
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
                domain->tasks.always.taskList.push_back(command);
            }
        }
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::alwaysClear(int argc, char* argv[]){
    domain->tasks.always.taskList.clear();
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// To move


// To GDM_StateManagement

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::stateLoad(int argc, char* argv[]){ 
    std::cerr << "Function load not implemented yet!" << std::endl;
    return Nebulite::Constants::ErrorTable::FUNCTIONALL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::General::stateSave(int argc, char* argv[]){
    // <stateName>
    // Change std::string Nebulite::stateName to name
    // Check if dir ./States/stateName exists

    // If any env is deloaded, save in stateName

    // Every load of any file must be linked to state! If file exists in state load from there
    // if not, load from usual path

    std::cerr << "Function save not implemented yet!" << std::endl;
    return Nebulite::Constants::ErrorTable::FUNCTIONALL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}