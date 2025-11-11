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
 * int main(int argc,  char** argv){
 *     FuncTree<std::string> funcTree("Nebulite", "ok", "Function not found");
 *     funcTree.bindFunction([](int argc,  char** argv){
 *         // Function implementation
 *         return "Function executed";
 *     }, "myFunction", "This function does something");
 *
 *     std::string result = funcTree.parse(argc,argv);
 *     Utility::Capture::cout() << result << "\n";
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

#ifndef NEBULITE_INTERACTION_EXECUTION_FUNCTREE_HPP
#define NEBULITE_INTERACTION_EXECUTION_FUNCTREE_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <utility>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Utility/Capture.hpp"        // Using Capture for output capturing

//------------------------------------------
namespace Nebulite::Interaction::Execution{
/**
 * @class Nebulite::Interaction::Execution::FuncTree
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
 * int foo(std::span<std::string const> const& args){Utility::Capture::cout() << "foo: " << myArgument << Utility::Capture::endl}
 * int bar(std::span<std::string const> const& args){Utility::Capture::cout() << "bar: " << myArgument << Utility::Capture::endl}
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
template<typename RETURN_TYPE, typename... additionalArgs>
class FuncTree {
public:
    // Make sure all FuncTrees are friends
    template<typename RT, typename... AA>
    friend class FuncTree;

    // canonical span function type (no reference-qualified std::function)
    using SpanArgs = std::span<std::string const>;
    using SpanFn = std::function<RETURN_TYPE (SpanArgs const&, additionalArgs...)>;

    // Function pointer type
    using FunctionPtr = std::variant<
        // Legacy (goal is to rewrite all functions to modern style, so we can remove these eventually)
        std::function<RETURN_TYPE (int, char**)>,
        std::function<RETURN_TYPE (int, char const**)>,
        // Modern
        SpanFn
    >;

    // Function pointer with class type
    template<typename ClassType>
    using MemberMethod = std::variant<
        // Legacy
        RETURN_TYPE (ClassType::*)(int, char**),
        RETURN_TYPE (ClassType::*)(int, char const**),
        // Modern
        RETURN_TYPE (ClassType::*)(SpanArgs, additionalArgs...)
        //, RETURN_TYPE (ClassType::*)(SpanArgs, additionalArgs...) const
    >;

    //------------------------------------------
    // Constructor and inheritance

    /**
     * @brief Constructor for the FuncTree class.
     * @param treeName Name of the tree
     * @param standard Value to return if everything is okay
     * @param functionNotFoundError Value to return if the parsed function was not found
     */
    FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError);

    /**
     * @brief Inherits functions from another Tree.
     * 
     * @param toInherit FuncTree pointer to inherit functions from.
     */
    void inherit(std::shared_ptr<FuncTree> toInherit){
        inheritedTrees.push_back(toInherit);
    }

    //------------------------------------------
    // Parsing

    /**
     * @brief Links a function to call before parsing (e.g., for setting up variables or locking resources)
     * @param func Function to call before parsing
     */
    void setPreParse(std::function<RETURN_TYPE()> func){
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
    RETURN_TYPE parseStr(std::string const& cmd);

    //------------------------------------------
    // Binding functions and variables

    /**
     * @brief Creates a category.
     * 
     * A category acts a "function bundler" to the main tree.
     * 
     * @param name Name of the category
     * @param helpDescription Pointer to description of the category, shown in the help command. First line is shown in the general help, full description in detailed help
     * @return true if the category was created successfully, 
     * false if a category with the same name already exists.
     */
    bool bindCategory(std::string const& name, std::string const* helpDescription);

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
    void bindFunction(ClassType* obj, MemberMethod<ClassType> method, std::string const& name, std::string const* helpDescription);

    /**
     * @brief Binds a variable to the command tree.
     * Make sure the variable is of type std::string*.
     * Once bound, it can be set via command line arguments: `--varName` (Must be before the function name!)
     * 
     * @param varPtr Pointer to the variable to bind
     * @param name Name of the variable in the command tree
     * @param helpDescription Help description for the variable. First line is shown in the general help, full description in detailed help.
     */
    void bindVariable(bool* varPtr, std::string const& name, std::string const* helpDescription);

private:
    // Function to call before parsing (e.g., for setting up variables or locking resources)
    std::function<RETURN_TYPE()> preParse = nullptr;

    // Function - Description pair
    struct FunctionInfo {
        FunctionPtr function;
        std::string const* description;
    };

    // Variable - Description pair
    struct VariableInfo {
        bool* pointer;
        std::string const* description;
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
    std::vector<std::shared_ptr<FuncTree>> inheritedTrees;

    /**
     * @struct category
     * @brief Represents a category within the FuncTree with its description.
     */
    struct category {
        std::unique_ptr<FuncTree> tree;
        std::string const* description;
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
    bool hasFunction(std::string const& nameOrCommand);
    
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
    RETURN_TYPE executeFunction(std::string const& name, int argc, char* argv[]);

    /**
     * @brief Displays help information to all bound functions. Automatically bound to any FuncTree on construction.
     */
    RETURN_TYPE help(std::span<std::string const> const& args);

    /**
     * @brief Retrieves a list of all functions and their descriptions.
     * 
     * @return A vector of pairs containing function names and their descriptions.
     */
    std::vector<std::pair<std::string, std::string const*>> getAllFunctions();

    /**
     * @brief Retrieves a list of all variables and their descriptions.
     * 
     * @return A vector of pairs containing variable names and their descriptions.
     */
    std::vector<std::pair<std::string, std::string const*>> getAllVariables();

    /**
     * @brief Help description for the help function.
     */
    std::string const help_desc = R"(Show available commands and their descriptions)";

    //------------------------------------------
    // Helper functions for better readability
    
    /**
     * @brief Displays detailed help for a specific function, category, or variable.
     */
    void specificHelp(std::string funcName);

    /**
     * @brief After calling find on each hashmap, this function takes a closer look at the results
     * and sets the found flags accordingly.
     */
    void find(std::string const& name, bool& funcFound, auto& funcIt,  bool& subFound, auto& subIt, bool& varFound, auto& varIt);

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
    void processVariableArguments(size_t& argc, char**& argv){
        while(argc > 0){
            if(std::string arg = argv[0]; arg.length() >= 2 && arg.substr(0, 2) == "--" /*same as arg.starts_with("--"), but C++17 compatible*/){
                // Extract name
                std::string name = arg.substr(2);

                // Set variable if attached
                // TODO: Search in inherited FuncTrees as well
                if (auto varIt = variables.find(name); varIt != variables.end()){
                    if (auto const& varInfo = varIt->second; varInfo.pointer){
                        *varInfo.pointer = true;
                    }
                } else {
                    Utility::Capture::cerr() << "Warning: Unknown variable '--" << name << "'\n";
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
    std::shared_ptr<FuncTree> findInInheritedTrees(std::string const& funcName){
        // Prerequisite if an inherited FuncTree is linked
        if(!inheritedTrees.empty() && !hasFunction(funcName)){
            // Check if the function is in an inherited tree
            for(auto& inheritedTree : inheritedTrees){
                if(inheritedTree != nullptr && inheritedTree->hasFunction(funcName)){
                    // Function is in inherited tree, parse there
                    return inheritedTree;
                }
            }
        }
        return nullptr;
    }

    //------------------------------------------
    // Binding Helper functions

    /**
     * @brief Checks if there is a binding conflict with the given function name. Prints an error message and exits if a conflict is found.
     * @param name The name of the function to check for conflicts.
     */
    void conflictCheck(std::string const& name);

    /**
     * @brief Binds a function directly to this FuncTree without checking for categories or conflicts.
     * @tparam ClassType The class type of the object instance
     * @param name The name of the function to bind
     * @param helpDescription Pointer to the help description for the function
     * @param method The member method to bind
     * @param obj The object instance that holds the member method
     */
    template<typename ClassType>
    void directBind(std::string const& name, std::string const* helpDescription, MemberMethod<ClassType> method, ClassType* obj);
};
} // namespace Nebulite::Interaction::Execution

#endif // NEBULITE_INTERACTION_EXECUTION_FUNCTREE_HPP

// Template implementation
#include "Interaction/Execution/FuncTree.tpp"