/**
 * @file FuncTree.hpp
 * 
 * This file defines the FuncTree class, which is responsible for managing and executing functions
 * through a command tree structure. 
 * The main goal of this class is to manage hierarchical commands and arguments for modular and flexible execution.
 * 
 * Example usage:
 * ```cpp
 * #include "Interaction/Execution/FuncTree.hpp"
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

#pragma once

//------------------------------------------
// Includes

// General
#include <iostream>
#include <vector>

// External
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Utility/StringHandler.hpp"  // Using StringHandler for easy argument splitting

namespace Nebulite{
namespace Interaction{
namespace Execution{
//------------------------------------------
/**
 * @class Nebulite::FuncTree
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
    // Make sure all FuncTrees are friends
    template<typename RT>
    friend class FuncTree;

    /**
     * @brief Constructor for the FuncTree class.
     * @param treeName Name of the tree
     * @param standard Value to return if everything is okay
     * @param functionNotFoundError Value to return if the parsed function was not found
     */
    FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError);

    /**
     * @brief Links a function to call before parsing (e.g., for setting up variables or locking resources)
     * @param func Function to call before parsing
     */
    void setPreParse(std::function<Nebulite::Constants::Error()> func){
        preParse = func;
    }

    /**
     * @brief Inherits functions from another Tree.
     * 
     * @param toInherit FuncTree pointer to inherit functions from.
     */
    void inherit(FuncTree<RETURN_TYPE>* toInherit) {
        inheritedTrees.push_back(toInherit);
    }

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
     * 
     * @param cmd Command string to parse
     * @return The return value of the executed function, or the standard/error value.
     */
    RETURN_TYPE parseStr(const std::string& cmd);

    /**
     * @brief Creates a subtree.
     * 
     * A subtree acts a "function bundler" to the main tree.
     * 
     * @param name Name of the subtree
     * @param description Description of the subtree, shown in the help command. First line is shown in the general help, full description in detailed help
     * @return true if the subtree was created successfully, 
     * false if a subtree with the same name already exists.
     */
    bool bindSubtree(const std::string& name, const std::string& description){
        if(subtrees.find(name) != subtrees.end()){
            // Subtree already exists
            /**
             * @note Warning is suppressed here, 
             * as with different modules we might need to call this in each module, 
             * just to make sure the subtree exists
             */
            // std::cerr << "Warning: A subtree with the name '" << name << "' already exists in the FuncTree '" << TreeName << "'." << std::endl;
            return false;
        }
        subtrees[name] = {std::make_unique<FuncTree<RETURN_TYPE>>(name, _standard, _functionNotFoundError), description};
        return true;
    }

    /**
     * @brief Binds a function to the command tree.
     * 
     * Make sure the function has the signature:
     * ```cpp
     * RETURN_TYPE functionName(int argc, char** argv);
     * ```
     * 
     * @tparam ClassType The class type of the object instance
     * @param obj Pointer to the object instance (for member functions)
     * @param method Pointer to the member function to bind
     * @param name Name of the function in the command tree
     * @param helpDescription Help description for the function. First line is shown in the general help, full description in detailed help.
     */
    template<typename ClassType>
    void bindFunction(ClassType* obj, RETURN_TYPE (ClassType::*method)(int, char**), const std::string& name, const std::string& helpDescription);

    /**
     * @brief Binds a variable to the command tree.
     * Make sure the variable is of type std::string*.
     * Once bound, it can be set via command line arguments: --varName=value (Must be before the function name!)
     * A simple argument of `"--varName"` will set the value to `"true"`
     * 
     * @param varPtr Pointer to the variable to bind
     * @param name Name of the variable in the command tree
     * @param helpDescription Help description for the variable. First line is shown in the general help, full description in detailed help.
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
     * 
     * @param nameOrCommand Name of the function or full command string
     */
    bool hasFunction(const std::string& nameOrCommand);

    /**
     * @brief Gets the last parsed string.
     * 
     * @return The last parsed string.
     */
    std::string getLastParsedString() const {
        return lastParsedString;
    }

private:
    // Function to call before parsing (e.g., for setting up variables or locking resources)
    std::function<Nebulite::Constants::Error()> preParse = nullptr;

    //------------------------------------------
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

    // inherited FuncTrees linked to this tree
    std::vector<FuncTree<RETURN_TYPE>*> inheritedTrees;

    /**
     * @struct subtree
     * @brief Represents a subtree within the FuncTree with its description.
     */
    struct subtree {
        std::unique_ptr<FuncTree<RETURN_TYPE>> tree;
        std::string description;
    };

    /**
     * @brief Map of subtrees within the FuncTree.
     */
    absl::flat_hash_map<std::string, subtree> subtrees;

    //------------------------------------------
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
     * 
     * @todo Improve help output formatting:
     * ./bin/Nebulite help foo
     * and
     * ./bin/Nebulite foo help
     * should output the same, but the formatting is not ideal yet.
     * Perhaps some helper functions to retrieve and format all functions and variables could be useful.
     */
    RETURN_TYPE help(int argc, char* argv[]);

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

    /**
     * @brief Stores the last parsed string.
     */
    std::string lastParsedString;
};
}   // namespace Execution
}   // namespace Interaction
}   // namespace Nebulite

//------------------------------------------
// Binding helper

template<typename RETURN_TYPE>
template<typename ClassType>
void Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::bindFunction(ClassType* obj, RETURN_TYPE (ClassType::*method)(int, char**), const std::string& name, const std::string& help) {
    // If the name has a whitespace, the function has to be bound to a subtree
    if(name.find(' ') != name.npos){
        // Split the name by whitespace
        auto parts = Nebulite::Utility::StringHandler::split(name, ' ');
        if(parts.size() < 2){
            std::cerr << "Error: Invalid function name '" << name << "'." << std::endl;
            return;
        }
        std::string subtreeName = parts[0];

        // We re-join the rest of the parts to form the argument for the subtree
        // If there are more than 2 parts, the subtree will create a new subtree
        std::string functionName = parts[1];
        for(int i = 2; i < parts.size(); i++){
            functionName += parts[i];
        }

        // Check if the subtree exists
        if(subtrees.find(subtreeName) == subtrees.end()){
            // If the subtree does not exist, throw an exception
            // This only fails on an improper function binding,
            // meaning we tried to bind a function to a subtree that does not exist
            // exit the entire program
            std::cerr << "---------------------------------------------------------------" << std::endl;
            std::cerr << "A Nebulite FuncTree binding failed!" << std::endl;;
            std::cerr << "Error: Subtree '" << subtreeName << "' does not exist when trying to bind function '" << name << "'." << std::endl;;
            std::cerr << "Please create the subtree first using bindSubtree()." << std::endl;;
            std::cerr << "This Tree: " << TreeName << std::endl;
            exit(EXIT_FAILURE);
        }

        // Bind the function to the subtree
        subtrees[subtreeName].tree->bindFunction(obj, method, functionName, help);

        return; // Function bound to subtree, return
    }

    // Make sure the function to bind is not a subtree
    for (const auto& [subtreeName, subtree] : subtrees) {
        if (subtreeName == name) {
            std::cerr << "---------------------------------------------------------------" << std::endl;
            std::cerr << "A Nebulite FuncTree binding failed!" << std::endl;
            std::cerr << "Error: Cannot bind function '" << name << "' because a subtree with the same name already exists." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // Making sure the function name is not registered in the inherited FuncTree
    // Note: inherited FuncTree is checked only after constructor completes, so this should be safe
    // The only overwrite that is allowed is the help function
    for (const auto& inheritedTree : inheritedTrees) {
        if (inheritedTree && name != "help" && inheritedTree->hasFunction(name)) {
            // Throw a proper error
            // exit the entire program
            std::cerr << "---------------------------------------------------------------\n";
            std::cerr << "A Nebulite FuncTree initialization failed!\n";
            std::cerr << "Error: A bound Function already exists in the inherited FuncTree.\n";
            std::cerr << "Function overwrite is heavily discouraged and thus not allowed.\n";
            std::cerr << "Please choose a different name or remove the existing function.\n";
            std::cerr << "This Tree: " << TreeName << "\n";
            std::cerr << "inherited FuncTree:   " << inheritedTree->TreeName << "\n";
            std::cerr << "Function:  " << name << "\n";
            std::exit(EXIT_FAILURE);  // Exit with failure status

            // Just for completion, this will never be reached
            return;
        }
    }

    // Same for the own tree
    if (hasFunction(name)) {
        // Throw a proper error
        // exit the entire program
        std::cerr << "---------------------------------------------------------------\n";
        std::cerr << "Nebulite FuncTree initialization failed!\n";
        std::cerr << "Error: A bound Function already exists in this tree.\n";
        std::cerr << "Function overwrite is heavily discouraged and thus not allowed.\n";
        std::cerr << "Please choose a different name or remove the existing function.\n";
        std::cerr << "This Tree: " << TreeName << "\n";
        std::cerr << "Function:  " << name << "\n";
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
void Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription) {
    variables[name] = VariableInfo{varPtr, helpDescription};
}

//------------------------------------------
// Getter

template<typename RETURN_TYPE>
std::vector<std::pair<std::string, std::string>> Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::getAllFunctions() {
    std::vector<std::pair<std::string, std::string>> allFunctions;
    for (const auto& [name, info] : functions) {
        allFunctions.emplace_back(name, info.description);
    }

    // Get functions from inherited FuncTrees
    for(auto& inheritedTree : inheritedTrees) {
        auto inheritedFuncTreeFunctions = inheritedTree->getAllFunctions();
        for (const auto& [name, description] : inheritedFuncTreeFunctions) {
            if (functions.find(name) == functions.end()) {
                allFunctions.emplace_back(name, description);
            }
        }
    }

    // Get just the names of the subtrees
    for (const auto& [subtreeName, subtree] : subtrees) {
        allFunctions.emplace_back(subtreeName, subtree.description);
    }

    return allFunctions;
}

template<typename RETURN_TYPE>
std::vector<std::pair<std::string, std::string>> Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::getAllVariables() {
    std::vector<std::pair<std::string, std::string>> allVariables;
    for (const auto& [name, info] : variables) {
        allVariables.emplace_back(name, info.description);
    }

    // Get from inherited FuncTree
    for (auto& inheritedTree : inheritedTrees) {
        auto inheritedFuncTreeFunctions = inheritedTree->getAllVariables();

        // Case by case, making sure we do not have duplicates
        for (const auto& [name, description] : inheritedFuncTreeFunctions) {
            if (variables.find(name) == variables.end()) {
                allVariables.emplace_back(name, description);
            }
        }
    }

    return allVariables;
}

//------------------------------------------
// Constructor implementation

template<typename RETURN_TYPE>
Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError)
{
    // Store name
    TreeName = treeName;

    // Attach the help function to read out the description of all attached functions
    functions["help"] = FunctionInfo{std::function<RETURN_TYPE(int, char**)>([this](int argc, char** argv) {
        return this->help(argc, argv);
    }), "Show available commands and their descriptions"};

    _standard = standard;
    _functionNotFoundError = functionNotFoundError;
}

//------------------------------------------
// Parsing and execution

template<typename RETURN_TYPE>
RETURN_TYPE Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::parse(int argc, char* argv[]) {
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
RETURN_TYPE Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::parseStr(const std::string& cmd) {
    // Store last parsed string 
    lastParsedString = cmd;

    // Prerequisite if an inherited FuncTree is linked
    if(inheritedTrees.size() && !hasFunction(cmd)) {
        // Check if the function is in an inherited tree
        for(auto& inheritedTree : inheritedTrees) {
            if(inheritedTree != nullptr && inheritedTree->hasFunction(cmd)) {
                // Function is in inherited tree, parse there
                return inheritedTree->parseStr(cmd);
            }
        }
    }

    // Call preParse function if set
    if(preParse != nullptr){
        Nebulite::Constants::Error err = preParse();
        if(err != Nebulite::Constants::ErrorTable::NONE()){
            return err; // Return error if preParse failed
        }
    }

    // Quote-aware tokenization
    std::vector<std::string> tokens = Nebulite::Utility::StringHandler::parseQuotedArguments(cmd);

    // Convert to argc/argv
    int argc = static_cast<int>(tokens.size());
    std::vector<char*> argv;
    argv.reserve(argc + 1);
    for (auto& str : tokens) {
        argv.push_back(const_cast<char*>(str.c_str()));
    }
    argv.push_back(nullptr); // Null-terminate

    // Parse arguments in this FuncTree
    return parse(argc, argv.data());
}

template<typename RETURN_TYPE>
RETURN_TYPE Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::executeFunction(const std::string& name, int argc, char* argv[]) {
    // Strip whitespaces of name
    std::string function = name;
    function = Nebulite::Utility::StringHandler::lstrip(name, ' ');
    function = Nebulite::Utility::StringHandler::rstrip(name, ' ');
    auto functionPosition = functions.find(function);
    if (functionPosition != functions.end()) {
        auto& [functionPtr, description] = functionPosition->second;
        return functionPtr(argc, argv);  // Call the function
    } else {
        // Find function name in subtrees
        if(subtrees.find(function) != subtrees.end()){
            return subtrees[function].tree->parse(argc, argv);
        }
        else{
            std::cerr << "Function '" << function << "' not found in FuncTree " << TreeName << " or its SubTrees!\n";
            std::cerr << "Arguments are:" << std::endl;
            for(int i = 0; i < argc; i++){
                std::cerr << "argv[" << i << "] = '" << argv[i] << "'\n";
            }
            std::cerr << "Available functions: " << functions.size() << std::endl;
            std::cerr << "Available SubTrees:  " << subtrees.size()  << std::endl;
            return _functionNotFoundError;  // Return error if function not found
        }
    }
}

template<typename RETURN_TYPE>
bool Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::hasFunction(const std::string& nameOrCommand) {
    // Make sure only the command name is used
    std::vector<std::string> tokens = Nebulite::Utility::StringHandler::split(nameOrCommand, ' ');

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

    // Depending on token count, function name is at different positions
    std::string function;
    if(tokens.size() == 1){
        // Case 1:
        // Is a single function name. 
        // e.g.: "set"
        function = tokens[0];
    }
    else{
        // Case 2:
        // Is a full command
        // e.g.: <whereCommandComesFrom> set key value
        function = tokens[1];
    }
    
    // See if the function is linked
    return  (functions.find(function) != functions.end()) || 
            (subtrees.find(function)  != subtrees.end());
}


//------------------------------------------
// Help function

template<typename RETURN_TYPE>
RETURN_TYPE Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::help(int argc, char* argv[]) {
    //------------------------------------------
    // Case 1: Detailed help for a specific function, subtree or variable
    if(argc > 1){
        for(int i = 1; i < argc; i++){
            //------------------------------------------
            // Name
            std::string funcName = argv[i];

            //------------------------------------------
            // Find

           
            // Functions
            bool funcFound = false;
            auto funcIt = functions.find(funcName);
            if(funcIt != functions.end()){funcFound = true;}
            else{
                for(auto& inheritedTree : inheritedTrees){
                    if(inheritedTree != nullptr){
                        funcIt = inheritedTree->functions.find(funcName);
                    }
                    if(funcIt != inheritedTree->functions.end()){
                        funcFound = true;
                        break; // Found in inherited tree, stop searching
                    }
                }
            }

            // Subtrees
            bool subFound = false;
            auto subIt = subtrees.find(funcName);
            if(subIt != subtrees.end()){subFound = true;}
            else{
                for(auto& inheritedTree : inheritedTrees){
                    if(inheritedTree != nullptr){
                        subIt = inheritedTree->subtrees.find(funcName);
                    }
                    if(subIt != inheritedTree->subtrees.end()){
                        subFound = true;
                        break; // Found in inherited tree, stop searching
                    }
                }
            }


            // Variables
            bool varFound = false;
            auto varIt = variables.find(funcName);
            if(varIt != variables.end()){varFound = true;}
            else{
                for(auto& inheritedTree : inheritedTrees){
                    if(inheritedTree != nullptr){
                        varIt = inheritedTree->variables.find(funcName);
                    }
                    if(varIt != inheritedTree->variables.end()){
                        varFound = true;
                        break; // Found in inherited tree, stop searching
                    }
                }
            }

            //------------------------------------------
            // Print

            // 1.) Function
            if(funcFound){
                // Found function, display detailed help
                std::cout << "\nHelp for function '" << funcName << "':\n" << std::endl;
                std::cout << funcIt->second.description << "\n";
            }
            // 2.) Subtree
            else if(subFound){
                // Found subtree, display detailed help
                std::cout << "\nHelp for subtree '" << funcName << "':\n" << std::endl;
                std::cout << subIt->second.description << "\n";
                std::cout << "Subtree functions:\n";
                subIt->second.tree->help(0, nullptr); // Display all functions in the subtree
            }
            // 3.) Variable
            else if(varFound){
                // Found variable, display detailed help
                std::cout << "\nHelp for variable '--" << funcName << "':\n" << std::endl;
                std::cout << varIt->second.description << "\n";
            }
            // 4.) Not found
            else{
                std::cout << "Function or Subtree '" << funcName << "' not found in FuncTree '" << TreeName << "'.\n";
            }
        }
        return _standard;
    }

    //------------------------------------------
    // Case 2: General help for all functions, subtrees and variables

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
    std::cout << "\nHelp for " << TreeName << "\nAdd the entries name to the command for more details.\n";
    std::cout << "Available functions:\n";
    for (const auto& [name, description] : allFunctions) {
        // Only show the first line of the description
        std::string descriptionFirstLine = description;
        size_t newlinePos = description.find('\n');
        if (newlinePos != std::string::npos) {
            descriptionFirstLine = description.substr(0, newlinePos);
        }
        std::cout << "  " << std::setw(25) << std::left << name
                  << " - " << descriptionFirstLine << std::endl;
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
