// This header defines the FuncTree class, which is responsible for parsing command-line
// arguments and executing the corresponding functions in the Nebulite project. The main goal
// of this class is to manage hierarchical commands and arguments for modular and flexible execution.

// TODO: Going away from classic C-style argc/argv to a more modern approach:
// - std::vector<std::string> callTrace // shows the call trace of the function, e.g.: "Nebulite", "eval", "echo"
// - std::vector<std::string> args      // shows the arguments of the function, e.g.: "echo", "Hello World!"
#pragma once

#include <iomanip>  // For std::setw
#include <iostream>
#include <vector>
#include "absl/container/flat_hash_map.h"
#include <functional>
#include <cstring>

// Custom includes
#include "StringHandler.h"

template<typename RETURN_TYPE>
class FuncTree{
public:
    // Two different types are given to the constructor:
    // - What to return if okay (e.g. Calling help should always return ok)
    // - What to return if no function was found
    FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError);

    // functions get std::string as args and return a given type
    using FunctionPtr = std::function<RETURN_TYPE(int argc, char* argv[])>;

    // Parse is the entry to execution after all functions and arguments are attached
    // takes argc and argv, function name and executes
    // - The first argument is the function name itself
    // - call executeFunction provided by second argv
    //   pass all argv afterwards. Making sure argc is adjusted accordingly
    RETURN_TYPE parse(int argc, char* argv[]);

    // Same as parse, but using an std::string
    RETURN_TYPE parseStr(const std::string& cmd);

    // Binding helper
    // TODO: Instead of calling attachFunction, directly modify map
    // Binding helper
    // e.g.: bindFunction(&ComplexData::sqlCall, "sqlCall", "Handles SQL calls");
    template<typename ClassType>
    void bindFunction(ClassType* obj,
            RETURN_TYPE (ClassType::*method)(int, char**),
            const std::string& name,
            const std::string& help) {
        // Create std::function that binds the member function to the object
        functions[name] = std::make_pair(
            [obj, method](int argc, char** argv) {
                return (obj->*method)(argc, argv);
            },
            help
        );
    }
    // Required overload to attach the help-function
    template<typename FuncType>
    void bindFunction(FuncType&& func,
            const std::string& name,
            const std::string& help) {
        functions[name] = std::make_pair(
            std::function<RETURN_TYPE(int, char**)>(std::forward<FuncType>(func)),
            help
        );
    }

    // Attach a variable to the menu (by name)
    void bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription);

    // Check if a function with the given name or from a full command exists
    bool hasFunction(const std::string& nameOrCommand);

private:
    // Execute a given function
    RETURN_TYPE executeFunction(const std::string& name, int argc, char* argv[]);

    // Help-command, called with argv[1] = "help"
    RETURN_TYPE help(int argc, char* argv[]);

    // Status "Function not found"
    RETURN_TYPE _functionNotFoundError;

    // Status "ok"
    RETURN_TYPE _standard;

    // Map for Functions: name -> (functionPtr, info)
    absl::flat_hash_map<std::string, std::pair<FunctionPtr,  std::string>> functions;

    // Map for variables: name -> (pointer, info)
    absl::flat_hash_map<std::string, std::pair<std::string*, std::string>> variables;

    std::string TreeName; 
};



template<typename RETURN_TYPE>
FuncTree<RETURN_TYPE>::FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError){
    // Attach the help function to read out the description of all attached functions
    // using lambda
    TreeName = treeName;
    bindFunction([this](int argc, char* argv[]) { 
        return this->help(argc, argv); 
    }, "help", "Show available commands and their descriptions");

    _standard = standard;
    _functionNotFoundError = functionNotFoundError;
}

template<typename RETURN_TYPE>
void FuncTree<RETURN_TYPE>::bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription) {
    variables[name] = std::make_pair(varPtr, helpDescription);
}

template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::parse(int argc, char* argv[]) {
    // First argument is binary name or last function name
    // remove it from the argument list
    argv++;
    argc--;

    // No arguments left to process
    if (argc < 1) {  
        return _standard;  // End of execution
    }

    // Process arguments directly after binary/function name (like --count or -c)
    bool parseVars = true;
    while(parseVars && argc > 0){
        std::string arg = argv[0];
        if(arg.starts_with("--")){
            // -key=value or --key
            std::string key, val;
            size_t eqPos = arg.find('=');
            if (eqPos != std::string::npos) {
                key = arg.substr(2, eqPos - 2);
                val = arg.substr(eqPos + 1);
            } else {
                key = arg.substr(2);
                val = "true";
            }

            // Set variable if attached
            auto varIt = variables.find(key);
            if (varIt != variables.end() && varIt->second.first) {
                *(varIt->second.first) = val;
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

    // Check if there are still arguments left
    if(argc>0){
        // The first argument left is the new function name
        std::string funcName = argv[0];  

        // Execute the function corresponding to funcName with the remaining arguments
        return executeFunction(funcName, argc, argv);
    }
    return _standard;
}

template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::parseStr(const std::string& cmd) {
    std::vector<std::string> tokens = Nebulite::StringHandler::split(cmd, ' ');

    // Convert to argc/argv
    int argc = static_cast<int>(tokens.size());
    std::vector<char*> argv;
    argv.reserve(argc + 1);

    for (auto& str : tokens) {
        argv.push_back(const_cast<char*>(str.c_str()));
    }
    argv.push_back(nullptr); // Null-terminate

    // Call existing parse
    return parse(argc, argv.data());
}


// Execute the function based on its name, passing the remaining argc and argv
template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::executeFunction(const std::string& name, int argc, char* argv[]) {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second.first(argc, argv);  // Call the function
    } else {
        std::cerr << "Function '" << name << "' not found.\n";
        return _functionNotFoundError;  // Return error if function not found
    }
}

template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::help(int argc, char* argv[]) {
    std::cout << "\n\tHelp for " << TreeName << "\n\n";

    // If no arguments are provided, list all functions and variables
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

        // Step 3: Print functions
        for (const auto& entry : sortedFunctions) {
            std::cout << "  "
                    << std::setw(25) << std::left << entry.first
                    << " - "
                    << entry.second.second
                    << std::endl;
        }

        // List all attached variables
        if (!variables.empty()) {
            std::cout << "\nAvailable variables:\n";
            // Sort variables by name
            std::vector<std::pair<std::string, std::pair<std::string*, std::string>>> sortedVariables(
                variables.begin(), variables.end());
            std::sort(sortedVariables.begin(), sortedVariables.end(),
                [](const auto& a, const auto& b) {
                    return a.first < b.first;
                });
            for (const auto& entry : sortedVariables) {
                std::cout << "  "
                        << std::setw(25) << std::left << entry.first
                        << " - "
                        << entry.second.second
                        << std::endl;
            }
        }

        return _standard;
    }

    // Otherwise, display help for the provided functions
    for (int i = 1; i < argc; i++) {
        if (argv[i] == nullptr) {
            std::cerr << "Error: Null argument found.\n";
            continue;  // Skip null arguments
        }

        auto it = functions.find(std::string(argv[i]));
        if (it != functions.end()) {
            std::cout << std::string(argv[i]) << std::endl;
            std::cout << it->second.second << std::endl;  // Print function description
        } else {
            std::cerr << "Function '" << argv[i] << "' not found.\n";
        }
    }

    return _standard;
}

template<typename RETURN_TYPE>
bool FuncTree<RETURN_TYPE>::hasFunction(const std::string& nameOrCommand) {
    // Make sure only the command name is used
    std::vector<std::string> tokens = Nebulite::StringHandler::split(nameOrCommand, ' ');
    if (tokens.empty()) {
        return false;  // No command provided
    }
    if(tokens.size() == 1){
        // Is a single function name. 
        // e.g.: "set"
        return functions.find(tokens[0]) != functions.end();
    }
    // Is a full command
    // e.g.: <whereCommandComesFrom> set key value
    return functions.find(tokens[1]) != functions.end();

}
