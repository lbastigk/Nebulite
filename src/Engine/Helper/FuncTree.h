// This header defines the FuncTree class, which is responsible for parsing command-line
// arguments and executing the corresponding functions in the Nebulite project. The main goal
// of this class is to manage hierarchical commands and arguments for modular and flexible execution.


#pragma once

#include <iostream>
#include <vector>
#include "absl/container/flat_hash_map.h"
#include <functional>
#include <cstring>

template<typename T>
class FuncTree{
public:
    FuncTree(std::string treeName, T standard, T functionNotFoundError);

    // functions get std::string as args and return an int
    
    using FunctionPtr = std::function<T(int argc, char* argv[])>;

    // Parse is the entry to execution after all functions and arguments are attached
    // takes argc and argv, function name and executes
    // - The first argument is the function name itself
    // - call executeFunction provided by second argv
    //   pass all argv afterwards. Making sure argc is adjusted accordingly
    T parse(int argc, char* argv[]);

    // Attach a function to the menu
    void attachFunction(FunctionPtr func, const std::string& name, const std::string& helpDescription);

    // Execute the function
    T executeFunction(const std::string& name, int argc, char* argv[]);

private:
    T help(int argc, char* argv[]);

    T _functionNotFoundError;
    T _standard;
    
    absl::flat_hash_map<std::string, std::pair<FunctionPtr,  std::string>> functions;
    std::string TreeName; 
};

#include <iomanip>  // For std::setw

template<typename T>
FuncTree<T>::FuncTree(std::string treeName, T standard, T functionNotFoundError){
    // Attach the help function to read out the description of all attached functions
    // using lambda
    TreeName = treeName;
    (void) attachFunction([this](int argc, char* argv[]) { return this->help(argc, argv); },"help","");

    _standard = standard;
    _functionNotFoundError = functionNotFoundError;
}

// Attach a function to the menu
template<typename T>
void FuncTree<T>::attachFunction(FunctionPtr func, const std::string& name, const std::string& helpDescription) {
    functions[name] = std::make_pair(func, helpDescription);
}

template<typename T>
T FuncTree<T>::parse(int argc, char* argv[]) {
    // No arguments left to process
    if (argc < 1) {  
        return _standard;  // End of execution
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

    if(argc>1){
        // e.g. ./bin/Nebulite eval echo '$(1+1)'
        // turns into:
        // {"Nebulite", "eval", "echo", "$(1+1)"}
        // Parsing Part 1, calling eval with {"eval", "echo", "$i(1+1)"}
        // Parsing Part 2, calling echo with {"echo", "2"}
        std::string funcName = argv[1];  // The second argument left is the function name

        // remove previous function name
        argv++;
        argc--;

        // Execute the function corresponding to funcName with the remaining arguments
        return executeFunction(funcName, argc, argv);
    }
    return _standard;
}



// Execute the function based on its name, passing the remaining argc and argv
template<typename T>
T FuncTree<T>::executeFunction(const std::string& name, int argc, char* argv[]) {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second.first(argc, argv);  // Call the function
    } else {
        std::cout << "Function '" << name << "' not found.\n";
        return _functionNotFoundError;  // Return error if function not found
    }
}

template<typename T>
T FuncTree<T>::help(int argc, char* argv[]) {
    std::cout << "\n\tHelp for " << TreeName << "\n\n";

    // If no arguments are provided, list all functions
    if (argc <= 1) {
        std::cout << "Available functions:\n";

        // Step 1: Extract entries into a vector (excluding "help")
        std::vector<std::pair<std::string, std::pair<FunctionPtr, std::string>>> sortedFunctions;
        for (const auto& entry : functions) {
            if (entry.first != "help") {
                sortedFunctions.push_back(entry);
            }
        }

        // Step 2: Sort by function name (entry.first)
        std::sort(sortedFunctions.begin(), sortedFunctions.end(),
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            });

        // Step 3: Print
        for (const auto& entry : sortedFunctions) {
            std::cout << "  "
                    << std::setw(25) << std::left << entry.first
                    << " - "
                    << entry.second.second
                    << std::endl;
        }

        return _standard;
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

    return _standard;
}

