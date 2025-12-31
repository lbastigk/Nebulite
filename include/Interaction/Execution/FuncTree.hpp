/**
 * @file FuncTree.hpp
 * @brief This file defines the FuncTree class, which is responsible for managing and executing functions
 *        through a command tree structure.
 *        The main goal of this class is to manage hierarchical commands and arguments for modular and flexible execution.
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

    struct CmdArgs {
        using Span = std::span<std::string const>;
        using SpanConstRef = std::span<std::string const> const&;
    };
    
    struct SupportedFunctions {
        struct Legacy {
            using IntChar = std::function<returnValue(int, char**)>;
            using IntConstChar = std::function<returnValue(int, char const**)>;
        };

        struct Modern {
            using Full = std::function<returnValue(typename CmdArgs::Span, additionalArgs...)>;
            using FullConstRef = std::function<returnValue(typename CmdArgs::SpanConstRef, additionalArgs...)>;
            using NoAddArgs = std::function<returnValue(typename CmdArgs::Span)>;
            using NoAddArgsConstRef = std::function<returnValue(typename CmdArgs::SpanConstRef)>;
            using NoCmdArgs = std::function<returnValue(additionalArgs...)>;
            using NoArgs = std::function<returnValue()>;
        };
    };

    // Function pointer type
    using FunctionPtr = std::conditional_t<
        (sizeof...(additionalArgs) == 0),
        // no additional args -> avoid duplicates (keep only no-add variants)
        std::variant<
            typename SupportedFunctions::Legacy::IntChar,
            typename SupportedFunctions::Legacy::IntConstChar,
            typename SupportedFunctions::Modern::NoAddArgs,
            typename SupportedFunctions::Modern::NoAddArgsConstRef,
            typename SupportedFunctions::Modern::NoArgs
        >,
        // with additional args -> include full set
        std::variant<
            typename SupportedFunctions::Legacy::IntChar,
            typename SupportedFunctions::Legacy::IntConstChar,
            typename SupportedFunctions::Modern::Full,
            typename SupportedFunctions::Modern::FullConstRef,
            typename SupportedFunctions::Modern::NoAddArgs,
            typename SupportedFunctions::Modern::NoAddArgsConstRef,
            typename SupportedFunctions::Modern::NoCmdArgs
        >
    >;

    //------------------------------------------

    /**
     * @brief Creates a FunctionPtr variant from a user-provided callable (free function pointer,
     *        std::function, lambda, etc.). Throws/aborts on unknown signatures.
     */
    template <typename Func>
    static FunctionPtr makeFunctionPtr(Func functionPtr);

    /**
     * @brief Creates a FunctionPtr variant by binding a member-function pointer to a concrete object.
     *        Usage: FuncTree::makeFunctionPtr(objPtr, &Obj::member)
     */
    template <typename Obj, typename MemFunc>
    static FunctionPtr makeFunctionPtr(Obj* objectPtr, MemFunc memberFunctionPtr);

    //------------------------------------------
    // Constructor and inheritance

    /**
     * @brief Constructor for the FuncTree class.
     * @param treeName Name of the tree
     * @param valDefault Value to return if everything is okay
     * @param valFunctionNotFound Value to return if the parsed function was not found
     */
    FuncTree(std::string_view const& treeName, returnValue const& valDefault, returnValue const& valFunctionNotFound);

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
     * @param cmd Command string to parse
     * @param addArgs Additional arguments to pass to the executed function
     * @return The return value of the executed function, or the standard/error value.
     * @todo Add support for string_view as well as vector<string_view> as command input
     */
    returnValue parseStr(std::string const& cmd, additionalArgs... addArgs);

    //------------------------------------------
    // Binding (Functions, Categories, Variables)

    /**
     * @brief Creates a category.
     *        A category acts a "function bundler" to the main tree.
     * @param name Name of the category
     * @param helpDescription Description of the category, shown in the help command. First line is shown in the general help, full description in detailed help
     * @return true if the category was created successfully, 
     * false if a category with the same name already exists.
     */
    bool bindCategory(std::string_view const& name, std::string_view const& helpDescription);

    /**
     * @brief Binds a function to the command tree.
     *        Make sure the function has the signature:
     *        ```cpp
     *        returnValue functionName(int argc, char** argv);
     *        ```
     * @param func Pointer to the function to bind
     * @param name Name of the function in the command tree
     * @param helpDescription Help description for the function. First line is shown in the general help, full description in detailed help.
     */
    void bindFunction(FunctionPtr const& func, std::string_view const& name, std::string_view const& helpDescription);

    /**
     * @brief Binds a variable to the command tree.
     *        Make sure the variable is of type std::string*.
     *        Once bound, it can be set via command line arguments: `--varName` (Must be before the function name!)
     * @param varPtr Pointer to the variable to bind
     * @param name Name of the variable in the command tree
     * @param helpDescription Help description for the variable. First line is shown in the general help, full description in detailed help.
     */
    void bindVariable(bool* varPtr, std::string_view const& name, std::string_view const& helpDescription);

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
        std::string_view description;
    };

    /**
     * @struct FunctionInfo
     * @brief Contains information about a bound function, including its pointer and description.
     */
    struct FunctionInfo {
        FunctionPtr function;
        std::string_view description;
    };

    /**
     * @struct VariableInfo
     * @brief Contains information about a bound variable, including its pointer and description.
     */
    struct VariableInfo {
        bool* pointer = nullptr;
        std::string_view description;
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
    bool hasFunction(std::string_view const& nameOrCommand);

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
     * @return The standard return value.
     */
    returnValue help(std::span<std::string const> const& args);

    /**
     * @brief Retrieves a list of all functions and their descriptions.
     * @return A vector of pairs containing function names and their descriptions.
     */
    std::vector<std::pair<std::string, std::string_view>> getAllFunctions();

    /**
     * @brief Retrieves a list of all variables and their descriptions.
     * @return A vector of pairs containing variable names and their descriptions.
     */
    std::vector<std::pair<std::string, std::string_view>> getAllVariables();

    //------------------------------------------
    // Descriptions for built-in functions

    static std::string_view constexpr helpName = "help";
    static std::string_view constexpr helpDesc = R"(Show available commands and their descriptions)";

    static std::string_view constexpr completeName = "__complete__";
    static std::string_view constexpr completeDesc = R"(Provide command completion suggestions based on the current arguments)";

    //------------------------------------------
    // Helper functions for better readability

    /**
     * @brief Displays detailed help for a specific function, category, or variable.
     */
    void specificHelp(std::string const& funcName);

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
    bool conflictCheck(std::string_view const& name);

    //------------------------------------------
    // Completion function

    /**
     * @brief Provides command completion suggestions based on the current arguments.
     * @details Prints possible completions to stdout.
     * @param args A list of arguments to complete
     * @return The standard return value.
     */
    returnValue complete(std::span<std::string const> const& args);

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

/*
enum class FunctionShape {
        Unknown,

        // Member shapes
        Member_Legacy_IntChar,
        Member_Legacy_IntConstChar,

        Member_Modern_NoAddArgs,
        Member_Modern_NoAddArgsConstRef,

        Member_Modern_Full,
        Member_Modern_FullConstRef,

        Member_NoCmdArgs,
        Member_NoArgs,

        // Free / static shapes
        Free_Legacy_IntChar,
        Free_Legacy_IntConstChar,

        Free_Modern_NoAddArgs,
        Free_Modern_NoAddArgsConstRef,

        Free_Modern_Full,
        Free_Modern_FullConstRef,

        Free_NoArgs,
        Free_NoCmdArgs
    };
*/
