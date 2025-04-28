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
    if (argc <= 1) {  // No arguments left to process
        return 0;  // End of execution
    }

    std::string funcName = argv[1];  // The first argument is the function name
    char** newArgv = argv + 1;       // Skip the first argument (function name)
    int newArgc = argc - 1;          // Reduce the argument count (function name is processed)

    // Process arguments (like --count or -c)
    for (int i = 1; i < newArgc; ++i) {
        std::string arg = newArgv[i];

        // Check if the argument matches any long or short form argument
        auto it = argumentPtrs.find(arg);  // Look for long or short argument format in argumentPtrs
        if (it != argumentPtrs.end()) {
            // Argument found, set the value in the corresponding string pointer
            std::string* valuePtr = it->second.first;  // Pointer to the value for this argument
            std::string helpDescription = it->second.second;  // Description (optional)
            
            // Check if the argument has a value (e.g., --count=100 or --level=2)
            if (arg.find('=') != std::string::npos) {
                // Argument has a value (e.g., --count=100), parse it
                size_t equalPos = arg.find('=');
                *valuePtr = arg.substr(equalPos + 1);  // Store the value (e.g., "100" for --count=100)
                // Remove the argument from the list (as it's fully processed)
                newArgc--; 
                for (int j = i; j < newArgc; ++j) {
                    newArgv[j] = newArgv[j + 1];  // Shift arguments left
                }
                i--;  // Adjust index since we removed an argument
            } else {
                // If no value, we can handle it differently (e.g., boolean flags)
                *valuePtr = "true";  // Just set a placeholder for boolean-style flags (if needed)
            }
        }
    }

    // Execute the function corresponding to funcName with the remaining arguments
    int result = executeFunction(funcName, newArgc, newArgv);

    // Return the result from the function execution
    return result;
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
