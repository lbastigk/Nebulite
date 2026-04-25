/**
 * @file FuncTree.hpp
 * @brief This file defines the FuncTree class, which is responsible for managing and executing functions
 *        through a command tree structure.
 * @details The main goal of this class is to manage hierarchical commands and arguments for modular and flexible execution.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_HPP
#define NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <utility>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Interaction/Execution/FuncTreeErrorMessages.hpp"
#include "Utility/IO/Capture.hpp"  // Due to circular dependencies, we use Capture for logging instead of Nebulite.hpp that we use as constructor argument for the FuncTree to use

//------------------------------------------
namespace Nebulite::Interaction::Execution {
/**
 * @class Nebulite::Interaction::Execution::FuncTree
 * @brief Function tree class for managing and executing functions through linguistic commands.
 * @details The FuncTree class allows for the binding of functions and variables to a command tree structure,
 *          enabling modular and flexible execution of commands based on user input.
 *          Functions are identified by their names and can have multiple arguments.
 *          Variables can be bound to the command tree and accessed within functions,
 *          provided the functions themselves have access to the space of the variables.
 */
template <typename returnValue, typename... additionalArgs>
class FuncTree {
public:
    //------------------------------------------
    // Important types

    // Command argument span types
    struct CmdArgs {
        using Span = std::span<std::string const>;
        using SpanConstRef = std::span<std::string const> const&;
    };

    // Supported function signatures
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
    using FunctionPtr = std::conditional_t<sizeof...(additionalArgs) == 0,
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

    /**
     * @brief A unique identity to any FunctionPtr, allowing for comparison of function pointers to check if the same function (from the same location in memory)
     *        is being bound again
     */
    struct FunctionIdentity {
        void const* object = nullptr; // nullptr for free/static functions
        std::array<std::byte, sizeof(void*) * 2> function{}; // stores function pointer bits

        // Equality
        bool operator==(FunctionIdentity const& other) const {
            return object == other.object &&
                   std::memcmp(function.data(), other.function.data(), function.size()) == 0;
        }

        // Check if Fn is of the correct type

        // --- Constructors ---

        // Free/static function
        template<typename Fn>
        //requires std::is_pointer_v<Fn> && std::is_function_v<std::remove_pointer_t<Fn>>
        explicit FunctionIdentity(Fn fn) {
            // Lambdas / closures are class types
            static_assert(!std::is_class_v<Fn>,
                "Lambdas and functors are not allowed. Extracting a pointer from them is not portable and may lead to collisions. Use a raw function pointer instead."
            );

            // std::function is not allowed
            static_assert(!std::is_same_v<std::remove_cv_t<std::remove_reference_t<Fn>>, std::function<void()>>,
                "std::function objects are not allowed. Use a raw function pointer instead."
            );

            // Must be a raw function pointer
            static_assert(std::is_pointer_v<Fn> && std::is_function_v<std::remove_pointer_t<Fn>>,
                "Only raw function pointers are allowed."
            );

            // Ensure the function fits in storage
            static_assert(sizeof(function) >= sizeof(fn),
                "Function too large to store in FunctionIdentity buffer. Please increase buffer size in FunctionIdentity::function."
            );
            std::memcpy(function.data(), &fn, sizeof(fn));
        }

        // Member function + object
        template<typename Obj, typename MemFn>
        explicit FunctionIdentity(Obj* obj, MemFn memFn) : object(obj) {
            // Ensure the passed obj is not a nullptr
            // As we use "this" inside the constructor of FuncTree to bind functions like "help" and "__complete__", this somehow fails...
            if (obj == nullptr) {
                throw std::invalid_argument(
                    "Object pointer cannot be nullptr for member function pointers. "
                    "Please use the free/static function constructor for improved safety."
                );
            }

            // Ensure the function fits in storage
            static_assert(sizeof(function) >= sizeof(memFn),
                "Function too large to store in FunctionIdentity buffer. Please increase buffer size in FunctionIdentity::function."
            );
            std::memcpy(function.data(), &memFn, sizeof(memFn));
        }
    };

    /**
     * @struct WrappedFunction
     * @brief Wraps a FunctionPtr with its identity for comparison purposes.
     */
    struct WrappedFunction {
        FunctionPtr function;
        FunctionIdentity identity;
    };

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
     * @param captureInstance Capture instance for logging
     */
    FuncTree(std::string_view const& treeName, returnValue const& valDefault, returnValue const& valFunctionNotFound, Utility::IO::Capture& captureInstance);

    /**
     * @brief Inherits functions from another Tree.
     * @param toInherit FuncTree pointer to inherit functions from.
     */
    void inherit(std::shared_ptr<FuncTree> const& toInherit) {
        inheritedTrees.push_back(toInherit);
    }

    //------------------------------------------
    // Parsing

    /**
     * @brief Links a function to call before parsing (e.g., for setting up variables or locking resources)
     * @param func Function to call before parsing
     */
    void setPreParse(std::function<returnValue()> func) {
        preParse = std::move(func);
    }

    /**
     * @brief Parses the command line arguments and executes the corresponding function.
     * @details All whitespaces outside of quotes are seen as argument separators.
     *          The first argument should be used to identify where the execution comes from.
     *          All subsequent arguments starting with -- are treated as variable assignments.
     *          The first argument after the variable assignments is the function to execute.
     * @param cmd Command string to parse
     * @param addArgs Additional arguments to pass to the executed function
     * @return The return value of the executed function, or the standard/error value.
     */
    returnValue parseStr(std::string_view const& cmd, additionalArgs... addArgs);
    returnValue parse(std::vector<std::string> const& args, additionalArgs... addArgs);
    returnValue parse(std::span<std::string const> const& args, additionalArgs... addArgs);

    //------------------------------------------
    // Binding (Functions, Categories, Variables)

    /**
     * @brief Binds a function to the command tree.
     * @details If the function name already exists, the program will throw an error to prevent accidental overwriting of functions.
     *          However, if the function being bound has the same pointer as the existing function, the binding will simply be ignored.
     * @param func Pointer to the function to bind, wrapped to include information about its signature.
     * @param name Name of the function in the command tree
     * @param helpDescription Help description for the function. First line is shown in the general help, full description in detailed help.
     */
    void bindFunction(WrappedFunction const& func, std::string_view const& name, std::string_view const& helpDescription);

    template <typename R, typename C, typename... Ps>
    void bindFunction(
        R (C::*functionPtr)(Ps...),
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename R, typename C, typename... Ps>
    void bindFunction(
        R (C::*functionPtr)(Ps...) const,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    template <typename Func>
    void bindFunction(
        Func functionPtr,
        std::string_view const& name,
        std::string_view const& helpDescription
    );

    /**
     * @brief Creates a category.
     * @details A category acts a "function bundler" to the main tree.
     * @param name Name of the category
     * @param helpDescription Description of the category, shown in the help command. First line is shown in the general help, full description in detailed help
     * @throws std::runtime_error if the category already exists or if category hierarchy is invalid.
     */
    void bindCategory(std::string_view const& name, std::string_view const& helpDescription);

    /**
     * @brief Binds a variable to the command tree.
     *        Make sure the variable is of type std::string*.
     *        Once bound, it can be set via command line arguments: `--varName` (Must be before the function name!)
     * @param varPtr Pointer to the variable to bind
     * @param name Name of the variable in the command tree
     * @param helpDescription Help description for the variable. First line is shown in the general help, full description in detailed help.
     */
    void bindVariable(bool* varPtr, std::string_view const& name, std::string_view const& helpDescription);


    //------------------------------------------
    // Binding checker

    /**
     * @brief Checks if a function with the given name or from a full command exists.
     * @param nameOrCommand Name of the function or full command string,
     *                      where arg[1] is the command and arg[0] is the caller
     */
    bool hasFunction(std::string_view const& nameOrCommand);

    //------------------------------------------
    // Complete

    /**
     * @brief Finds possible completions for a given pattern and prefix in the current FuncTree.
     * @param patternStr The pattern to match for completions, full command
     * @return A vector of possible completions
     */
    std::vector<std::string> findCompletionForFullCommand(std::string const& patternStr);

private:
    // Name of the tree, used for help and output
    std::string TreeName;

    // Function to call before parsing (e.g., for setting up variables or locking resources)
    std::function<returnValue()> preParse = nullptr;

    // Capture instance for logging
    Utility::IO::Capture& capture;

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
        std::string description;
    };

    /**
     * @struct FunctionInfo
     * @brief Contains information about a bound function, including its pointer and description.
     */
    struct FunctionInfo {
        WrappedFunction function; // Stores both the function pointer and its identity for comparison
        std::string description;
    };

    /**
     * @struct VariableInfo
     * @brief Contains information about a bound variable, including its pointer and description.
     */
    struct VariableInfo {
        bool* pointer = nullptr;
        std::string description;
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
     * @brief Looks up the function by name and calls it with the provided arguments.
     * @param name The name of the function to execute.
     * @param args Modern argument span.
     * @param addArgs Additional arguments to pass to the function.
     * @return The return value of the function.
     */
    returnValue executeFunction(std::string const& name, std::span<std::string const> const& args, additionalArgs... addArgs);

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

    static auto constexpr helpName = "help";
    static auto constexpr helpDesc = R"(Show available commands and their descriptions)";

    static auto constexpr completeName = "__complete__";
    static auto constexpr completeDesc = R"(Provide command completion suggestions based on the current arguments)";

    //------------------------------------------
    // Helper functions for better readability

    /**
     * @brief Displays detailed help for a specific function, category, or variable.
     */
    void specificHelp(std::string const& funcName);

    using categoryIterator = decltype(bindingContainer.categories)::iterator;
    using functionIterator = decltype(bindingContainer.functions)::iterator;
    using variableIterator = decltype(bindingContainer.variables)::iterator;
    /**
     * @using BindingSearchResult
     * @brief Helper type to store search results for finding functions, categories, and variables by name.
     */
    using BindingSearchResult = std::optional<
        std::variant<
            categoryIterator,
            functionIterator,
            variableIterator
        >
    >;

    /**
     * @brief Tries to find a function, category, or variable by name in the current FuncTree.
     *        Fully keyword required, e.g.: "category1 category2 function"
     *        Just "function" will not find the function iterator, as it's nested inside categories.
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
     * @param args The arguments to remove and process variable assignments from.
     */
    void processVariableArguments(std::span<std::string const>& args) {
        while (!args.empty()) {
            if (auto const& arg = args[0]; arg.length() >= 2 && arg.starts_with("--")) {
                // Extract name
                std::string name = arg.substr(2);

                // Set variable if attached
                // TODO: Search in inherited FuncTrees as well
                if (auto varIt = bindingContainer.variables.find(name); varIt != bindingContainer.variables.end()) {
                    if (auto const& varInfo = varIt->second; varInfo.pointer) {
                        *varInfo.pointer = true;
                    }
                } else {
                    ExecutionErrorMessage::unknownVariable(TreeName, name);
                }

                // Remove first arg
                args = args.subspan(1);
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
    // Completion

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
    FuncTree* traverseIntoCategory(std::string const& categoryName, FuncTree const* ftree);
};
} // namespace Nebulite::Interaction::Execution

// Template implementations
#include "Interaction/Execution/FuncTree.tpp"
#endif // NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_HPP
