/**
 * @file FuncTree.hpp
 * @brief This file defines the FuncTree class, which is responsible for managing and executing functions
 *        through a command tree structure.
 *        The main goal of this class is to manage hierarchical commands and arguments for modular and flexible execution.
 * 
 *        Example usage:
 *        ```cpp
 *        #include "Interaction/Execution/FuncTree.hpp"
 *        int main(int argc,  char** argv){
 *            FuncTree<std::string> funcTree("Nebulite", "ok", "Function not found");
 *            funcTree.bindFunction([](int argc, char** argv){
 *                // Function implementation
 *                return "Function executed";
 *            }, "myFunction", "This function does something");
 *
 *            std::string result = funcTree.parse(argc,argv);
 *            Nebulite::cout() << result << "\n";
 *        }
 *        ```
 *
 *        This will parse the command-line arguments and execute the "myFunction" if it is called:
 *        ```bash
 *        ./main myFunction        #-> cout: "Function executed"
 *        ./main                   #-> cout: "ok" (SUCCESS: no function called)
 *        ./main help              #-> cout: shows available commands and their descriptions
 *        ./main someOtherFunction #-> cout: "Function not found",
 *                                 #   cerr: "Function 'someOtherFunction' not found."
 *        ```
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
#include "Utility/Capture.hpp"  // Due to circular dependencies, we use Capture for logging instead of Nebulite.hpp

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @class Nebulite::Interaction::Execution::FuncTree
 * @brief Function tree class for managing and executing functions through linguistic commands.
 *        The FuncTree class allows for the binding of functions and variables to a command tree structure,
 *        enabling modular and flexible execution of commands based on user input.
 *        Functions are identified by their names and can have multiple arguments.
 *        Variables can be bound to the command tree and accessed within functions,
 *        provided the functions themselves have access to the space of the variables.
 *
 *        Example:
 *
 *        ```cpp
 *        bool myArgument = false;
 *        int foo(std::span<std::string const> const& args){Nebulite::Utility::Capture::cerr() << "foo: " << myArgument << Nebulite::Utility::Capture::endl}
 *        int bar(std::span<std::string const> const& args){Nebulite::Utility::Capture::cerr() << "bar: " << myArgument << Nebulite::Utility::Capture::endl}
 *
 *        // Note: The description actually needs to be a pointer to a string, so that it can be stored without copying.
 *        // The snippet below is simplified for clarity.
 *        FuncTree<int> funcTree;
 *        funcTree.bindCategory("myCategory", "This is my category");
 *        funcTree.bindVariable(&myArgument, "myArgument", "This is my argument");
 *        funcTree.bindFunction(&foo, "foo",            "This function does foo");
 *        funcTree.bindFunction(&bar, "myCategory bar", "This function does bar and is only callable via the category");
 *
 *        std::string command = "FromExample --myArgument foo";
 *        funcTree.parseStr(command);  // output: "foo: true"
 *        ```
 */
template <typename returnValue, typename... additionalArgs>
class FuncTree {
public:
    //------------------------------------------
    // Access

    // Make sure all FuncTrees are friends
    template <typename RT, typename... AA>
    friend class FuncTree;

    //------------------------------------------
    // Important types

    // canonical span function type (no reference-qualified std::function)
    using SpanArgs = std::span<std::string const>;
    using SpanArgsConstRef = std::span<std::string const> const&;
    using SpanFn = std::function<returnValue (SpanArgs, additionalArgs...)>;
    using SpanFnConstRef = std::function<returnValue (SpanArgsConstRef, additionalArgs...)>;

    // Function pointer type
    using FunctionPtr = std::variant<
        // Legacy (goal is to rewrite all functions to modern style, so we can remove these eventually)
        std::function<returnValue (int, char**)>,
        std::function<returnValue (int, char const**)>,
        // Modern
        SpanFn,
        SpanFnConstRef
    >;

    /**
     * @todo Allow for the binding of static functions as well!
     */

    // Function pointer with class type
    template <typename ClassType>
    using MemberMethod = std::variant<
        // Legacy
        returnValue (ClassType::*)(int, char**),
        returnValue (ClassType::*)(int, char const**),
        // Modern
        returnValue (ClassType::*)(SpanArgs, additionalArgs...),
        returnValue (ClassType::*)(SpanArgs, additionalArgs...) const,
        returnValue (ClassType::*)(SpanArgsConstRef, additionalArgs...),
        returnValue (ClassType::*)(SpanArgsConstRef, additionalArgs...) const
    >;

    //------------------------------------------
    // Constructor and inheritance

    /**
     * @brief Constructor for the FuncTree class.
     * @param treeName Name of the tree
     * @param valDefault Value to return if everything is okay
     * @param valFunctionNotFound Value to return if the parsed function was not found
     */
    FuncTree(std::string treeName, returnValue valDefault, returnValue valFunctionNotFound);

    /**
     * @brief Inherits functions from another Tree.
     * 
     * @param toInherit FuncTree pointer to inherit functions from.
     */
    void inherit(std::shared_ptr<FuncTree> toInherit) {
        inheritedTrees.push_back(toInherit);
    }

    //------------------------------------------
    // Parsing

    /**
     * @brief Links a function to call before parsing (e.g., for setting up variables or locking resources)
     * @param func Function to call before parsing
     */
    void setPreParse(std::function<returnValue()> func) {
        preParse = func;
    }

    /**
     * @brief Parses the command line arguments and executes the corresponding function.
     *        All whitespaces outside of quotes are seen as argument separators.
     *        The first argument should be used to identify where the execution comes from.
     *        All subsequent arguments starting with -- are treated as variable assignments.
     *        The first argument after the variable assignments is the function to execute.
     *
     *        Example:
     *
     *        ```cpp
     *        // Input string:
     *        "./bin/Nebulite --headless if '$({myCondition} and {myOtherCondition})' echo here!"
     *        // Result being parsed:
     *        argv[0] = "./bin/Nebulite"                           // The executable name or any other name specified on FuncTree construction
     *        argv[1] = "--headless"                               // Variable assignment
     *        argv[2] = "if"                                       // The function being executed
     *        argv[3] = "$({myCondition} and {myOtherCondition})"  // due to the quotes, this is treated as a single argument
     *        argv[4] = "echo"                                     // function argument
     *        argv[5] = "here!"                                    // function argument
     *        ```
     *
     *        The following arguments are passed to the executed function:
     *
     *        - argv[0] = "if"
     *
     *        - argv[1] = "$({myCondition} and {myOtherCondition})"
     *
     *        - argv[2] = "echo"
     *
     *        - argv[3] = "here!"
     * @param cmd Command string to parse
     * @param addArgs Additional arguments to pass to the executed function
     * @return The return value of the executed function, or the standard/error value.
     */
    returnValue parseStr(std::string const& cmd, additionalArgs... addArgs);

    //------------------------------------------
    // Binding (Functions, Categories, Variables)

    /**
     * @brief Creates a category.
     *        A category acts a "function bundler" to the main tree.
     * @param name Name of the category
     * @param helpDescription Pointer to description of the category, shown in the help command. First line is shown in the general help, full description in detailed help
     * @return true if the category was created successfully, 
     * false if a category with the same name already exists.
     */
    bool bindCategory(std::string const& name, std::string const* helpDescription);

    /**
     * @brief Binds a function to the command tree.
     *        Make sure the function has the signature:
     *        ```cpp
     *        returnValue functionName(int argc, char** argv);
     *        ```
     * @tparam ClassType The class type of the object instance
     * @param obj Pointer to the object instance (for member functions)
     * @param method Pointer to the member function to bind
     * @param name Name of the function in the command tree
     * @param helpDescription Help description for the function. First line is shown in the general help, full description in detailed help.
     */
    template <typename ClassType>
    void bindFunction(ClassType* obj, MemberMethod<ClassType> method, std::string const& name, std::string const* helpDescription);

    /**
     * @brief Binds a variable to the command tree.
     *        Make sure the variable is of type std::string*.
     *        Once bound, it can be set via command line arguments: `--varName` (Must be before the function name!)
     * @param varPtr Pointer to the variable to bind
     * @param name Name of the variable in the command tree
     * @param helpDescription Help description for the variable. First line is shown in the general help, full description in detailed help.
     */
    void bindVariable(bool* varPtr, std::string const& name, std::string const* helpDescription);

private:
    // Name of the tree, used for help and output
    std::string TreeName;

    // Function to call before parsing (e.g., for setting up variables or locking resources)
    std::function<returnValue()> preParse = nullptr;

    struct StandardReturnValues {
        returnValue valDefault;
        returnValue valFunctionNotFound;
    } standardReturn;

    /**
     * @struct CategoryInfo
     * @brief Represents a category within the FuncTree with its description.
     */
    struct CategoryInfo {
        std::unique_ptr<FuncTree> tree;
        std::string const* description;
    };

    /**
     * @struct FunctionInfo
     * @brief Contains information about a bound function, including its pointer and description.
     */
    struct FunctionInfo {
        FunctionPtr function;
        std::string const* description;
    };

    /**
     * @struct VariableInfo
     * @brief Contains information about a bound variable, including its pointer and description.
     */
    struct VariableInfo {
        bool* pointer;
        std::string const* description;
    };

    // inherited FuncTrees linked to this tree
    std::vector<std::shared_ptr<FuncTree>> inheritedTrees;

    /**
     * @struct BindingContainer
     * @brief Contains all bindings for categories, functions, and variables.
     */
    struct BindingContainer {
        // Map for Categories: name -> (category, info)
        absl::flat_hash_map<std::string, CategoryInfo> categories;

        // Map for Functions: name -> (functionPtr, info)
        absl::flat_hash_map<std::string, FunctionInfo> functions;

        // Map for variables: name -> (pointer, info)
        absl::flat_hash_map<std::string, VariableInfo> variables;
    } bindingContainer;

    //------------------------------------------
    // Functions: basic functionality

    /**
     * @brief Checks if a function with the given name or from a full command exists.
     *        Examples:
     *        ```cpp
     *        // Both check if the function "myFunction" exists
     *        funcTree.hasFunction("myFunction");
     *        funcTree.hasFunction("./bin/Nebulite --myVariable myFunction argumentOfMyFunction");
     *        ```
     * @param nameOrCommand Name of the function or full command string
     */
    bool hasFunction(std::string const& nameOrCommand);

    /**
     * @brief Looks up the function by name and calls it with the provided arguments.
     * @param name The name of the function to execute.
     * @param argc [Legacy] Argument count.
     * @param argv [Legacy] Argument vector.
     * @param args Modern argument span.
     * @param addArgs Additional arguments to pass to the function.
     * @return The return value of the function.
     */
    returnValue executeFunction(std::string const& name, int argc, char** argv, std::span<std::string const> const& args, additionalArgs... addArgs);

    /**
     * @brief Displays help information to all bound functions. Automatically bound to any FuncTree on construction.
     */
    returnValue help(std::span<std::string const> const& args, additionalArgs... addArgs);

    /**
     * @brief Retrieves a list of all functions and their descriptions.
     * @return A vector of pairs containing function names and their descriptions.
     */
    std::vector<std::pair<std::string, std::string const*>> getAllFunctions();

    /**
     * @brief Retrieves a list of all variables and their descriptions.
     * @return A vector of pairs containing variable names and their descriptions.
     */
    std::vector<std::pair<std::string, std::string const*>> getAllVariables();

    /**
     * @brief Help description for the help function.
     */
    std::string const help_desc = R"(Show available commands and their descriptions)";

    /**
     * @brief Help description for the complete function.
     */
    std::string const complete_desc = R"(Provide command completion suggestions based on the current arguments)";

    //------------------------------------------
    // Helper functions for better readability

    /**
     * @brief Displays detailed help for a specific function, category, or variable.
     */
    void specificHelp(std::string const& funcName, additionalArgs... addArgs);

    /**
     * @struct BindingSearchResult
     * @brief Helper struct to store search results for find
     */
    struct BindingSearchResult {
        bool any = false;
        bool function = false;
        bool category = false;
        bool variable = false;

        absl::flat_hash_map<std::string, CategoryInfo>::iterator catIt;
        absl::flat_hash_map<std::string, FunctionInfo>::iterator funIt;
        absl::flat_hash_map<std::string, VariableInfo>::iterator varIt;
    };

    /**
     * @brief After calling find on each hashmap, this function takes a closer look at the results
     *        and sets the found flags accordingly.
     * @param name Name of the function/category/variable to find
     * @return SearchResult struct containing found flags
     */
    BindingSearchResult find(std::string const& name);

    /**
     * @brief Displays general help for all functions, categories, and variables.
     */
    void generalHelp();

    //------------------------------------------
    // Argument processing helper

    /**
     * @brief Processes variable arguments at the start of the argument list.
     * @param argc Argument count
     * @param argv Argument vector
     */
    void processVariableArguments(size_t& argc, char**& argv) {
        while (argc > 0) {
            if (std::string arg = argv[0]; arg.length() >= 2 && arg.substr(0, 2) == "--" /*same as arg.starts_with("--"), but C++17 compatible*/) {
                // Extract name
                std::string name = arg.substr(2);

                // Set variable if attached
                // TODO: Search in inherited FuncTrees as well
                if (auto varIt = bindingContainer.variables.find(name); varIt != bindingContainer.variables.end()) {
                    if (auto const& varInfo = varIt->second; varInfo.pointer) {
                        *varInfo.pointer = true;
                    }
                } else {
                    Nebulite::Utility::Capture::cerr() << "Warning: Unknown variable '--" << name << "'\n";
                }

                // Remove from argument list
                argv++; // Skip the first argument (function name)
                argc--; // Reduce the argument count (function name is processed)
            } else {
                // no more vars to parse
                return;
            }
        }
    }

    /**
     * @brief Finds an argument in inherited FuncTrees.
     * @param funcName Name of the function to find
     * @return Pointer to the FuncTree where the function was found, or nullptr if not found.
     */
    std::shared_ptr<FuncTree> findInInheritedTrees(std::string const& funcName) {
        // Prerequisite if an inherited FuncTree is linked
        if (!inheritedTrees.empty() && !hasFunction(funcName)) {
            // Check if the function is in an inherited tree
            for (auto& inheritedTree : inheritedTrees) {
                if (inheritedTree != nullptr && inheritedTree->hasFunction(funcName)) {
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
     * @brief Checks if there is a binding conflict with the given function name.
     * @details Prints an error message and exits if a bad conflict is found, such as overshadowing or overwriting existing functions.
     *          Returns false if a special conflict (help or __complete__ exists).
     *          Returns true if no conflicts are found.
     * @param name The name of the function to check for conflicts.
     */
    bool conflictCheck(std::string const& name);

    /**
     * @brief Binds a function directly to this FuncTree without checking for categories or conflicts.
     * @tparam ClassType The class type of the object instance
     * @param name The name of the function to bind
     * @param helpDescription Pointer to the help description for the function
     * @param method The member method to bind
     * @param obj The object instance that holds the member method
     */
    template <typename ClassType>
    void directBind(std::string const& name, std::string const* helpDescription, MemberMethod<ClassType> method, ClassType* obj);

    //------------------------------------------
    // Completion function

    /**
     * @brief Provides command completion suggestions based on the current arguments.
     * @details Prints possible completions to stdout.
     * @param args A list of arguments to complete
     * @param addArgs Additional arguments
     * @return A returnValue containing completion suggestions
     */
    returnValue complete(std::span<std::string const> const& args, additionalArgs... addArgs);

    /**
     * @brief Finds possible completions for a given pattern and prefix in the current FuncTree.
     * @param pattern The pattern to match for completions
     * @return A vector of possible completions
     */
    std::vector<std::string> findCompletions(std::string const& pattern);

    /**
     * @brief Traverses into a category based on the provided name.
     * @param categoryName Name of the category to traverse into
     * @param ftree Pointer to the current FuncTree
     * @return Pointer to the FuncTree of the category, or nullptr if not found.
     */
    FuncTree<returnValue, additionalArgs...>* traverseIntoCategory(std::string const& categoryName, FuncTree* ftree);
};
} // namespace Nebulite::Interaction::Execution

#endif // NEBULITE_INTERACTION_EXECUTION_FUNCTREE_HPP

// Template implementation
#include "Interaction/Execution/FuncTree.tpp"
