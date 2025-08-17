/**
 * @file FuncTree.h
 * 
 * This file defines the FuncTree class, which is responsible for managing and executing functions
 * through a command tree structure. 
 * The main goal of this class is to manage hierarchical commands and arguments for modular and flexible execution.
 * 
 * Example usage:
 * ```cpp
 * #include "FuncTree.h"
 * int main(int argc, char* argv[]) {
 *     FuncTree<std::string> funcTree("Nebulite", "ok", "Function not found");
 *     funcTree.bindFunction([](int argc, char* argv[]) {
 *         // Function implementation
 *         return "Function executed";
 *     }, "myFunction", "This function does something");
 *
 *     std::string result = funcTree.parse(argc,argv);
 *     std::cout << result << "\n";
 * }
 * ```
 * 
 * This will parse the command-line arguments and execute the "myFunction" if it is called:
 * ```cpp
 * ./main myFunction        //-> cout: "Function executed"
 * ./main                   //-> cout: "ok" (SUCCESS: no function called)
 * ./main help              //-> cout: shows available commands and their descriptions
 * ./main someOtherFunction //-> cout: "Function not found",
 *                          //   cerr: "Function 'someOtherFunction' not found."
 * ```
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

// TODO: Allow for longer descriptions via an extra argument: descLong

#pragma once

// Basic includes
#include <iostream>
#include <vector>
#include "absl/container/flat_hash_map.h"

// Custom includes
#include "StringHandler.h"  // Using StringHandler for easy argument splitting

/**
 * @brief Function tree class for managing and executing functions through linguistic commands.
 *
 * The FuncTree class allows for the binding of functions and variables to a command tree structure,
 * enabling modular and flexible execution of commands based on user input.
 * Functions are identified by their names and can have multiple arguments.
 * Variables can be bound to the command tree and accessed within functions, 
 * provided the functions themselves have access to the space of the variables.
 * 
 * Example:
 * ```cpp
 * std::string myArgument = "false";
 * int foo(int argc, char** argv){std::cout << "foo: " << myArgument << std::endl}
 * int bar(int argc, char** argv){std::cout << "bar: " << myArgument << std::endl}
 *
 * FuncTree<int> funcTree;
 * funcTree.bindVariable(&myArgument, "myArgument", "This is my argument");
 * funcTree.bindFunction(&foo, "foo", "This function does foo");
 * funcTree.bindFunction(&bar, "bar", "This function does bar");
 *
 * std::string command = "FromExample --myArgument foo";
 * funcTree.parseStr(command);  // output: "foo: true"
 * ```
 */
template<typename RETURN_TYPE>
class FuncTree{
public:
    /**
     * @brief Constructor for the FuncTree class.
     * @param treeName Name of the tree
     * @param standard Value to return if everything is okay
     * @param functionNotFoundError Value to return if the parsed function was not found
     * @param subtree Pointer to a subtree (optional)
     */
    FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError, FuncTree<RETURN_TYPE>* subtree = nullptr);

    /**
     * @brief Parses the command line arguments and executes the corresponding function.
     * 
     * The first argument should be used to identify where the execution comes from.
     * All subsequent arguments starting with -- are treated as variable assignments.
     * The first argument after the variable assignments is the function to execute.
     * 
     * Example: 
     * ```cpp 
     * "./bin/Nebulite --myArgument foo"
     * ```
     */
    RETURN_TYPE parse(int argc, char* argv[]);

    /**
     * @brief Parses the command line arguments and executes the corresponding function.
     * 
     * All whitespaces outside of quotes are seen as argument separators:
     * 
     * Example: 
     * ```cpp
     * // Input string:
     * "./bin/Nebulite if '$({myCondition} and {myOtherCondition})' echo here!"
     * // Result:
     * argv[0] = "./bin/Nebulite"
     * argv[1] = "if"
     * argv[2] = "$({myCondition} and {myOtherCondition})"  // due to the quotes, this is treated as a single argument
     * argv[3] = "echo"
     * argv[4] = "here!"
     * ```
     * 
     * The first argument should be used to identify where the execution comes from.
     * All subsequent arguments starting with -- are treated as variable assignments.
     * The first argument after the variable assignments is the function to execute.
     * 
     * Examples: 
     * ```cpp 
     * "./bin/Nebulite --myArgument foo"
     * ```
     */
    RETURN_TYPE parseStr(const std::string& cmd);

    /**
     * @brief Binds a function to the command tree.
     * 
     * Make sure the function has the signature:
     * ```cpp
     * RETURN_TYPE functionName(int argc, char** argv);
     * ```
     */
    template<typename ClassType>
    void bindFunction(ClassType* obj, RETURN_TYPE (ClassType::*method)(int, char**), const std::string& name, const std::string& help);

    /**
     * @brief Binds a variable to the command tree.
     * 
     * Make sure the variable is of type std::string*.
     * 
     * Once bound, it can be set via command line arguments: --varName=value (Must be before the function name!)
     * 
     * A simple argument of '--varName' will set the value to "true"
     */
    void bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription);

    // Check if a function with the given name or from a full command exists
    /**
     * @brief Checks if a function with the given name or from a full command exists.
     * 
     * Examples:
     * ```cpp
     * // Both check if the function "myFunction" exists
     * funcTree.hasFunction("myFunction");
     * funcTree.hasFunction("./bin/Nebulite --myVariable myFunction argumentOfMyFunction");
     * ```
     */
    bool hasFunction(const std::string& nameOrCommand);

private:
    //---------------------------------------
    // Variables

    using FunctionPtr = std::function<RETURN_TYPE(int argc, char* argv[])>;

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

    //---------------------------------------
    // Functions

    // Execute the function based on its name, passing the remaining argc and argv
    /**
     * @brief Executes the function with the given name.
     * 
     * This function looks up the function by name and calls it with the provided arguments.
     * 
     * @param name The name of the function to execute.
     * @param argc The argument count.
     * @param argv The argument vector.
     * @return The return value of the function.
     */
    RETURN_TYPE executeFunction(const std::string& name, int argc, char* argv[]);

    /**
     * @brief Displays help information to all bound functions. Automatically bound to any FuncTree on construction.
     */
    RETURN_TYPE help(int argc, char* argv[]);

    /**
     * @brief Parses a command string into individual arguments, taking quotes into account.
     */
    std::vector<std::string> parseQuotedArguments(const std::string& cmd);

    /**
     * @brief Retrieves a list of all functions and their descriptions.
     * 
     * @return A vector of pairs containing function names and their descriptions.
     */
    std::vector<std::pair<std::string, std::string>> getAllFunctions();

    /**
     * @brief Retrieves a list of all variables and their descriptions.
     * 
     * @return A vector of pairs containing variable names and their descriptions.
     */
    std::vector<std::pair<std::string, std::string>> getAllVariables();
};

//---------------------------------
// Binding helper

// bindfunction todo...
template<typename RETURN_TYPE>
template<typename ClassType>
void FuncTree<RETURN_TYPE>::bindFunction(ClassType* obj, RETURN_TYPE (ClassType::*method)(int, char**), const std::string& name, const std::string& help) {
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

template<typename RETURN_TYPE>
void FuncTree<RETURN_TYPE>::bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription) {
    variables[name] = VariableInfo{varPtr, helpDescription};
}

//---------------------------------
// Getter

template<typename RETURN_TYPE>
std::vector<std::pair<std::string, std::string>> FuncTree<RETURN_TYPE>::getAllFunctions() {
    std::vector<std::pair<std::string, std::string>> allFunctions;
    for (const auto& [name, info] : functions) {
        allFunctions.emplace_back(name, info.description);
    }

    // Get from subtree
    if(subtree) {
        auto subtreeFunctions = subtree->getAllFunctions();

        // Case by case, making sure we do not have duplicates
        for (const auto& [name, description] : subtreeFunctions) {
            if (functions.find(name) == functions.end()) {
                allFunctions.emplace_back(name, description);
            }
        }
    }
    return allFunctions;
}

template<typename RETURN_TYPE>
std::vector<std::pair<std::string, std::string>> FuncTree<RETURN_TYPE>::getAllVariables() {
    std::vector<std::pair<std::string, std::string>> allVariables;
    for (const auto& [name, info] : variables) {
        allVariables.emplace_back(name, info.description);
    }

    // Get from subtree
    if(subtree) {
        auto subtreeVariables = subtree->getAllVariables();

        // Case by case, making sure we do not have duplicates
        for (const auto& [name, description] : subtreeVariables) {
            if (variables.find(name) == variables.end()) {
                allVariables.emplace_back(name, description);
            }
        }
    }

    return allVariables;
}


//---------------------------------
// Constructor implementation

template<typename RETURN_TYPE>
FuncTree<RETURN_TYPE>::FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError, FuncTree<RETURN_TYPE>* subtree)
    : subtree(nullptr)  // Initialize to nullptr first to avoid issues during construction
{
    // Store name
    TreeName = treeName;

    // Attach the help function to read out the description of all attached functions
    functions["help"] = FunctionInfo{std::function<RETURN_TYPE(int, char**)>([this](int argc, char** argv) {
        return this->help(argc, argv);
    }), "Show available commands and their descriptions"};

    _standard = standard;
    _functionNotFoundError = functionNotFoundError;
    
    // Link the subTree AFTER basic initialization is complete
    if(subtree){
        this->subtree = subtree;
    }
}

//---------------------------------
// Parsing and execution

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
                std::cerr << "Warning: Unknown variable '--" << key << "' in Tree: '" << TreeName << "'\n";
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
    // Strip whitespaces of name
    std::string function = name;
    function = Nebulite::StringHandler::lstrip(name, ' ');
    function = Nebulite::StringHandler::rstrip(name, ' ');

    auto functionPosition = functions.find(function);
    if (functionPosition != functions.end()) {
        auto& [functionPtr, description] = functionPosition->second;
        return functionPtr(argc, argv);  // Call the function
    } else {
        std::cerr << "Function '" << function << "' not found.\n";
        return _functionNotFoundError;  // Return error if function not found
    }
}

template<typename RETURN_TYPE>
bool FuncTree<RETURN_TYPE>::hasFunction(const std::string& nameOrCommand) {
    // Make sure only the command name is used
    std::vector<std::string> tokens = Nebulite::StringHandler::split(nameOrCommand, ' ');

    // Remove all tokens starting with "--"
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
        [](const std::string& token) {
            return token.starts_with("--");
        }), tokens.end());

    // Remove all empty tokens
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
        [](const std::string& token) {
            return token.empty();
        }), tokens.end());

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
        // Keep empty tokens as extra whitespace
        // This is important, as the user explicitly specified an extra whitespace!
        // e.g. for text: "eval echo Value: {global.myVal}  |  Expected: {global.expected}"
        // So we shouldnt strip those!
        // The important part now is to strip those on command parsing!
        if (token.empty()) {
            if(!inQuoteV1 && !inQuoteV2) {
                // If not in quotes, just add an empty token
                result.push_back("");
                result.back() += " ";  // Keep the whitespace
            }
            else{
                result.back() += " ";
            }
            
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

//---------------------------------
// Help function

template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::help(int argc, char* argv[]) {
    // All info: [name, description]
    std::vector<std::pair<std::string, std::string>> allFunctions = getAllFunctions();
    std::vector<std::pair<std::string, std::string>> allVariables = getAllVariables();

    // Case-insensitive comparison function
    auto caseInsensitiveLess = [](const auto& a, const auto& b) {
        const std::string& sa = a.first;
        const std::string& sb = b.first;
        size_t n = std::min(sa.size(), sb.size());
        for (size_t i = 0; i < n; ++i) {
            char ca = std::tolower(static_cast<unsigned char>(sa[i]));
            char cb = std::tolower(static_cast<unsigned char>(sb[i]));
            if (ca < cb) return true;
            if (ca > cb) return false;
        }
        return sa.size() < sb.size();
    };

    // Sort by name
    std::sort(allFunctions.begin(), allFunctions.end(), caseInsensitiveLess);
    std::sort(allVariables.begin(), allVariables.end(), caseInsensitiveLess);

    // Display:
    std::cout << "\n\tHelp for " << TreeName << "\n\n";
    std::cout << "Available functions:\n";
    for (const auto& [name, description] : allFunctions) {
        std::cout << "  " << std::setw(25) << std::left << name
                  << " - " << description << std::endl;
    }

    // Display variables
    std::cout << "Available variables:\n";
    for (const auto& [name, description] : allVariables) {
        std::string fullName = "--" + name;  // Prefix with --
        std::cout << "  " << std::setw(25) << std::left << fullName
                  << " - " << description << std::endl;
    }
    return _standard;
}
