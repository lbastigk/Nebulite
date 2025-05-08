#include "FuncTree.h"
#include <iomanip>  // For std::setw

FuncTree::FuncTree(std::string treeName){
    // Attach the help function to read out the description of all attached functions
    // using lambda
    TreeName = treeName;
    (void) attachFunction([this](int argc, char* argv[]) { return this->help(argc, argv); },"help","TODO... If you read this, you have called 'help' of 'help'");
}

// Attach a function to the menu
void FuncTree::attachFunction(FunctionPtr func, const std::string& name, const std::string& helpDescription) {
    functions[name] = std::make_pair(func, helpDescription);
}

int FuncTree::parse(int argc, char* argv[]) {
    // No arguments left to process
    if (argc < 1) {  
        return 0;  // End of execution
    }

    // Process arguments (like --count or -c)
    // only process those that still
    if(argc > 0){
        bool parseVars = true;
        while(parseVars && argc > 0){
            std::string arg = argv[0];
            if(arg.starts_with('-')){
                std::string key, val;
                if(arg.starts_with('--')){
                    // Long form: --key=value or --key
                    size_t eqPos = arg.find('=');
                    if (eqPos != std::string::npos) {
                        key = arg.substr(2, eqPos - 2);
                        val = arg.substr(eqPos + 1);
                    } else {
                        key = arg.substr(2);
                        val = "true";
                    }
                }
                else{
                    size_t eqPos = arg.find('=');
                    if (eqPos != std::string::npos) {
                        key = arg.substr(1, eqPos - 1);
                        val = arg.substr(eqPos + 1);
                    } else {
                        key = arg.substr(1);
                        val = "true";
                    }
                }
                // Check if arg is in std::map
                auto it = argumentPtrs.find(key);
                if(it != argumentPtrs.end()){
                    *argumentPtrs[key].first = val;
                }

                // Remove from argument list
                argv++;       // Skip the first argument (function name)
                argc--;       // Reduce the argument count (function name is processed)
            }
            else{
                // no more vars to parse
                parseVars = false;
            }
            
        }
    }

    if(argc>0){
        std::string funcName = argv[0];  // The first argument left is the function name

        // remove function name
        argv++;
        argc--;

        // Execute the function corresponding to funcName with the remaining arguments
        return executeFunction(funcName, argc, argv);
    }
    return 0;
}


void FuncTree::attachArgument(std::string* stringPtr, const std::string& argLong, const std::string& argShort, const std::string& helpDescription) {
    // Ensure that at least one of the argument formats (long or short) is specified
    if (argLong.empty() && argShort.empty()) {
        std::cerr << "Error: Must specify at least one argument (long or short)." << std::endl;
        return;
    }

    // Attach the argument (long form and/or short form) to the provided string pointer
    if (!argLong.empty()) {
        argumentPtrs[argLong] = std::make_pair(stringPtr, helpDescription);
    }
    if (!argShort.empty()) {
        argumentPtrs[argShort] = std::make_pair(stringPtr, helpDescription);
    }

    // Now each argument has both the string pointer and its description attached
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
    std::cout << "\n\tHelp for " << TreeName << "\n\n";

    // If no arguments are provided, list all functions
    if (argc <= 1) {
        std::cout << "Available functions:\n";
        for (const auto& entry : functions) {
            // Help is not presented
            if (entry.first != "help"){
                std::cout << "  " 
                      << std::setw(20) << std::left << entry.first  // Fixed width of 20, left-aligned
                      << " - " 
                      << entry.second.second 
                      << std::endl;
            }
        }
        return 0;  // No error
    }

    // Otherwise, display help for the provided functions
    for (int i = 1; i < argc; i++) {
        if (argv[i] == nullptr) {
            std::cout << "Error: Null argument found.\n";
            continue;  // Skip null arguments
        }

        auto it = functions.find(std::string(argv[i]));
        if (it != functions.end()) {
            std::cout << std::string(argv[i]) << std::endl;
            std::cout << it->second.second << std::endl;  // Print function description
        } else {
            std::cout << "Function '" << argv[i] << "' not found.\n";
        }
    }

    return 0;
}
