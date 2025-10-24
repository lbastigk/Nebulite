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
 * int main(int argc,  char* argv[]) {
 *     FuncTree<std::string> funcTree("Nebulite", "ok", "Function not found");
 *     funcTree.bindFunction([](int argc,  char* argv[]) {
 *         // Function implementation
 *         return "Function executed";
 *     }, "myFunction", "This function does something");
 *
 *     std::string result = funcTree.parse(argc,argv);
 *     Nebulite::Utility::Capture::cout() << result << "\n";
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
#include "Utility/Capture.hpp"        // Using Capture for output capturing

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
 * bool myArgument = false;
 * int foo(int argc, char** argv){Nebulite::Utility::Capture::cout() << "foo: " << myArgument << Nebulite::Utility::Capture::endl}
 * int bar(int argc, char** argv){Nebulite::Utility::Capture::cout() << "bar: " << myArgument << Nebulite::Utility::Capture::endl}
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

    //------------------------------------------
    // Constructor and inheritance

    /**
     * @brief Constructor for the FuncTree class.
     * @param treeName Name of the tree
     * @param standard Value to return if everything is okay
     * @param functionNotFoundError Value to return if the parsed function was not found
     */
    FuncTree(const std::string& treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError);

    /**
     * @brief Inherits functions from another Tree.
     * 
     * @param toInherit FuncTree pointer to inherit functions from.
     */
    void inherit(std::shared_ptr<FuncTree<RETURN_TYPE>> toInherit) {
        inheritedTrees.push_back(toInherit);
    }

    //------------------------------------------
    // Parsing

    /**
     * @brief Links a function to call before parsing (e.g., for setting up variables or locking resources)
     * @param func Function to call before parsing
     */
    void setPreParse(std::function<Nebulite::Constants::Error()> func){
        preParse = func;
    }

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

    //------------------------------------------
    // Binding functions and variables

    /**
     * @brief Creates a category.
     * 
     * A category acts a "function bundler" to the main tree.
     * 
     * @param name Name of the category
     * @param description Description of the category, shown in the help command. First line is shown in the general help, full description in detailed help
     * @return true if the category was created successfully, 
     * false if a category with the same name already exists.
     */
    bool bindCategory(const std::string& name, const std::string* helpDescription){
        if(categories.find(name) != categories.end()){
            // Category already exists
            /**
             * @note Warning is suppressed here, 
             * as with different modules we might need to call this in each module, 
             * just to make sure the category exists
             */
            // Nebulite::Utility::Capture::cerr() << "Warning: A category with the name '" << name << "' already exists in the FuncTree '" << TreeName << "'." << Nebulite::Utility::Capture::endl;
            return false;
        }
        // Split based on whitespaces
        std::vector<std::string> categoryStructure = Nebulite::Utility::StringHandler::split(name, ' ');
        size_t depth = categoryStructure.size();

        absl::flat_hash_map<std::string, Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::category>* currentCategoryMap = &categories;
        for(size_t idx = 0; idx < depth; idx++){
            std::string currentCategoryName = categoryStructure[idx];

            if(idx < depth -1){
                // Not yet at last category
                if(currentCategoryMap->find(currentCategoryName) != currentCategoryMap->end()){
                    // Category exists, go deeper
                    currentCategoryMap = &(*currentCategoryMap)[currentCategoryName].tree->categories;
                }
                else{
                    // Category does not exist, throw error
                    Nebulite::Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because parent category '"
                              << currentCategoryName << "' does not exist." << Nebulite::Utility::Capture::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else{
                // Last category, create it, if it doesnt exist yet
                if(currentCategoryMap->find(currentCategoryName) != currentCategoryMap->end()){
                    // Category exists, throw error
                    Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
                    Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree initialization failed!\n";
                    Nebulite::Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because it already exists." << Nebulite::Utility::Capture::endl;
                    exit(EXIT_FAILURE);
                }
                // Create category
                (*currentCategoryMap)[currentCategoryName] = {std::make_unique<FuncTree<RETURN_TYPE>>(currentCategoryName, _standard, _functionNotFoundError), helpDescription};
            }
        }
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
    void bindFunction(ClassType* obj, std::variant<RETURN_TYPE (ClassType::*)(int, char**), RETURN_TYPE (ClassType::*)(int, const char**)> method, const std::string& name, const std::string* helpDescription);

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
    void bindVariable(bool* varPtr, const std::string& name, const std::string* helpDescription);

private:
    // Function to call before parsing (e.g., for setting up variables or locking resources)
    std::function<Nebulite::Constants::Error()> preParse = nullptr;

    // Function pointer type
    using FunctionPtr = std::variant<
        std::function<RETURN_TYPE(int, char**)>,
        std::function<RETURN_TYPE(int, const char**)>
    >;

    // Function - Description pair
    struct FunctionInfo {
        FunctionPtr function;
        const std::string* description;
    };

    // Variable - Description pair
    struct VariableInfo {
        bool* pointer;
        const std::string* description;
    };

    // Status "ok"
    RETURN_TYPE _standard;

    // Status "Function not found"
    RETURN_TYPE _functionNotFoundError;

    // Map for Functions: name -> (functionPtr, info)
    absl::flat_hash_map<std::string, FunctionInfo> functions;

    // Map for variables: name -> (pointer, info)
    absl::flat_hash_map<std::string, VariableInfo> variables;

    // Name of the tree, used for help and output
    std::string TreeName; 

    // inherited FuncTrees linked to this tree
    std::vector<std::shared_ptr<FuncTree<RETURN_TYPE>>> inheritedTrees;

    /**
     * @struct category
     * @brief Represents a category within the FuncTree with its description.
     */
    struct category {
        std::unique_ptr<FuncTree<RETURN_TYPE>> tree;
        const std::string* description;
    };

    /**
     * @brief Map of categories within the FuncTree.
     */
    absl::flat_hash_map<std::string, category> categories;

    //------------------------------------------
    // Functions

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
    RETURN_TYPE help(int argc,  const char* argv[]);

    /**
     * @brief Retrieves a list of all functions and their descriptions.
     * 
     * @return A vector of pairs containing function names and their descriptions.
     */
    std::vector<std::pair<std::string, const std::string*>> getAllFunctions();

    /**
     * @brief Retrieves a list of all variables and their descriptions.
     * 
     * @return A vector of pairs containing variable names and their descriptions.
     */
    std::vector<std::pair<std::string, const std::string*>> getAllVariables();

    /**
     * @brief Help description for the help function.
     */
    const std::string help_desc = R"(Show available commands and their descriptions)";

    //------------------------------------------
    // Splitted help functions for better readability
    
    /**
     * @brief Displays detailed help for a specific function, category, or variable.
     */
    void specificHelp(std::string funcName);

    /**
     * @brief After calling find on each hashmap, this function takes a closer look at the results
     * and sets the found flags accordingly.
     */
    void find(const std::string& name, bool& funcFound, auto& funcIt,  bool& subFound, auto& subIt, bool& varFound, auto& varIt);

    /**
     * @brief Displays general help for all functions, categories, and variables.
     */
    void generalHelp();

    //------------------------------------------
    // Argument processing helper

    /**
     * @brief Processes variable arguments at the start of the argument list.
     * 
     * @param argc Argument count
     * @param argv Argument vector
     */
    void processVariableArguments(int& argc, char**& argv){
        while(argc > 0){
            std::string arg = argv[0];
            if(arg.length() >= 2 && arg.substr(0, 2) == "--" /*same as arg.starts_with("--"), but C++17 compatible*/){
                // Extract name
                std::string name = arg.substr(2);

                // Set variable if attached
                // TODO: Search in inherited FuncTrees as well
                if (auto varIt = variables.find(name); varIt != variables.end()) {
                    const auto& varInfo = varIt->second;  // Now it's VariableInfo, not a pair
                    if (varInfo.pointer) {
                        *varInfo.pointer = true;
                    }
                } else {
                    Nebulite::Utility::Capture::cerr() << "Warning: Unknown variable '--" << name << "'\n";
                }

                // Remove from argument list
                argv++;       // Skip the first argument (function name)
                argc--;       // Reduce the argument count (function name is processed)
            }
            else{
                // no more vars to parse
                return;
            }
        }
    }

    /**
     * @brief Finds an argument in inherited FuncTrees.
     * 
     * @param funcName Name of the function to find
     * @return Pointer to the FuncTree where the function was found, or nullptr if not found.
     */
    std::shared_ptr<FuncTree<RETURN_TYPE>> findInInheritedTrees(const std::string& funcName){
        // Prerequisite if an inherited FuncTree is linked
        if(inheritedTrees.size() && !hasFunction(funcName)) {
            // Check if the function is in an inherited tree
            for(auto& inheritedTree : inheritedTrees) {
                if(inheritedTree != nullptr && inheritedTree->hasFunction(funcName)) {
                    // Function is in inherited tree, parse there
                    return inheritedTree;
                }
            }
        }
        return nullptr;
    }
};
}   // namespace Execution
}   // namespace Interaction
}   // namespace Nebulite

//------------------------------------------
// Bindinging error messages

namespace bindErrorMessage{
    inline void MissingCategory(const std::string& tree, const std::string& category, const std::string& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "Error: Category '" << category << "' does not exist when trying to bind function '" << function << "'." << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "Please create the category hierarchy first using bindCategory()." << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << Nebulite::Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    inline void FunctionShadowsCategory(const std::string& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "Error: Cannot bind function '" << function << "' because a category with the same name already exists." << Nebulite::Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    inline void FunctionExistsInInheritedTree(const std::string& tree, const std::string& inheritedTree, const std::string& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
        Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree initialization failed!\n";
        Nebulite::Utility::Capture::cerr() << "Error: A bound Function already exists in the inherited FuncTree.\n";
        Nebulite::Utility::Capture::cerr() << "Function overwrite is heavily discouraged and thus not allowed.\n";
        Nebulite::Utility::Capture::cerr() << "Please choose a different name or remove the existing function.\n";
        Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << "\n";
        Nebulite::Utility::Capture::cerr() << "inherited FuncTree:   " << inheritedTree << "\n";
        Nebulite::Utility::Capture::cerr() << "Function:  " << function << "\n";
        std::exit(EXIT_FAILURE);
    }

    inline void FunctionExists(const std::string& tree, const std::string& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
        Nebulite::Utility::Capture::cerr() << "Nebulite FuncTree initialization failed!\n";
        Nebulite::Utility::Capture::cerr() << "Error: A bound Function already exists in this tree.\n";
        Nebulite::Utility::Capture::cerr() << "Function overwrite is heavily discouraged and thus not allowed.\n";
        Nebulite::Utility::Capture::cerr() << "Please choose a different name or remove the existing function.\n";
        Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << "\n";
        Nebulite::Utility::Capture::cerr() << "Function:  " << function << "\n";
        std::exit(EXIT_FAILURE);
    }
} // namespace bindError

template<typename RETURN_TYPE>
template<typename ClassType>
void Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::bindFunction(
    ClassType* obj,
    std::variant<RETURN_TYPE (ClassType::*)(int, char**), RETURN_TYPE (ClassType::*)(int, const char**)> method,
    const std::string& name,
    const std::string* helpDescription)
{
    // If the name has a whitespace, the function has to be bound to a category hierarchically
    if(name.find(' ') != name.npos){
        std::vector<std::string> pathStructure = Nebulite::Utility::StringHandler::split(name, ' ');
        if(pathStructure.size() < 2){
            Nebulite::Utility::Capture::cerr() << "Error: Invalid function name '" << name << "'." << Nebulite::Utility::Capture::endl;
            return;
        }
        absl::flat_hash_map<std::string, Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::category>* currentCategoryMap = &categories;
        FuncTree<RETURN_TYPE>* targetTree = this;
        for(size_t idx = 0; idx < pathStructure.size() - 1; idx++){
            std::string currentCategoryName = pathStructure[idx];
            if(currentCategoryMap->find(currentCategoryName) == currentCategoryMap->end()){
                bindErrorMessage::MissingCategory(TreeName, currentCategoryName, name);
            }
            targetTree = (*currentCategoryMap)[currentCategoryName].tree.get();
            currentCategoryMap = &targetTree->categories;
        }
        std::string functionName = pathStructure.back();
        targetTree->bindFunction(obj, method, functionName, helpDescription);
        return;
    }
    for (const auto& [categoryName, category] : categories) {
        if (categoryName == name) {
            bindErrorMessage::FunctionShadowsCategory(name);
        }
    }
    auto conflictIt = std::find_if(
        inheritedTrees.begin(), inheritedTrees.end(),
        [&](const auto& inheritedTree) {
            return inheritedTree && name != "help" && inheritedTree->hasFunction(name);
        }
    );
    if (conflictIt != inheritedTrees.end()) {
        auto conflictTree = *conflictIt;
        bindErrorMessage::FunctionExistsInInheritedTree(TreeName, conflictTree->TreeName, name);
    }
    if (hasFunction(name)) {
        bindErrorMessage::FunctionExists(TreeName, name);
    }

    // Use std::visit to bind the correct function type
    std::visit([&](auto&& mptr) {
        using MethodType = std::decay_t<decltype(mptr)>;
        if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(int, char**)>) {
            functions[name] = FunctionInfo{
                [obj, mptr](int argc, char** argv) {
                    return (obj->*mptr)(argc, argv);
                },
                helpDescription
            };
        } else if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(int, const char**)>) {
            functions[name] = FunctionInfo{
                [obj, mptr](int argc, char** argv) {
                    std::vector<const char*> argv_const(argc);
                    for (int i = 0; i < argc; ++i) argv_const[i] = argv[i];
                    return (obj->*mptr)(argc, argv_const.data());
                },
                helpDescription
            };
        }
    }, method);
}



template<typename RETURN_TYPE>
void Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::bindVariable(bool* varPtr, const std::string& name, const std::string* helpDescription) {
    // Make sure there are no whitespaces in the variable name
    if (name.find(' ') != name.npos) {
        Nebulite::Utility::Capture::cerr() << "Error: Variable name '" << name << "' cannot contain whitespaces." << Nebulite::Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    // Make sure the variable isnt bound yet
    if (variables.find(name) != variables.end()) {
        Nebulite::Utility::Capture::cerr() << "Error: Variable '" << name << "' is already bound." << Nebulite::Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }
    
    // Bind the variable
    variables[name] = VariableInfo{varPtr, helpDescription};
}

//------------------------------------------
// Getter

template<typename RETURN_TYPE>
std::vector<std::pair<std::string, const std::string*>> Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::getAllFunctions() {
    std::vector<std::pair<std::string, const std::string*>> allFunctions;
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

    // Get just the names of the categories
    for (const auto& [categoryName, category] : categories) {
        allFunctions.emplace_back(categoryName, category.description);
    }

    return allFunctions;
}

template<typename RETURN_TYPE>
std::vector<std::pair<std::string, const std::string*>> Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::getAllVariables() {
    std::vector<std::pair<std::string, const std::string*>> allVariables;
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

template <typename RETURN_TYPE>
Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::FuncTree(const std::string& treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError)
: _standard(standard), _functionNotFoundError(functionNotFoundError), TreeName(treeName)
{
    // Attach the help function to read out the description of all attached functions
    functions["help"] = FunctionInfo{std::function<RETURN_TYPE(int, const char**)>([this](int argc, const char** argv) {
        return this->help(argc, argv);
    }), &help_desc};
}

//------------------------------------------
// Parsing and execution

template<typename RETURN_TYPE>
RETURN_TYPE Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::parseStr(const std::string& cmd) {
    // Quote-aware tokenization
    std::vector<std::string> tokens = Nebulite::Utility::StringHandler::parseQuotedArguments(cmd);

    // Convert to argc/argv
    int argc = static_cast<int>(tokens.size());
    std::vector<char*> argv_vec;
    argv_vec.reserve(argc + 1);
    std::transform(tokens.begin(), tokens.end(), std::back_inserter(argv_vec),
        [](const std::string& str) { return const_cast<char*>(str.c_str()); });
    argv_vec.push_back(nullptr); // Null-terminate

    // First argument is binary name or last function name
    // remove it from the argument list
    char **argv = argv_vec.data();
    argv++;
    argc--;

    // Process arguments directly after binary/function name (like --count or -c)
    processVariableArguments(argc, argv);

    // Check if there are still arguments left
    if(argc == 0){
        return _standard;   // Nothing to execute, return standard
    }

    // The first argument left is the new function name
    std::string funcName = argv[0];

    // Check in inherited FuncTrees first
    auto inheritedTree = findInInheritedTrees(funcName);
    if(inheritedTree != nullptr){
        // Function is in inherited tree, parse there
        return inheritedTree->executeFunction(funcName, argc, argv);
    }

    // Not found in inherited trees, execute the function the main tree
    return executeFunction(funcName, argc, argv);
}

template<typename RETURN_TYPE>
RETURN_TYPE Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::executeFunction(const std::string& name, int argc, char* argv[]) {
    // Call preParse function if set
    if(preParse != nullptr){
        Nebulite::Constants::Error err = preParse();
        if(err != Nebulite::Constants::ErrorTable::NONE()){
            return err; // Return error if preParse failed
        }
    }

    // Strip whitespaces of name
    std::string function = name;
    function = Nebulite::Utility::StringHandler::lstrip(function, ' ');
    function = Nebulite::Utility::StringHandler::rstrip(function, ' ');

    // Find and execute the function
    auto functionPosition = functions.find(function);
    if (functionPosition != functions.end()) {
        auto& [functionPtr, description] = functionPosition->second;
        return std::visit(
            [&](auto&& func) -> RETURN_TYPE {
                using T = std::decay_t<decltype(func)>;
                if constexpr (std::is_same_v<T, std::function<RETURN_TYPE(int, char**)>>) {
                    return func(argc, argv);
                } else if constexpr (std::is_same_v<T, std::function<RETURN_TYPE(int, const char**)>>) {
                    std::vector<const char*> argv_const(argc);
                    for (int i = 0; i < argc; ++i) argv_const[i] = argv[i];
                    return func(argc, argv_const.data());
                }
            },
            functionPtr
        );
    } else {
        // Find function name in categories
        if(categories.find(function) != categories.end()){
            std::string cmd = "";
            for(int i = 0; i < argc; i++){
                cmd += std::string(argv[i]) + " ";
            }
            return categories[function].tree->parseStr(cmd);
        }
        else{
            Nebulite::Utility::Capture::cerr() << "Function '" << function << "' not found in FuncTree " << TreeName << " or its SubTrees!\n";
            Nebulite::Utility::Capture::cerr() << "Arguments are:" << Nebulite::Utility::Capture::endl;
            for(int i = 0; i < argc; i++){
                Nebulite::Utility::Capture::cerr() << "argv[" << i << "] = '" << argv[i] << "'\n";
            }
            Nebulite::Utility::Capture::cerr() << "Available functions: " << functions.size() << Nebulite::Utility::Capture::endl;
            Nebulite::Utility::Capture::cerr() << "Available SubTrees:  " << categories.size()  << Nebulite::Utility::Capture::endl;
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
            (categories.find(function)  != categories.end());
}

//------------------------------------------
// Help function and its helpers

template<typename RETURN_TYPE>
RETURN_TYPE Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::help(int argc, const char* argv[]) {
    //------------------------------------------
    // Case 1: Detailed help for a specific function, category or variable
    if(argc > 1){
        for(int i = 1; i < argc; i++){
            std::string funcName = argv[i];
            specificHelp(funcName);
        }
        return _standard;
    }

    //------------------------------------------
    // Case 2: General help for all functions, categories and variables
    generalHelp();
    return _standard;
}

template<typename RETURN_TYPE>
void Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::specificHelp(std::string funcName) {
    //------------------------------------------
    // Find
    bool funcFound = false;
    bool subFound = false;
    bool varFound = false;
    auto funcIt = functions.find(funcName);
    auto subIt = categories.find(funcName);
    auto varIt = variables.find(funcName);
    find(funcName, funcFound, funcIt, subFound, subIt, varFound, varIt);

    //------------------------------------------
    // Print

    // 1.) Function
    if(funcFound){
        // Found function, display detailed help
        Nebulite::Utility::Capture::cout() << "\nHelp for function '" << funcName << "':\n" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cout() << *funcIt->second.description << "\n";
    }
    // 2.) Category
    else if(subFound){
        // Found category, display detailed help
        subIt->second.tree->help(0, nullptr); // Display all functions in the category
    }
    // 3.) Variable
    else if(varFound){
        // Found variable, display detailed help
        Nebulite::Utility::Capture::cout() << "\nHelp for variable '--" << funcName << "':\n" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cout() << *varIt->second.description << "\n";
    }
    // 4.) Not found
    else{
        Nebulite::Utility::Capture::cerr() << "Function or Category '" << funcName << "' not found in FuncTree '" << TreeName << "'.\n";
    }
}

template<typename RETURN_TYPE>
void Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::generalHelp() {
    // All info: [name, description]
    std::vector<std::pair<std::string, const std::string*>> allFunctions = getAllFunctions();
    std::vector<std::pair<std::string, const std::string*>> allVariables = getAllVariables();

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
    Nebulite::Utility::Capture::cout() << "\nHelp for " << TreeName << "\nAdd the entries name to the command for more details: " << TreeName << " help <foo>\n";
    Nebulite::Utility::Capture::cout() << "Available functions:\n";
    for (const auto& [name, description] : allFunctions) {
        // Only show the first line of the description
        std::string descriptionFirstLine = *description;
        size_t newlinePos = description->find('\n');
        if (newlinePos != std::string::npos) {
            descriptionFirstLine = description->substr(0, newlinePos);
        }
        std::string paddedName = name;
        paddedName.resize(25, ' ');
        Nebulite::Utility::Capture::cout() << "  " << paddedName << " - " << descriptionFirstLine << Nebulite::Utility::Capture::endl;
    }

    // Display variables
    Nebulite::Utility::Capture::cout() << "Available variables:\n";
    for (const auto& [name, description] : allVariables) {
        std::string fullName = "--" + name;  // Prefix with --
        std::string paddedName = name;
        paddedName.resize(25, ' ');
        Nebulite::Utility::Capture::cout() << "  " << paddedName << " - " << *description << Nebulite::Utility::Capture::endl;
    }
}

template<typename RETURN_TYPE>
void Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::find(const std::string& name, bool& funcFound, auto& funcIt,  bool& subFound, auto& subIt, bool& varFound, auto& varIt){
    // Functions
    if(funcIt != functions.end()){funcFound = true;}
    else{
        for(const auto& inheritedTree : inheritedTrees){
            if(inheritedTree != nullptr){
                funcIt = inheritedTree->functions.find(name);
            }
            if(funcIt != inheritedTree->functions.end()){
                funcFound = true;
                break; // Found in inherited tree, stop searching
            }
        }
    }

    // Categories
    if(subIt != categories.end()){subFound = true;}
    else{
        for(const auto& inheritedTree : inheritedTrees){
            if(inheritedTree != nullptr){
                subIt = inheritedTree->categories.find(name);
            }
            if(subIt != inheritedTree->categories.end()){
                subFound = true;
                break; // Found in inherited tree, stop searching
            }
        }
    }

    // Variables
    if(varIt != variables.end()){varFound = true;}
    else{
        for(const auto& inheritedTree : inheritedTrees){
            if(inheritedTree != nullptr){
                varIt = inheritedTree->variables.find(name);
            }
            if(varIt != inheritedTree->variables.end()){
                varFound = true;
                break; // Found in inherited tree, stop searching
            }
        }
    }
}
