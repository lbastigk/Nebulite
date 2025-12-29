/**
 * @file FuncTree.tpp
 * @brief Implementation file for the FuncTree class template.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP
#define NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP

//------------------------------------------
// Includes

// Standard library
#include <cxxabi.h>
#include <memory>
#include <cstdlib>
#include <typeinfo>
#include <string>

// Nebulite
#include "Utility/StringHandler.hpp"  // Using StringHandler for easy argument splitting
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
// Binding error messages

namespace bindErrorMessage {
[[noreturn]] inline void MissingCategory(std::string_view const& tree, std::string_view const& category, std::string_view const& function) {
    Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Error: Category '" << category << "' does not exist when trying to bind function '" << function << "'." << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Please create the category hierarchy first using bindCategory()." << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << Nebulite::Utility::Capture::endl;
    throw std::runtime_error("FuncTree binding failed due to missing category.");
}

[[noreturn]] inline void FunctionShadowsCategory(std::string_view const& function) {
    Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Error: Cannot bind function '" << function << "' because a category with the same name already exists." << Nebulite::Utility::Capture::endl;
    throw std::runtime_error("FuncTree binding failed due to function shadowing category.");
}

[[noreturn]] inline void FunctionExistsInInheritedTree(std::string_view const& tree, std::string_view const& inheritedTree, std::string_view const& function) {
    Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree initialization failed!\n";
    Nebulite::Utility::Capture::cerr() << "Error: A bound Function already exists in the inherited FuncTree.\n";
    Nebulite::Utility::Capture::cerr() << "Function overwrite is heavily discouraged and thus not allowed.\n";
    Nebulite::Utility::Capture::cerr() << "Please choose a different name or remove the existing function.\n";
    Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << "\n";
    Nebulite::Utility::Capture::cerr() << "inherited FuncTree:   " << inheritedTree << "\n";
    Nebulite::Utility::Capture::cerr() << "Function:  " << function << "\n";
    throw std::runtime_error("FuncTree binding failed due to function existing in inherited tree.");
}

[[noreturn]] inline void ParentCategoryDoesNotExist(std::string_view const& name, std::string_view const& currentCategoryName) {
    Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Nebulite::Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because parent category '" << currentCategoryName << "' does not exist." << Nebulite::Utility::Capture::endl;
    throw std::runtime_error("FuncTree binding failed due to missing parent category.");
}

[[noreturn]] inline void CategoryExists(std::string_view const& name) {
    Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree initialization failed!\n";
    Nebulite::Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because it already exists." << Nebulite::Utility::Capture::endl;
    throw std::runtime_error("FuncTree binding failed due to category already existing.");
}

[[noreturn]] inline void FunctionExists(std::string_view const& tree, std::string_view const& function) {
    Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Nebulite::Utility::Capture::cerr() << "Nebulite FuncTree initialization failed!\n";
    Nebulite::Utility::Capture::cerr() << "Error: A bound Function already exists in this tree.\n";
    Nebulite::Utility::Capture::cerr() << "Function overwrite is heavily discouraged and thus not allowed.\n";
    Nebulite::Utility::Capture::cerr() << "Please choose a different name or remove the existing function.\n";
    Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << "\n";
    Nebulite::Utility::Capture::cerr() << "Function:  " << function << "\n";
    throw std::runtime_error("FuncTree binding failed due to function already existing.");
}

[[noreturn]] inline void UnknownMethodPointerType(std::string_view const& tree, std::string_view const& function) {
    Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
    Nebulite::Utility::Capture::cerr() << "Nebulite FuncTree initialization failed!\n";
    Nebulite::Utility::Capture::cerr() << "Error: Unknown method pointer type for function '" << function << "' in FuncTree '" << tree << "'.\n";
    throw std::runtime_error("FuncTree binding failed due to unknown method pointer type.");
}
} // anonymous namespace

/**
 * @brief Unmangles a C++ type name.
 * @param name The mangled type name from typeid().name()
 * @return An unmangled, human-readable type name.
 */
inline std::string demangle(const char* name) {
    int status = 0;
    char* dem = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string out = status == 0 && dem ? dem : name;
    std::free(dem);
    return out;
}

//------------------------------------------
namespace Nebulite::Interaction::Execution {

//------------------------------------------
// Constructor implementation

template <typename returnType, typename... additionalArgs>
FuncTree<returnType, additionalArgs...>::FuncTree(std::string_view const& treeName, returnType const& valDefault, returnType const& valFunctionNotFound)
    : TreeName(std::move(treeName)),
      standardReturn{valDefault, valFunctionNotFound} {
    // construct the help entry in-place to avoid assignment and ambiguous lambda conversions
    bindingContainer.functions.emplace(
        helpName,
        FunctionInfo{
            std::function<returnType(std::span<std::string const> const&, additionalArgs...)>(
                [this](std::span<std::string const> const& args, additionalArgs... rest) {
                    return this->help(args, std::forward<additionalArgs>(rest)...);
                }
                ),
            helpDesc
        }
        );

    // Add __complete__ function for command completion
    bindingContainer.functions.emplace(
        "__complete__",
        FunctionInfo{
            std::function<returnType(std::span<std::string const> const&, additionalArgs...)>(
                [this](std::span<std::string const> const& args, additionalArgs... rest) {
                    return this->complete(args, std::forward<additionalArgs>(rest)...);
                }
                ),
            completeDesc
        }
        );
}

//------------------------------------------
// Template comparison

// TODO: Move to private part of FuncTree
template <typename T> bool isEqual(T const& a, T const& b) {
    if constexpr (std::is_floating_point_v<T>) {
        // Consider EPSILON for floating point comparison
        return std::fabs(a - b) < std::numeric_limits<T>::epsilon();
    } else {
        // Default comparison
        return a == b;
    }
}

//------------------------------------------
// Binding (Functions, Categories, Variables)

template <typename returnType, typename... additionalArgs>
template <typename ClassType>
void FuncTree<returnType, additionalArgs...>::bindFunction(
    ClassType* obj,
    MemberMethod<ClassType> method,
    std::string_view const& name,
    std::string_view const& helpDescription) {
    // If the name has a whitespace, the function has to be bound to a category hierarchically
    if (name.find(' ') != std::string::npos) {
        std::vector<std::string> const pathStructure = Utility::StringHandler::split(name, ' ');
        if (pathStructure.size() < 2) {
            Nebulite::Utility::Capture::cerr() << "Error: Invalid function name '" << name << "'." << Nebulite::Utility::Capture::endl;
            return;
        }
        absl::flat_hash_map<std::string, CategoryInfo>* currentCategoryMap = &bindingContainer.categories;
        FuncTree* targetTree = this;
        for (size_t idx = 0; idx < pathStructure.size() - 1; idx++) {
            std::string const& currentCategoryName = pathStructure[idx];
            if (currentCategoryMap->find(currentCategoryName) == currentCategoryMap->end()) {
                bindErrorMessage::MissingCategory(TreeName, currentCategoryName, std::string(name));
            }
            targetTree = (*currentCategoryMap)[currentCategoryName].tree.get();
            currentCategoryMap = &targetTree->bindingContainer.categories;
        }
        std::string functionName = pathStructure.back();
        targetTree->bindFunction(obj, method, functionName, helpDescription);
        return;
    }

    // Check for name conflicts, then bind directly
    if (conflictCheck(name)) {
        directBind(name, helpDescription, method, obj);
    }
}

template <typename returnType, typename... additionalArgs>
bool FuncTree<returnType, additionalArgs...>::bindCategory(std::string_view const& name, std::string_view const& helpDescription) {
    if (bindingContainer.categories.find(std::string(name)) != bindingContainer.categories.end()) {
        // Category already exists
        /**
         * @note Warning is suppressed here,
         * as with different modules we might need to call this in each module,
         * just to make sure the category exists
         */
        return false;
    }
    // Split based on whitespaces
    std::vector<std::string> const categoryStructure = Utility::StringHandler::split(name, ' ');
    size_t const depth = categoryStructure.size();

    absl::flat_hash_map<std::string, CategoryInfo>* currentCategoryMap = &bindingContainer.categories;
    for (size_t idx = 0; idx < depth; idx++) {
        std::string currentCategoryName = categoryStructure[idx];

        if (idx < depth - 1) {
            // Not yet at last category
            if (currentCategoryMap->find(currentCategoryName) != currentCategoryMap->end()) {
                // Category exists, go deeper
                currentCategoryMap = &(*currentCategoryMap)[currentCategoryName].tree->bindingContainer.categories;
            } else {
                // Category does not exist, throw error
                bindErrorMessage::ParentCategoryDoesNotExist(std::string(name), currentCategoryName);
            }
        } else {
            // Last category, create it, if it doesn't exist yet
            if (currentCategoryMap->find(currentCategoryName) != currentCategoryMap->end()) {
                // Category exists, throw error
                bindErrorMessage::CategoryExists(std::string(name));
            }
            // Create category
            (*currentCategoryMap)[currentCategoryName] = {std::make_unique<FuncTree>(currentCategoryName, standardReturn.valDefault, standardReturn.valFunctionNotFound), helpDescription};
        }
    }
    return true;
}

// Using noLint, as varPtr would be flagged as it's not const.
// But this causes issues with binding variables.
template <typename returnType, typename... additionalArgs>
// NOLINTNEXTLINE
void FuncTree<returnType, additionalArgs...>::bindVariable(bool* varPtr, std::string_view const& name, std::string_view const& helpDescription) {
    // Make sure there are no whitespaces in the variable name
    if (name.find(' ') != std::string::npos) {
        Nebulite::Utility::Capture::cerr() << "Error: Variable name '" << name << "' cannot contain whitespaces." << Nebulite::Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    // Make sure the variable isn't bound yet
    if (bindingContainer.variables.find(name) != bindingContainer.variables.end()) {
        Nebulite::Utility::Capture::cerr() << "Error: Variable '" << name << "' is already bound." << Nebulite::Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    // Bind the variable
    bindingContainer.variables.emplace(name, VariableInfo{varPtr, helpDescription});
}

//------------------------------------------
// Binding helper functions

template <typename returnType, typename... additionalArgs>
bool FuncTree<returnType, additionalArgs...>::conflictCheck(std::string_view const& name) {
    for (auto const& [categoryName, _] : bindingContainer.categories) {
        if (categoryName == name) {
            bindErrorMessage::FunctionShadowsCategory(name);
        }
    }

    if (name == "help" || name == "__complete__") {
        if (bindingContainer.functions.find(name) != bindingContainer.functions.end()) {
            // Special function already exists
            return false;
        }
    }

    auto conflictIt = std::find_if(
        inheritedTrees.begin(), inheritedTrees.end(),
        [&](auto const& inheritedTree) {
            return inheritedTree && inheritedTree->hasFunction(name);
        }
        );
    if (conflictIt != inheritedTrees.end()) {
        auto conflictTree = *conflictIt;
        bindErrorMessage::FunctionExistsInInheritedTree(TreeName, conflictTree->TreeName, name);
        return false;
    }
    if (hasFunction(name)) {
        bindErrorMessage::FunctionExists(TreeName, name);
        return false;
    }
    return true;
}

template <typename returnType, typename... additionalArgs>
template <typename ClassType>
void FuncTree<returnType, additionalArgs...>::directBind(std::string_view const& name, std::string_view const& helpDescription, MemberMethod<ClassType> method, ClassType* obj) {
    // Use std::visit to bind the correct function type
    std::visit([&]<typename MethodPointer>(MethodPointer&& methodPointer) {
        using MethodType = std::decay_t<MethodPointer>;

        // See if the method pointer is a modernized function
        bool constexpr isModern = std::is_same_v<MethodType, returnType (ClassType::*)(SpanArgs, additionalArgs...)>
                                  || std::is_same_v<MethodType, returnType (ClassType::*)(SpanArgs, additionalArgs...) const>;
        bool constexpr isModernRefArgs = std::is_same_v<MethodType, returnType (ClassType::*)(SpanArgsConstRef, additionalArgs...)>
                                         || std::is_same_v<MethodType, returnType (ClassType::*)(SpanArgsConstRef, additionalArgs...) const>;

        // Legacy Bindings, not supporting additionalArgs at the moment
        if constexpr (std::is_same_v<MethodType, returnType (ClassType::*)(int, char**)>) {
            bindingContainer.functions.emplace(
                name,
                FunctionInfo{
               std::function<returnType(int, char**)>(
                   [obj, methodPointer](int argc, char** argv) {
                       return (obj->*methodPointer)(argc, argv);
                   }
                   ),
               helpDescription
           });
        } else if constexpr (std::is_same_v<MethodType, returnType (ClassType::*)(int, char const**)>) {
            bindingContainer.functions.emplace(
                name,
                FunctionInfo{
               std::function<returnType(int, char const**)>(
                   [obj, methodPointer](int argc, char const** argv) {
                       return (obj->*methodPointer)(argc, argv);
                   }
                   ),
               helpDescription
           });
        }

        // Modern Bindings, allow additionalArgs...
        else if constexpr (isModern || isModernRefArgs) {
            bindingContainer.functions.emplace(
                name,
                FunctionInfo{
               std::function<returnType(std::span<std::string const>, additionalArgs...)>(
                   [obj, methodPointer](std::span<std::string const> args, additionalArgs... rest) {
                       return (obj->*methodPointer)(args, std::forward<additionalArgs>(rest)...);
                   }
                   ),
               helpDescription
           });
        }
        // 5.) Unsupported method pointer type
        else {
            bindErrorMessage::UnknownMethodPointerType(TreeName, name);
        }
    }, method);
}

//------------------------------------------
// Getter

template <typename returnType, typename... additionalArgs>
std::vector<std::pair<std::string, std::string_view>> FuncTree<returnType, additionalArgs...>::getAllFunctions() {
    std::vector<std::pair<std::string, std::string_view>> allFunctions;
    for (auto const& [name, info] : bindingContainer.functions) {
        allFunctions.emplace_back(name, info.description);
    }

    // Get functions from inherited FuncTrees
    for (auto& inheritedTree : inheritedTrees) {
        for (auto const& [name, description] : inheritedTree->getAllFunctions()) {
            if (bindingContainer.functions.find(name) == bindingContainer.functions.end()) {
                allFunctions.emplace_back(name, description);
            }
        }
    }

    // Get just the names of the bindingContainer.categories
    for (auto const& [categoryName, cat] : bindingContainer.categories) {
        allFunctions.emplace_back(categoryName, cat.description);
    }

    return allFunctions;
}

template <typename returnType, typename... additionalArgs>
std::vector<std::pair<std::string, std::string_view>> FuncTree<returnType, additionalArgs...>::getAllVariables() {
    std::vector<std::pair<std::string, std::string_view>> allVariables;
    for (auto const& [name, info] : bindingContainer.variables) {
        allVariables.emplace_back(name, info.description);
    }

    // Get from inherited FuncTree
    for (auto& inheritedTree : inheritedTrees) {
        // Case by case, making sure we do not have duplicates
        for (auto const& [name, description] : inheritedTree->getAllVariables()) {
            if (bindingContainer.variables.find(name) == bindingContainer.variables.end()) {
                allVariables.emplace_back(name, description);
            }
        }
    }

    return allVariables;
}

//------------------------------------------
// Parsing and execution

template <typename returnType, typename... additionalArgs>
returnType FuncTree<returnType, additionalArgs...>::parseStr(std::string const& cmd, additionalArgs... addArgs) {
    // Quote-aware tokenization
    std::vector<std::string> tokens = Utility::StringHandler::parseQuotedArguments(cmd);

    ////////////////////////////////////////////
    //------------------------------------------
    // TODO: parsing variables etc should be done with tokens or with the span
    //       then we only convert to argc/argv if we need to inside executeFunction

    // Convert to argc/argv
    size_t argc = tokens.size();
    std::vector<char*> argv_vec;
    argv_vec.reserve(argc + 1);
    std::transform(tokens.begin(), tokens.end(), std::back_inserter(argv_vec),
                   [](std::string const& str) { return const_cast<char*>(str.c_str()); });
    argv_vec.push_back(nullptr); // Null-terminate

    // First argument is binary name or last function name
    // remove it from the argument list
    char** argv = argv_vec.data();
    argv++;
    argc--;

    // Process arguments directly after binary/function name (like --count or -c)
    processVariableArguments(argc, argv);

    // Check if there are still arguments left
    if (argc == 0) {
        return standardReturn.valDefault; // Nothing to execute, return standard
    }

    // turn argc/argv into span
    std::vector<std::string> argsVec;
    argsVec.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        argsVec.emplace_back(argv[i]);
    }
    auto tokensSpan = std::span<std::string const>(argsVec.data(), argsVec.size());

    ////////////////////////////////////////////

    // The first argument left is the new function name
    std::string funcName = tokensSpan.front();

    // Check in inherited FuncTrees first
    auto inheritedTree = findInInheritedTrees(funcName);
    if (inheritedTree != nullptr) {
        // Function is in inherited tree, parse there
        return inheritedTree->executeFunction(funcName, static_cast<int>(argc), argv, tokensSpan, addArgs...);
    }

    // Not found in inherited trees, execute the function the main tree
    return executeFunction(funcName, static_cast<int>(argc), argv, tokensSpan, addArgs...);
}

template <typename returnType, typename... additionalArgs>
returnType FuncTree<returnType, additionalArgs...>::executeFunction(std::string const& name, int argc, char** argv, std::span<std::string const> const& args, additionalArgs... addArgs) {
    // Call preParse function if set
    if (preParse != nullptr) {
        if (returnType err = preParse(); !isEqual(err, standardReturn.valDefault)) {
            return err; // Return error if preParse failed
        }
    }

    // Strip whitespaces of name
    std::string function = name;
    function = Utility::StringHandler::lStrip(function, ' ');
    function = Utility::StringHandler::rStrip(function, ' ');

    // Find and execute the function
    auto functionPosition = bindingContainer.functions.find(function);
    if (functionPosition != bindingContainer.functions.end()) {
        auto& [functionPtr, description] = functionPosition->second;
        return std::visit([&]<typename Func>(Func&& func) {
            using T = std::decay_t<Func>;

            // Legacy function types
            if constexpr (std::is_same_v<T, std::function<returnType(int, char**)>>) {
                // Convert to argc/argv
                return func(argc, argv);
            } else if constexpr (std::is_same_v<T, std::function<returnType(int, char const**)>>) {
                // Convert char** to char const**
                std::vector<char const*> argv_const(static_cast<size_t>(argc));
                for (size_t i = 0; i < static_cast<size_t>(argc); ++i)
                    argv_const[i] = argv[i];
                return func(argc, argv_const.data());
            }
            // Modern function types
            else if constexpr (std::is_same_v<T, SpanFn> || std::is_same_v<T, SpanFnConstRef>) {
                return func(args, addArgs...);
            }
            // Unknown function type
            else {
                Nebulite::Utility::Capture::cerr() << "Error: Unknown function signature for function '" << function << "' in FuncTree '" << TreeName << "'." << Nebulite::Utility::Capture::endl;
                Nebulite::Utility::Capture::cerr() << "Visitor matched type (mangled):   " << typeid(T).name() << Nebulite::Utility::Capture::endl;
                Nebulite::Utility::Capture::cerr() << "Visitor matched type (demangled): " << demangle(typeid(T).name()) << Nebulite::Utility::Capture::endl;
                std::exit(EXIT_FAILURE);
            }
        }, functionPtr);
    }
    // Find function name in bindingContainer.categories
    if (bindingContainer.categories.find(function) != bindingContainer.categories.end()) {
        std::string cmd;
        for (int i = 0; i < argc; i++) {
            cmd += std::string(argv[i]) + " ";
        }
        return bindingContainer.categories[function].tree->parseStr(cmd, addArgs...);
    }
    Nebulite::Utility::Capture::cerr() << "Function '" << function << "' not found in FuncTree " << TreeName << ", its inherited FuncTrees or their categories!\n";
    Nebulite::Utility::Capture::cerr() << "Arguments are:" << Nebulite::Utility::Capture::endl;
    for (int i = 0; i < argc; i++) {
        Nebulite::Utility::Capture::cerr() << "argv[" << i << "] = '" << argv[i] << "'\n";
    }
    Nebulite::Utility::Capture::cerr() << "Available functions:  " << bindingContainer.functions.size() << Nebulite::Utility::Capture::endl;
    Nebulite::Utility::Capture::cerr() << "Available categories: " << bindingContainer.categories.size() << Nebulite::Utility::Capture::endl;
    return standardReturn.valFunctionNotFound; // Return error if function not found
}

template <typename returnType, typename... additionalArgs>
bool FuncTree<returnType, additionalArgs...>::hasFunction(std::string_view const& nameOrCommand) {
    // Make sure only the command name is used
    std::vector<std::string> tokens = Utility::StringHandler::split(nameOrCommand, ' ');

    // Remove all tokens starting with "--"
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
                                [](std::string const& token) {
                                    return token.starts_with("--");
                                }), tokens.end());

    // Remove all empty tokens
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
                                [](std::string const& token) {
                                    return token.empty();
                                }), tokens.end());

    if (tokens.empty()) {
        return false; // No command provided
    }

    // Depending on token count, function name is at different positions
    std::string function;
    if (tokens.size() == 1) {
        // Case 1:
        // Is a single function name.
        // e.g.: "set"
        function = tokens[0];
    } else {
        // Case 2:
        // Is a full command
        // e.g.: <whereCommandComesFrom> set key value
        function = tokens[1];
    }

    // See if the function is linked
    return bindingContainer.functions.find(function) != bindingContainer.functions.end() ||
           bindingContainer.categories.find(function) != bindingContainer.categories.end();
}

//------------------------------------------
// Help function and its helpers

namespace SortFunctions {
// Case-insensitive comparison function
inline auto caseInsensitiveLess = [](auto const& a, auto const& b) {
    return std::ranges::lexicographical_compare(
        a.first, b.first,
        [](char const lhs, char const rhs) {
            return std::tolower(static_cast<unsigned char>(lhs)) <
                   std::tolower(static_cast<unsigned char>(rhs));
        }
        );
};
} // namespace SortFunctions

template <typename returnType, typename... additionalArgs>
returnType FuncTree<returnType, additionalArgs...>::help(std::span<std::string const> const& args, additionalArgs... addArgs) {
    //------------------------------------------
    // Case 1: Detailed help for a specific function, category or variable
    if (args.size() > 1) {
        // Call specific help for each argument, except the first one (which is the binary name or last function name)
        for (auto const& arg : args.subspan(1)) {
            specificHelp(arg, addArgs...);
        }
        return standardReturn.valDefault;
    }

    //------------------------------------------
    // Case 2: General help for all functions, bindingContainer.categories and variables
    generalHelp();
    return standardReturn.valDefault;
}

template <typename returnType, typename... additionalArgs>
void FuncTree<returnType, additionalArgs...>::specificHelp(std::string const& funcName, additionalArgs... addArgs) {
    if (BindingSearchResult const searchResult = find(funcName); searchResult.any) {
        // 1.) Function
        if (searchResult.function) {
            // Found function, display detailed help
            Nebulite::Utility::Capture::cout() << "\nHelp for function '" << funcName << "':\n" << Nebulite::Utility::Capture::endl;
            Nebulite::Utility::Capture::cout() << searchResult.funIt->second.description << "\n";
        }
        // 2.) Category
        else if (searchResult.category) {
            // Found category, display detailed help
            searchResult.catIt->second.tree->help({}, addArgs...); // Display all functions in the category
        }
        // 3.) Variable
        else if (searchResult.variable) {
            // Found variable, display detailed help
            Nebulite::Utility::Capture::cout() << "\nHelp for variable '--" << funcName << "':\n" << Nebulite::Utility::Capture::endl;
            Nebulite::Utility::Capture::cout() << searchResult.varIt->second.description << "\n";
        }
    } else {
        Nebulite::Utility::Capture::cerr() << "Function or Category '" << funcName << "' not found in FuncTree '" << TreeName << "'.\n";
    }
}

template <typename returnType, typename... additionalArgs>
void FuncTree<returnType, additionalArgs...>::generalHelp() {
    // Padding size for names
    // '<name padded> - <description>'
    uint16_t constexpr namePaddingSize = 25;

    // Define a lambda to process each member
    auto displayMember = [](std::string const& name, std::string_view const& description) -> void {
        // Only show the first line of the description
        std::string descriptionFirstLine = std::string(description);
        if (size_t const newlinePos = description.find('\n'); newlinePos != std::string::npos) {
            descriptionFirstLine = description.substr(0, newlinePos);
        }
        std::string paddedName = name;
        paddedName.resize(namePaddingSize, ' ');
        Nebulite::Utility::Capture::cout() << "  " << paddedName << " - " << descriptionFirstLine << Nebulite::Utility::Capture::endl;
    };

    // All info: [name, description]
    auto allFunctions = getAllFunctions(); // includes categories
    auto allVariables = getAllVariables();

    // Sort by name
    std::ranges::sort(allFunctions, SortFunctions::caseInsensitiveLess);
    std::ranges::sort(allVariables, SortFunctions::caseInsensitiveLess);

    // Display:
    Nebulite::Utility::Capture::cout() << "\nHelp for " << TreeName << "\nAdd the entries name to the command for more details: " << TreeName << " help <foo>\n";
    Nebulite::Utility::Capture::cout() << "Available functions:\n";

    // Use lambda with for_each on all functions and variables
    // TODO: using structured bindings here would be nice, but that won't compile for some reason
    std::ranges::for_each(allFunctions, [&](auto const& pair) {
        displayMember(pair.first, pair.second);
    });
    Nebulite::Utility::Capture::cout() << "Available variables:\n";
    std::ranges::for_each(allVariables, [&](auto const& pair) {
        displayMember(pair.first, pair.second);
    });
}

template <typename returnType, typename... additionalArgs>
FuncTree<returnType, additionalArgs...>::BindingSearchResult
FuncTree<returnType, additionalArgs...>::find(std::string const& name) {
    BindingSearchResult result;

    // Helper lambda to search in inherited trees
    auto searchInInherited = [&](auto mapMember, auto& iteratorMember, bool& foundFlag) {
        for (auto const& inheritedTree : inheritedTrees) {
            if (inheritedTree) {
                iteratorMember = (inheritedTree->bindingContainer.*mapMember).find(name);
                if (iteratorMember != (inheritedTree->bindingContainer.*mapMember).end()) {
                    foundFlag = true;
                    return;
                }
            }
        }
    };

    // --- Categories ---
    result.catIt = bindingContainer.categories.find(name);
    if (result.catIt != bindingContainer.categories.end()) {
        result.category = true;
    } else {
        searchInInherited(&BindingContainer::categories, result.catIt, result.category);
    }

    // --- Functions ---
    result.funIt = bindingContainer.functions.find(name);
    if (result.funIt != bindingContainer.functions.end()) {
        result.function = true;
    } else {
        searchInInherited(&BindingContainer::functions, result.funIt, result.function);
    }

    // --- Variables ---
    result.varIt = bindingContainer.variables.find(name);
    if (result.varIt != bindingContainer.variables.end()) {
        result.variable = true;
    } else {
        searchInInherited(&BindingContainer::variables, result.varIt, result.variable);
    }

    result.any = result.category || result.function || result.variable;
    return result;
}

template <typename returnValue, typename ... additionalArgs>
returnValue FuncTree<returnValue, additionalArgs...>::complete(std::span<std::string const> const& args, additionalArgs... addArgs){
    auto argsSpan = args.subspan(1); // Skip binary name or last function name
    FuncTree<returnValue, additionalArgs...>* ftree = this;
    while (argsSpan.size() > 1) {
        // Traverse functree categories
        std::string const& categoryName = argsSpan.front();
        ftree = traverseIntoCategory(categoryName, ftree);
        if (ftree != nullptr) {
            argsSpan = argsSpan.subspan(1); // Remove processed category
            continue; // Successfully traversed into category
        } else {
            break;
        }

    }

    // Return if traversal failed
    if (ftree == nullptr) {
        return standardReturn.valDefault;
    }

    // Now we check the ftree for completions
    std::string pattern;
    if (argsSpan.empty()) {
        // No pattern provided
        pattern = "";
    }
    else {
        pattern = argsSpan.front();
    }

    auto completions = ftree->findCompletions(pattern);

    // If there is only one completion, it might be a category, so we traverse into it
    bool lastWordIsLikelyCategory = completions.size() == 1 && completions.front() == pattern;
    if (lastWordIsLikelyCategory) {
        if (traverseIntoCategory(pattern, ftree)) {
            completions = ftree->findCompletions("");
        }
        else{
            completions.clear(); // No completions found
        }
    }

    // Sort and remove duplicates, filter out __complete__ from completions
    std::ranges::sort(completions);
    completions.erase(std::unique(completions.begin(), completions.end()), completions.end());
    completions.erase(std::remove(completions.begin(), completions.end(), "__complete__"), completions.end());

    // Output completions
    for (auto const& completion : completions) {
        Nebulite::Utility::Capture::cout() << completion << Nebulite::Utility::Capture::endl;
    }
    return standardReturn.valDefault;
}

template <typename returnValue, typename ... additionalArgs>
FuncTree<returnValue, additionalArgs...>* FuncTree<returnValue, additionalArgs...>::traverseIntoCategory(std::string const& categoryName, FuncTree<returnValue, additionalArgs...>* ftree) {
    bool foundDirect = false;
    bool foundInherited = false;

    // Check direct categories first
    if (auto catIt = ftree->bindingContainer.categories.find(categoryName); catIt != ftree->bindingContainer.categories.end()) {
        foundDirect = true;
        ftree = catIt->second.tree.get();
    } else {
        // Category not found, check inherited trees
        for (auto const& inheritedTree : ftree->inheritedTrees) {
            if (inheritedTree) {
                auto inheritedCatIt = inheritedTree->bindingContainer.categories.find(categoryName);
                if (inheritedCatIt != inheritedTree->bindingContainer.categories.end()) {
                    foundInherited = true;
                    ftree = inheritedCatIt->second.tree.get();
                    break; // Stop searching after first found
                }
            }
        }
    }

    // Break if category not found
    if (!foundDirect && !foundInherited) {
        return nullptr;
    }
    return ftree;
}

template <typename returnValue, typename ... additionalArgs>
std::vector<std::string> FuncTree<returnValue, additionalArgs...>::findCompletions(std::string const& pattern) {
    std::vector<std::string> completions;
    for (auto const& [name, _] : bindingContainer.functions) {
        if (name.starts_with(pattern)) {
            // Found a completion, store it
            completions.push_back(name);
        }
    }
    for (auto const& [name, _] : bindingContainer.categories) {
        if (name.starts_with(pattern)) {
            // Found a completion, store it
            completions.push_back(name);
        }
    }
    for (auto const& [name, _] : bindingContainer.variables) {
        std::string const fullVarName = "--" + name;
        if (fullVarName.starts_with(pattern)) {
            // Found a completion, store it
            completions.push_back(fullVarName);
        }
    }

    // Check in inherited trees
    for (auto const& inheritedTree : inheritedTrees) {
        if (inheritedTree) {
            auto inheritedCompletions = inheritedTree->findCompletions(pattern);
            completions.insert(completions.end(), inheritedCompletions.begin(), inheritedCompletions.end());
        }
    }
    return completions;
}


} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP
