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

namespace bindErrorMessage{
    inline void MissingCategory(std::string const& tree, std::string const& category, std::string const& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "Error: Category '" << category << "' does not exist when trying to bind function '" << function << "'." << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "Please create the category hierarchy first using bindCategory()." << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << Nebulite::Utility::Capture::endl;
        throw std::runtime_error("FuncTree binding failed due to missing category.");
    }

    inline void FunctionShadowsCategory(std::string const& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "Error: Cannot bind function '" << function << "' because a category with the same name already exists." << Nebulite::Utility::Capture::endl;
        throw std::runtime_error("FuncTree binding failed due to function shadowing category.");
    }

    inline void FunctionExistsInInheritedTree(std::string const& tree, std::string const& inheritedTree, std::string const& function){
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

    inline void ParentCategoryDoesNotExists(std::string const& name, std::string const& currentCategoryName) {
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
        Nebulite::Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because parent category '" << currentCategoryName << "' does not exist." << Nebulite::Utility::Capture::endl;
        throw std::runtime_error("FuncTree binding failed due to missing parent category.");
    }

    inline void CategoryExists(std::string const& name) {
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
        Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree initialization failed!\n";
        Nebulite::Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because it already exists." << Nebulite::Utility::Capture::endl;
        throw std::runtime_error("FuncTree binding failed due to category already existing.");
    }

    inline void FunctionExists(std::string const& tree, std::string const& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
        Nebulite::Utility::Capture::cerr() << "Nebulite FuncTree initialization failed!\n";
        Nebulite::Utility::Capture::cerr() << "Error: A bound Function already exists in this tree.\n";
        Nebulite::Utility::Capture::cerr() << "Function overwrite is heavily discouraged and thus not allowed.\n";
        Nebulite::Utility::Capture::cerr() << "Please choose a different name or remove the existing function.\n";
        Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << "\n";
        Nebulite::Utility::Capture::cerr() << "Function:  " << function << "\n";
        throw std::runtime_error("FuncTree binding failed due to function already existing.");
    }

    inline void UnknownMethodPointerType(std::string const& tree, std::string const& function){
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
namespace Nebulite::Interaction::Execution{

//------------------------------------------
// Constructor implementation

template<typename RETURN_TYPE, typename... additionalArgs>
FuncTree<RETURN_TYPE, additionalArgs...>::FuncTree(std::string treeName, RETURN_TYPE valDefault, RETURN_TYPE valFunctionNotFound)
: TreeName(std::move(treeName)),
  standardReturn{valDefault, valFunctionNotFound}
{
    // construct the help entry in-place to avoid assignment and ambiguous lambda conversions
    bindingContainer.functions.emplace(
        "help",
        FunctionInfo{
            std::function<RETURN_TYPE(std::span<std::string const> const&)>(
                [this](std::span<std::string const> const& args){
                    return this->help(args);
                }
            ),
            &help_desc
        }
    );
}

//------------------------------------------
// Binding (Functions, Categories, Variables)

template<typename RETURN_TYPE, typename... additionalArgs>
template<typename ClassType>
void FuncTree<RETURN_TYPE, additionalArgs...>::bindFunction(
    ClassType* obj,
    MemberMethod<ClassType> method,
    std::string const& name,
    std::string const* helpDescription)
{
    // If the name has a whitespace, the function has to be bound to a category hierarchically
    if(name.find(' ') != std::string::npos){
        std::vector<std::string> const pathStructure = Utility::StringHandler::split(name, ' ');
        if(pathStructure.size() < 2){
            Utility::Capture::cerr() << "Error: Invalid function name '" << name << "'." << Utility::Capture::endl;
            return;
        }
        absl::flat_hash_map<std::string, CategoryInfo>* currentCategoryMap = &bindingContainer.categories;
        FuncTree* targetTree = this;
        for(size_t idx = 0; idx < pathStructure.size() - 1; idx++){
            std::string const& currentCategoryName = pathStructure[idx];
            if(currentCategoryMap->find(currentCategoryName) == currentCategoryMap->end()){
                bindErrorMessage::MissingCategory(TreeName, currentCategoryName, name);
            }
            targetTree = (*currentCategoryMap)[currentCategoryName].tree.get();
            currentCategoryMap = &targetTree->bindingContainer.categories;
        }
        std::string functionName = pathStructure.back();
        targetTree->bindFunction(obj, method, functionName, helpDescription);
        return;
    }

    // Check for name conflicts
    conflictCheck(name);

    // Bind to this tree directly
    directBind(name, helpDescription, method, obj);
}

template<typename RETURN_TYPE, typename... additionalArgs>
bool FuncTree<RETURN_TYPE, additionalArgs...>::bindCategory(std::string const& name, std::string const* helpDescription){
    if(bindingContainer.categories.find(name) != bindingContainer.categories.end()){
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
    for(size_t idx = 0; idx < depth; idx++){
        std::string currentCategoryName = categoryStructure[idx];

        if(idx < depth -1){
            // Not yet at last category
            if(currentCategoryMap->find(currentCategoryName) != currentCategoryMap->end()){
                // Category exists, go deeper
                currentCategoryMap = &(*currentCategoryMap)[currentCategoryName].tree->bindingContainer.categories;
            }
            else{
                // Category does not exist, throw error
                bindErrorMessage::ParentCategoryDoesNotExists(name, currentCategoryName);
            }
        }
        else{
            // Last category, create it, if it doesn't exist yet
            if(currentCategoryMap->find(currentCategoryName) != currentCategoryMap->end()){
                // Category exists, throw error
                bindErrorMessage::CategoryExists(name);
            }
            // Create category
            (*currentCategoryMap)[currentCategoryName] = {std::make_unique<FuncTree>(currentCategoryName,   standardReturn.valDefault,  standardReturn.valFunctionNotFound), helpDescription};
        }
    }
    return true;
}

// Using noLint, as varPtr would be flagged as it's not const.
// But this causes issues with binding variables.
template<typename RETURN_TYPE, typename... additionalArgs>
// NOLINTNEXTLINE
void FuncTree<RETURN_TYPE, additionalArgs...>::bindVariable(bool* varPtr, std::string const& name, std::string const* helpDescription){
    // Make sure there are no whitespaces in the variable name
    if (name.find(' ') != std::string::npos){
        Utility::Capture::cerr() << "Error: Variable name '" << name << "' cannot contain whitespaces." << Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    // Make sure the variable isn't bound yet
    if (bindingContainer.variables.find(name) != bindingContainer.variables.end()){
        Utility::Capture::cerr() << "Error: Variable '" << name << "' is already bound." << Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    // Bind the variable
    bindingContainer.variables.emplace(name, VariableInfo{varPtr, helpDescription});
}

//------------------------------------------
// Binding helper functions

template<typename RETURN_TYPE, typename... additionalArgs>
void FuncTree<RETURN_TYPE, additionalArgs...>::conflictCheck(std::string const &name) {
    for (auto const& [categoryName, _] : bindingContainer.categories){
        if (categoryName == name){
            bindErrorMessage::FunctionShadowsCategory(name);
        }
    }
    auto conflictIt = std::find_if(
        inheritedTrees.begin(), inheritedTrees.end(),
        [&](auto const& inheritedTree){
            return inheritedTree && name != "help" && inheritedTree->hasFunction(name);
        }
    );
    if (conflictIt != inheritedTrees.end()){
        auto conflictTree = *conflictIt;
        bindErrorMessage::FunctionExistsInInheritedTree(TreeName, conflictTree->TreeName, name);
    }
    if (hasFunction(name)){
        bindErrorMessage::FunctionExists(TreeName, name);
    }
}

template<typename RETURN_TYPE, typename... additionalArgs>
template<typename ClassType>
void FuncTree<RETURN_TYPE, additionalArgs...>::directBind(std::string const& name, std::string const* helpDescription, MemberMethod<ClassType> method, ClassType* obj){
    // Use std::visit to bind the correct function type
    std::visit([&]<typename MethodPointer>(MethodPointer&& methodPointer){
        using MethodType = std::decay_t<MethodPointer>;

        // See if the method pointer is a modernized function
        bool constexpr isModern =  std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(SpanArgs, additionalArgs...)>
                                || std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(SpanArgs, additionalArgs...) const>
                                || std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(SpanArgs, additionalArgs...) const&>;

        // Legacy Bindings
        if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(int, char**)>){
            bindingContainer.functions.emplace(name, FunctionInfo{
                std::function<RETURN_TYPE(int, char**)>(
                    [obj, methodPointer](int argc, char** argv){
                        return (obj->*methodPointer)(argc, argv);
                    }
                ),
                helpDescription
            });
        }
        else if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(int, char const**)>){
            bindingContainer.functions.emplace(name, FunctionInfo{
                std::function<RETURN_TYPE(int, char const**)>(
                    [obj, methodPointer](int argc, char const** argv){
                        return (obj->*methodPointer)(argc, argv);
                    }
                ),
                helpDescription
            });
        }

        // Modern Bindings
        else if constexpr (isModern){
            bindingContainer.functions.emplace(name, FunctionInfo{
                std::function<RETURN_TYPE(std::span<std::string const>)>(
                    [obj, methodPointer](std::span<std::string const> args){
                        return (obj->*methodPointer)(args);
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

template<typename RETURN_TYPE, typename... additionalArgs>
std::vector<std::pair<std::string, std::string const*>> FuncTree<RETURN_TYPE, additionalArgs...>::getAllFunctions(){
    std::vector<std::pair<std::string, std::string const*>> allFunctions;
    for (auto const& [name, info] : bindingContainer.functions){
        allFunctions.emplace_back(name, info.description);
    }

    // Get functions from inherited FuncTrees
    for(auto& inheritedTree : inheritedTrees){
        for (auto const& [name, description] : inheritedTree->getAllFunctions()){
            if (bindingContainer.functions.find(name) == bindingContainer.functions.end()){
                allFunctions.emplace_back(name, description);
            }
        }
    }

    // Get just the names of the bindingContainer.categories
    for (auto const& [categoryName, cat] : bindingContainer.categories){
        allFunctions.emplace_back(categoryName, cat.description);
    }

    return allFunctions;
}

template<typename RETURN_TYPE, typename... additionalArgs>
std::vector<std::pair<std::string, std::string const*>> FuncTree<RETURN_TYPE, additionalArgs...>::getAllVariables(){
    std::vector<std::pair<std::string, std::string const*>> allVariables;
    for (auto const& [name, info] : bindingContainer.variables){
        allVariables.emplace_back(name, info.description);
    }

    // Get from inherited FuncTree
    for (auto& inheritedTree : inheritedTrees){
        // Case by case, making sure we do not have duplicates
        for (auto const& [name, description] : inheritedTree->getAllVariables()){
            if (bindingContainer.variables.find(name) == bindingContainer.variables.end()){
                allVariables.emplace_back(name, description);
            }
        }
    }

    return allVariables;
}

//------------------------------------------
// Parsing and execution

template<typename RETURN_TYPE, typename... additionalArgs>
RETURN_TYPE FuncTree<RETURN_TYPE, additionalArgs...>::parseStr(std::string const& cmd, additionalArgs... addArgs){
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
        [](std::string const& str){ return const_cast<char*>(str.c_str()); });
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
        return standardReturn.valDefault;   // Nothing to execute, return standard
    }

    // turn argc/argv into span
    std::vector<std::string> argsVec;
    argsVec.reserve(argc);
    for(size_t i = 0; i < argc; i++){
        argsVec.emplace_back(argv[i]);
    }
    auto tokensSpan = std::span<std::string const>(argsVec.data(), argsVec.size());

    ////////////////////////////////////////////

    // The first argument left is the new function name
    std::string funcName = tokensSpan.front();

    // Check in inherited FuncTrees first
    auto inheritedTree = findInInheritedTrees(funcName);
    if(inheritedTree != nullptr){
        // Function is in inherited tree, parse there
        return inheritedTree->executeFunction(funcName, argc, argv, tokensSpan, addArgs...);
    }

    // Not found in inherited trees, execute the function the main tree
    return executeFunction(funcName, argc, argv, tokensSpan, addArgs...);
}

template<typename RETURN_TYPE, typename... additionalArgs>
RETURN_TYPE FuncTree<RETURN_TYPE, additionalArgs...>::executeFunction(std::string const& name, int argc, char** argv, std::span<std::string const> const& args, additionalArgs... addArgs){
    // Call preParse function if set
    if(preParse != nullptr){
        RETURN_TYPE err = preParse();
        if(err !=    standardReturn.valDefault){
            return err; // Return error if preParse failed
        }
    }

    // Strip whitespaces of name
    std::string function = name;
    function = Utility::StringHandler::lStrip(function, ' ');
    function = Utility::StringHandler::rStrip(function, ' ');

    // Find and execute the function
    auto functionPosition = bindingContainer.functions.find(function);
    if (functionPosition != bindingContainer.functions.end()){
        auto& [functionPtr, description] = functionPosition->second;
        return std::visit([&]<typename Func>(Func&& func) -> RETURN_TYPE {
            using T = std::decay_t<Func>;

            // Legacy function types
            if constexpr (std::is_same_v<T, std::function<RETURN_TYPE(int, char**)>>){
                // Convert to argc/argv
                return func(argc, argv);
            } else if constexpr (std::is_same_v<T, std::function<RETURN_TYPE(int, char const**)>>){
                // Convert char** to char const**
                std::vector<char const*> argv_const(static_cast<size_t>(argc));
                for (size_t i = 0; i < static_cast<size_t>(argc); ++i) argv_const[i] = argv[i];
                return func(argc, argv_const.data());
            }
            // Modern function types
            else if constexpr (std::is_same_v<T, SpanFn>){
                return func(args, addArgs...);
            }
            // Unknown function type
            else {
                Utility::Capture::cerr() << "Error: Unknown function signature for function '" << function << "' in FuncTree '" << TreeName << "'." << Utility::Capture::endl;
                Utility::Capture::cerr() << "Visitor matched type (mangled):   " << typeid(T).name() << Utility::Capture::endl;
                Utility::Capture::cerr() << "Visitor matched type (demangled): " << demangle(typeid(T).name()) << Utility::Capture::endl;
                std::exit(EXIT_FAILURE);
            }
        }, functionPtr);
    }
    // Find function name in bindingContainer.categories
    if(bindingContainer.categories.find(function) != bindingContainer.categories.end()){
        std::string cmd;
        for(int i = 0; i < argc; i++){
            cmd += std::string(argv[i]) + " ";
        }
        return bindingContainer.categories[function].tree->parseStr(cmd);
    }
    Utility::Capture::cerr() << "Function '" << function << "' not found in FuncTree " << TreeName << ", its inherited FuncTrees or their categories!\n";
    Utility::Capture::cerr() << "Arguments are:" << Utility::Capture::endl;
    for(int i = 0; i < argc; i++){
        Utility::Capture::cerr() << "argv[" << i << "] = '" << argv[i] << "'\n";
    }
    Utility::Capture::cerr() << "Available functions:  " << bindingContainer.functions.size() << Utility::Capture::endl;
    Utility::Capture::cerr() << "Available categories: " << bindingContainer.categories.size()  << Utility::Capture::endl;
    return standardReturn.valFunctionNotFound;  // Return error if function not found
}

template<typename RETURN_TYPE, typename... additionalArgs>
bool FuncTree<RETURN_TYPE, additionalArgs...>::hasFunction(std::string const& nameOrCommand){
    // Make sure only the command name is used
    std::vector<std::string> tokens = Utility::StringHandler::split(nameOrCommand, ' ');

    // Remove all tokens starting with "--"
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
        [](std::string const& token){
            return token.starts_with("--");
        }), tokens.end());

    // Remove all empty tokens
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
        [](std::string const& token){
            return token.empty();
        }), tokens.end());

    if (tokens.empty()){
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
    return bindingContainer.functions.find(function)  != bindingContainer.functions.end() ||
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
}   // namespace SortFunctions

template<typename RETURN_TYPE, typename... additionalArgs>
RETURN_TYPE FuncTree<RETURN_TYPE, additionalArgs...>::help(std::span<std::string const> const& args){
    //------------------------------------------
    // Case 1: Detailed help for a specific function, category or variable
    if(args.size() > 1){
        // Call specific help for each argument, except the first one (which is the binary name or last function name)
        for (auto const& arg : args){
            specificHelp(arg);
        }
        return standardReturn.valDefault;
    }

    //------------------------------------------
    // Case 2: General help for all functions, bindingContainer.categories and variables
    generalHelp();
    return standardReturn.valDefault;
}

template<typename RETURN_TYPE, typename... additionalArgs>
void FuncTree<RETURN_TYPE, additionalArgs...>::specificHelp(std::string const& funcName){
    if (BindingSearchResult const searchResult = find(funcName); searchResult.any){
        // 1.) Function
        if(searchResult.function){
            // Found function, display detailed help
            Utility::Capture::cout() << "\nHelp for function '" << funcName << "':\n" << Utility::Capture::endl;
            Utility::Capture::cout() << *searchResult.funIt->second.description << "\n";
        }
        // 2.) Category
        else if(searchResult.category){
            // Found category, display detailed help
            searchResult.catIt->second.tree->help({}); // Display all functions in the category
        }
        // 3.) Variable
        else if(searchResult.variable){
            // Found variable, display detailed help
            Utility::Capture::cout() << "\nHelp for variable '--" << funcName << "':\n" << Utility::Capture::endl;
            Utility::Capture::cout() << *searchResult.varIt->second.description << "\n";
        }
    }
    else{
        Utility::Capture::cerr() << "Function or Category '" << funcName << "' not found in FuncTree '" << TreeName << "'.\n";
    }
}

template<typename RETURN_TYPE, typename... additionalArgs>
void FuncTree<RETURN_TYPE, additionalArgs...>::generalHelp(){
    // Padding size for names
    // '<name padded> - <description>'
    uint16_t constexpr namePaddingSize = 25;

    // Define a lambda to process each member
    auto displayMember = [](std::string const& name, std::string const* description) -> void {
        // Only show the first line of the description
        std::string descriptionFirstLine = *description;
        if (size_t const newlinePos = description->find('\n'); newlinePos != std::string::npos){
            descriptionFirstLine = description->substr(0, newlinePos);
        }
        std::string paddedName = name;
        paddedName.resize(namePaddingSize, ' ');
        Utility::Capture::cout() << "  " << paddedName << " - " << descriptionFirstLine << Utility::Capture::endl;
    };

    // All info: [name, description]
    auto allFunctions = getAllFunctions();   // includes categories
    auto allVariables = getAllVariables();

    // Sort by name
    std::ranges::sort(allFunctions, SortFunctions::caseInsensitiveLess);
    std::ranges::sort(allVariables, SortFunctions::caseInsensitiveLess);

    // Display:
    Utility::Capture::cout() << "\nHelp for " << TreeName << "\nAdd the entries name to the command for more details: " << TreeName << " help <foo>\n";
    Utility::Capture::cout() << "Available functions:\n";

    // Use lambda with for_each on all functions and variables
    // TODO: using structured bindings here would be nice, but that won't compile for some reason
    std::ranges::for_each(allFunctions, [&](auto const& pair){
        displayMember(pair.first, pair.second);
    });
    Utility::Capture::cout() << "Available variables:\n";
    std::ranges::for_each(allVariables, [&](auto const& pair){
        displayMember(pair.first, pair.second);
    });
}

template<typename RETURN_TYPE, typename... additionalArgs>
FuncTree<RETURN_TYPE, additionalArgs...>::BindingSearchResult
FuncTree<RETURN_TYPE, additionalArgs...>::find(std::string const& name) {
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


}   // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP