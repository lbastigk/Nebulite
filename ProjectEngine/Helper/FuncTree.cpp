#include "FuncTree.h"

FuncTree::FuncTree(){
    // Attach the help function to read out the description of all attached functions
    // using lambda
    (void) attachFunction([this](int argc, char* argv[]) { return this->help(argc, argv); },"help","TODO... If you read this, you have called 'help' of 'help'");
}

// Attach a function to the menu
void FuncTree::attachFunction(FunctionPtr func, const std::string& name, const std::string& helpDescription) {
    functions[name] = std::make_pair(func, helpDescription);
}

// Parse function reduces argc and argv as it calls functions
int FuncTree::parse(int argc, char* argv[]) {
    if (argc <= 1) {  // No arguments left to process
        return 0;  // End of execution
    }

    std::string funcName = argv[1];  // The first argument is the function name
    char** newArgv = argv + 1;  // Skip the first argument (already processed)
    int newArgc = argc - 1;  // The new argc is reduced by 1 (function name and its args)

    // Execute the function corresponding to funcName with the remaining arguments
    std::cout << "Function name is: " << funcName << std::endl;
    int result = executeFunction(funcName, newArgc, newArgv);

    // Return the result from the function execution
    return result;
}



// Execute the function based on its name, passing the remaining argc and argv
int FuncTree::executeFunction(const std::string& name, int argc, char* argv[]) {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second.first(argc, argv);  // Call the function
    } else {
        std::cout << "Function '" << name << "' not found.\n";
        return -1;  // Return error if function not found
    }
}


int FuncTree::help(int argc, char* argv[]) {
    for (int i=1; i<argc;i++){
        auto it = functions.find(std::string(argv[i]));
        if (it != functions.end()) {
            std::cout << it->second.second << std::endl;  // Print function description
        } else {
            std::cout << "Function '" << argv[i] << "' not found.\n";
        }
    }
}
