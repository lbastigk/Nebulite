/*
This header defines the FuncTree class, which is responsible for parsing command-line
arguments and executing the corresponding functions in the Nebulite project. The main goal
of this class is to manage hierarchical commands and arguments for modular and flexible execution.

Example usage:

--------------------------------
#include "FuncTree.h"
int main(int argc, char* argv[]) {
    FuncTree<std::string> funcTree("Nebulite", "ok", "Function not found");
    funcTree.bindFunction([](int argc, char* argv[]) {
        // Function implementation
        return "Function executed";
    }, "myFunction", "This function does something");

    std::string result = funcTree.parse(argc,argv);
    std::cout << result << "\n";
}
---------------------------------
This will parse the command-line arguments and execute the "myFunction" if it is called:
./main myFunction        -> cout: "Function executed"
./main                   -> cout: "ok" (SUCCESS: no function called)
./main help              -> cout: shows available commands and their descriptions
./main someOtherFunction -> cout: "Function not found", 
                            cerr: "Function 'someOtherFunction' not found." 
*/

// TODO: Automatic subtree generation
// If a function is linked with multiple args: bindFunction(&fun,"myCategory myFunction","Description");
// Create a subtree for "myCategory" and add the function to it
// As well as binding an entry function for the subtree
// This way, the user can call "myCategory myFunction" and it will execute the function
// If the user calls "myCategory", it will show the help for the subtree
// This simplifies the command structure a lot and allows for more modular command trees
// Also, allow for users to add help information to the subtree itself, e.g.:
// bindSubtree("myCategory", "This is a category of functions");
// Perhaps even throwing an error if the user tries to bind a function to a subtree that does not exist

// TODO: Going away from classic C-style argc/argv to a more modern approach:
// - std::vector<std::string> callTrace // shows the call trace of the function, e.g.: "Nebulite", "eval", "echo"
// - std::vector<std::string> args      // shows the arguments of the function, e.g.: "echo", "Hello World!"
// - std::vector<std::string> variables // shows the variables set by the user, e.g.: {"--verbose": "true"}

#pragma once

// Basic includes
#include <iomanip>
#include <iostream>
#include <vector>
#include "absl/container/flat_hash_map.h"
#include <functional>
#include <cstring>
#include <algorithm>

// Custom includes
#include "StringHandler.h"

template<typename RETURN_TYPE>
class FuncTree{
public:
    // Two different types are given to the constructor:
    // - What to return if okay (e.g. Calling help should always return ok)
    // - What to return if no function was found
    FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError, FuncTree<RETURN_TYPE>* subtree = nullptr);

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
    // e.g.: bindFunction(&ComplexData::sqlCall, "sqlCall", "Handles SQL calls");
    template<typename ClassType>
    void bindFunction(ClassType* obj,
            RETURN_TYPE (ClassType::*method)(int, char**),
            const std::string& name,
            const std::string& help) {

        
        // Making sure the function name is not registered in the subtree
        // Note: subtree is checked only after constructor completes, so this should be safe
        // The only overwrite that is allowed is the help function
        if (subtree && name != "help" && subtree->hasFunction(name)) {
            // Throw a proper error
            // exit the entire program
            std::cerr << "---------------------------------------------------------------\n";
            std::cerr << "Nebulite FuncTree initialization failed!\n";
            std::cerr << "Error: Function '" << name << "' already exists in a linked subtree.\n";
            std::cerr << "Function overwrite is heavily discouraged and thus not allowed.\n";
            std::cerr << "Please choose a different name or remove the existing function.\n";
            std::cerr << "This Tree: " << TreeName << "\n";
            std::cerr << "Subtree:   " << subtree->TreeName << "\n";
            std::cerr << "Function:  " << name << "\n";
            std::exit(EXIT_FAILURE);  // Exit with failure status

            // Just for completion, this will never be reached
            return;
        }

        // Same for the own tree
        if (hasFunction(name)) {
            // Throw a proper error
            // exit the entire program
            std::cerr << "---------------------------------------------------------------\n";
            std::cerr << "Nebulite FuncTree initialization failed!\n";
            std::cerr << "Error: Function '" << name << "' already exists in this tree.\n";
            std::cerr << "Function overwrite is heavily discouraged and thus not allowed.\n";
            std::cerr << "Please choose a different name or remove the existing function.\n";
            std::cerr << "Tree: " << TreeName << "\n";
            std::exit(EXIT_FAILURE);  // Exit with failure status
        }

        // Create FunctionInfo directly
        functions[name] = FunctionInfo{
            [obj, method](int argc, char** argv) {
                return (obj->*method)(argc, argv);
            },
            help
        };
    }
    // Required overload to attach the help-function
    template<typename FuncType>
    void bindFunction(FuncType&& func,
            const std::string& name,
            const std::string& help) {
        // Create FunctionInfo directly
        functions[name] = FunctionInfo{
            std::function<RETURN_TYPE(int, char**)>(std::forward<FuncType>(func)),
            help
        };
    }

    // Bind a variable to the menu (by name)
    // once bound, it can be set via command line arguments: --varName=value (Must be before the function name!)
    void bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription);

    // Check if a function with the given name or from a full command exists
    bool hasFunction(const std::string& nameOrCommand);

private:
    // Function - Description pair
    struct FunctionInfo {
        FunctionPtr function;
        std::string description;
    };

    // Variable - Description pair
    struct VariableInfo {
        std::string* pointer;
        std::string description;
    };

    // Execute the function based on its name, passing the remaining argc and argv
    RETURN_TYPE executeFunction(const std::string& name, int argc, char* argv[]);

    // Help-command, called with argv[1] = "help"
    RETURN_TYPE help(int argc, char* argv[]);

    // Quote-aware argument parsing helper
    std::vector<std::string> parseQuotedArguments(const std::string& cmd);

    // Status "Function not found"
    RETURN_TYPE _functionNotFoundError;

    // Status "ok"
    RETURN_TYPE _standard;

    // Map for Functions: name -> (functionPtr, info)
    absl::flat_hash_map<std::string, FunctionInfo> functions;

    // Map for variables: name -> (pointer, info)
    absl::flat_hash_map<std::string, VariableInfo> variables;

    // Name of the tree, used for help and output
    std::string TreeName; 

    // Subtree linked to this tree
    FuncTree<RETURN_TYPE>* subtree;
};


template<typename RETURN_TYPE>
FuncTree<RETURN_TYPE>::FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError, FuncTree<RETURN_TYPE>* subtree)
    : subtree(nullptr)  // Initialize to nullptr first to avoid issues during construction
{
    // Attach the help function to read out the description of all attached functions
    // using lambda
    TreeName = treeName;
    bindFunction([this](int argc, char* argv[]) { 
        return this->help(argc, argv); 
    }, "help", "Show available commands and their descriptions");

    _standard = standard;
    _functionNotFoundError = functionNotFoundError;
    
    // Link the subTree AFTER basic initialization is complete
    if(subtree){
        this->subtree = subtree;
    }
}

template<typename RETURN_TYPE>
void FuncTree<RETURN_TYPE>::bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription) {
    variables[name] = VariableInfo{varPtr, helpDescription};
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
        if(arg.length() >= 2 && arg.substr(0, 2) == "--" /*same as arg.starts_with("--"), but C++17 compatible*/){
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
            if (auto varIt = variables.find(key); varIt != variables.end()) {
                const auto& varInfo = varIt->second;  // Now it's VariableInfo, not a pair
                if (varInfo.pointer) {
                    *varInfo.pointer = val;
                }
            } else {
                std::cerr << "Warning: Unknown variable '--" << key << "'\n";
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
    // Prerequisite if a subtree is linked
    if(subtree && !hasFunction(cmd)){
        // Assume the subtree can handle the command
        return subtree->parseStr(cmd);
    }

    // Quote-aware tokenization
    std::vector<std::string> tokens = parseQuotedArguments(cmd);

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

template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::executeFunction(const std::string& name, int argc, char* argv[]) {
    auto functionPosition = functions.find(name);
    if (functionPosition != functions.end()) {
        auto& [functionPtr, description] = functionPosition->second;
        return functionPtr(argc, argv);  // Call the function
    } else {
        std::cerr << "Function '" << name << "' not found.\n";
        return _functionNotFoundError;  // Return error if function not found
    }
}

// TODO: While listing all functions this way works, 
// its much better to first combine all from parent and subtree
// e.g.: 
// std::vector<std::string> getAllFunctionDescriptions()
// std::vector<std::string> getAllVariableDescriptions()
// Combine them and sort them by name
template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::help(int argc, char* argv[]) {
    std::cout << "\n\tHelp for " << TreeName << "\n\n";

    // If no arguments are provided, list all functions and variables
    if (argc <= 1) {
        std::cout << "Available functions:\n";

        // Sort functions by name, excluding "help"
        std::vector<std::pair<std::string, FunctionInfo>> sortedFunctions;
        for (const auto& entry : functions) {
            if (entry.first != "help") {
                sortedFunctions.push_back(entry);
            }
        }

        // Sort by function name
        std::sort(sortedFunctions.begin(), sortedFunctions.end(),
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            });

        // Print functions
        for (const auto& [name, funcInfo] : sortedFunctions) {
            std::cout << "  "
                    << std::setw(25) << std::left << name
                    << " - "
                    << funcInfo.description
                    << std::endl;
        }

        // List all attached variables
        if (!variables.empty()) {
            std::cout << "\nAvailable variables:\n";
            std::vector<std::pair<std::string, VariableInfo>> sortedVariables(
                variables.begin(), variables.end());
            
            std::sort(sortedVariables.begin(), sortedVariables.end(),
                [](const auto& a, const auto& b) {
                    return a.first < b.first;
                });
            
            for (const auto& [name, varInfo] : sortedVariables) {
                std::cout << "  "
                          << std::setw(25) << std::left << name
                          << " - "
                          << varInfo.description
                          << std::endl;
            }
        } else {
            std::cout << "\nNo variables attached.\n";
        }

        // If a subtree is linked, show its help
        if (subtree) {
            subtree->help(argc, argv);
        }

        return _standard;
    }

    // Otherwise, display help for the provided functions
    for (int i = 1; i < argc; i++) {
        if (argv[i] == nullptr) {
            std::cerr << "Error: Null argument found.\n";
            continue;
        }

        auto functionPosition = functions.find(std::string(argv[i]));
        if (functionPosition != functions.end()) {
            std::cout << std::string(argv[i]) << std::endl;
            std::cout << functionPosition->second.description << std::endl;
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

template<typename RETURN_TYPE>
std::vector<std::string> FuncTree<RETURN_TYPE>::parseQuotedArguments(const std::string& cmd) {
    std::vector<std::string> tokens = Nebulite::StringHandler::split(cmd, ' ');
    std::vector<std::string> result;
    
    bool inQuoteV1 = false;  // Double quotes
    bool inQuoteV2 = false;  // Single quotes
    
    for (const auto& token : tokens) {
        // Skip empty tokens (from multiple spaces or trailing spaces)
        if (token.empty()) {
            continue;
        }
        
        if (!inQuoteV1 && !inQuoteV2) {
            // Not in quotes - check if this token starts a quote
            if (token[0] == '"') {
                inQuoteV1 = true;
                // Remove opening quote and add to result
                std::string cleanToken = token.substr(1);
                if (!cleanToken.empty() && cleanToken.back() == '"') {
                    // Quote opens and closes in same token
                    inQuoteV1 = false;
                    cleanToken.pop_back(); // Remove closing quote
                }
                result.push_back(cleanToken);
            } else if (token[0] == '\'') {
                inQuoteV2 = true;
                // Remove opening quote and add to result
                std::string cleanToken = token.substr(1);
                if (!cleanToken.empty() && cleanToken.back() == '\'') {
                    // Quote opens and closes in same token
                    inQuoteV2 = false;
                    cleanToken.pop_back(); // Remove closing quote
                }
                result.push_back(cleanToken);
            } else {
                // Regular token
                result.push_back(token);
            }
        } else {
            // Currently in quotes - append to last token
            if (inQuoteV1 && token.back() == '"') {
                // End of double quote
                inQuoteV1 = false;
                std::string cleanToken = token.substr(0, token.length() - 1);
                if (!result.empty()) {
                    result.back() += " " + cleanToken;
                }
            } else if (inQuoteV2 && token.back() == '\'') {
                // End of single quote
                inQuoteV2 = false;
                std::string cleanToken = token.substr(0, token.length() - 1);
                if (!result.empty()) {
                    result.back() += " " + cleanToken;
                }
            } else {
                // Still in quotes, append to last token
                if (!result.empty()) {
                    result.back() += " " + token;
                }
            }
        }
    }
    
    // Warning for unclosed quotes
    if (inQuoteV1 || inQuoteV2) {
        std::cerr << "Warning: Unclosed quote in command: " << cmd << std::endl;
    }
    
    return result;
}
