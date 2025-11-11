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
        exit(EXIT_FAILURE);
    }

    inline void FunctionShadowsCategory(std::string const& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "A Nebulite FuncTree binding failed!" << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "Error: Cannot bind function '" << function << "' because a category with the same name already exists." << Nebulite::Utility::Capture::endl;
        exit(EXIT_FAILURE);
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
        std::exit(EXIT_FAILURE);
    }

    inline void FunctionExists(std::string const& tree, std::string const& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
        Nebulite::Utility::Capture::cerr() << "Nebulite FuncTree initialization failed!\n";
        Nebulite::Utility::Capture::cerr() << "Error: A bound Function already exists in this tree.\n";
        Nebulite::Utility::Capture::cerr() << "Function overwrite is heavily discouraged and thus not allowed.\n";
        Nebulite::Utility::Capture::cerr() << "Please choose a different name or remove the existing function.\n";
        Nebulite::Utility::Capture::cerr() << "This Tree: " << tree << "\n";
        Nebulite::Utility::Capture::cerr() << "Function:  " << function << "\n";
        std::exit(EXIT_FAILURE);
    }

    inline void UnknownMethodPointerType(std::string const& tree, std::string const& function){
        Nebulite::Utility::Capture::cerr() << "---------------------------------------------------------------\n";
        Nebulite::Utility::Capture::cerr() << "Nebulite FuncTree initialization failed!\n";
        Nebulite::Utility::Capture::cerr() << "Error: Unknown method pointer type for function '" << function << "' in FuncTree '" << tree << "'.\n";
        std::exit(EXIT_FAILURE);
    }
} // anonymous namespace

static std::string demangle(const char* name) {
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

template <typename RETURN_TYPE>
FuncTree<RETURN_TYPE>::FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError)
: _standard(standard), _functionNotFoundError(functionNotFoundError), TreeName(std::move(treeName))
{
    // construct the help entry in-place to avoid assignment and ambiguous lambda conversions
    functions.emplace(
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

template<typename RETURN_TYPE>
template<typename ClassType>
void FuncTree<RETURN_TYPE>::bindFunction(
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
        absl::flat_hash_map<std::string, category>* currentCategoryMap = &categories;
        FuncTree* targetTree = this;
        for(size_t idx = 0; idx < pathStructure.size() - 1; idx++){
            std::string const& currentCategoryName = pathStructure[idx];
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

    // Check for name conflicts
    conflictCheck(name);

    // Bind to this tree directly
    directBind(name, helpDescription, method, obj);
}

template<typename RETURN_TYPE>
bool FuncTree<RETURN_TYPE>::bindCategory(std::string const& name, std::string const* helpDescription){
    if(categories.find(name) != categories.end()){
        // Category already exists
        /**
         * @note Warning is suppressed here,
         * as with different modules we might need to call this in each module,
         * just to make sure the category exists
         */
        // Utility::Capture::cerr() << "Warning: A category with the name '" << name << "' already exists in the FuncTree '" << TreeName << "'." << Utility::Capture::endl;
        return false;
    }
    // Split based on whitespaces
    std::vector<std::string> const categoryStructure = Utility::StringHandler::split(name, ' ');
    size_t const depth = categoryStructure.size();

    absl::flat_hash_map<std::string, category>* currentCategoryMap = &categories;
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
                Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because parent category '"
                          << currentCategoryName << "' does not exist." << Utility::Capture::endl;
                exit(EXIT_FAILURE);
            }
        }
        else{
            // Last category, create it, if it doesn't exist yet
            if(currentCategoryMap->find(currentCategoryName) != currentCategoryMap->end()){
                // Category exists, throw error
                Utility::Capture::cerr() << "---------------------------------------------------------------\n";
                Utility::Capture::cerr() << "A Nebulite FuncTree initialization failed!\n";
                Utility::Capture::cerr() << "Error: Cannot create category '" << name << "' because it already exists." << Utility::Capture::endl;
                exit(EXIT_FAILURE);
            }
            // Create category
            (*currentCategoryMap)[currentCategoryName] = {std::make_unique<FuncTree>(currentCategoryName, _standard, _functionNotFoundError), helpDescription};
        }
    }
    return true;
}

// Using noLint, as varPtr would be flagged as it's not const.
// But this causes issues with binding variables.
template<typename RETURN_TYPE>
// NOLINTNEXTLINE
void FuncTree<RETURN_TYPE>::bindVariable(bool* varPtr, std::string const& name, std::string const* helpDescription){
    // Make sure there are no whitespaces in the variable name
    if (name.find(' ') != std::string::npos){
        Utility::Capture::cerr() << "Error: Variable name '" << name << "' cannot contain whitespaces." << Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    // Make sure the variable isn't bound yet
    if (variables.find(name) != variables.end()){
        Utility::Capture::cerr() << "Error: Variable '" << name << "' is already bound." << Utility::Capture::endl;
        exit(EXIT_FAILURE);
    }

    // Bind the variable
    variables.emplace(name, VariableInfo{varPtr, helpDescription});
}

//------------------------------------------
// Binding helper functions

template <typename RETURN_TYPE>
void FuncTree<RETURN_TYPE>::conflictCheck(std::string const &name) {
    for (auto const& [categoryName, _] : categories){
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

template <typename RETURN_TYPE>
template<typename ClassType>
void FuncTree<RETURN_TYPE>::directBind(std::string const& name, std::string const* helpDescription, MemberMethod<ClassType> method, ClassType* obj){
    // Use std::visit to bind the correct function type
    std::visit([&]<typename MethodPointer>(MethodPointer&& methodPointer){
        using MethodType = std::decay_t<MethodPointer>;

        // Legacy Bindings
        if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(int, char**)>){
            functions.emplace(name, FunctionInfo{
                std::function<RETURN_TYPE(int, char**)>(
                    [obj, methodPointer](int argc, char** argv){
                        return (obj->*methodPointer)(argc, argv);
                    }
                ),
                helpDescription
            });
        }
        else if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(int, char const**)>){
            functions.emplace(name, FunctionInfo{
                std::function<RETURN_TYPE(int, char const**)>(
                    [obj, methodPointer](int argc, char const** argv){
                        return (obj->*methodPointer)(argc, argv);
                    }
                ),
                helpDescription
            });
        }

        // Modern Bindings
        else if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(std::span<std::string const>)>){
            functions.emplace(name, FunctionInfo{
                std::function<RETURN_TYPE(std::span<std::string const>)>(
                    [obj, methodPointer](std::span<std::string const> args){
                        return (obj->*methodPointer)(args);
                    }
                ),
                helpDescription
            });
        }
        // 4.) Bind std::span<std::string const> const method
        else if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(std::span<std::string const>) const> ||
                           std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(std::span<std::string const>) const&>){
            functions.emplace(name, FunctionInfo{
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

template<typename RETURN_TYPE>
std::vector<std::pair<std::string, std::string const*>> FuncTree<RETURN_TYPE>::getAllFunctions(){
    std::vector<std::pair<std::string, std::string const*>> allFunctions;
    for (auto const& [name, info] : functions){
        allFunctions.emplace_back(name, info.description);
    }

    // Get functions from inherited FuncTrees
    for(auto& inheritedTree : inheritedTrees){
        for (auto const& [name, description] : inheritedTree->getAllFunctions()){
            if (functions.find(name) == functions.end()){
                allFunctions.emplace_back(name, description);
            }
        }
    }

    // Get just the names of the categories
    for (auto const& [categoryName, cat] : categories){
        allFunctions.emplace_back(categoryName, cat.description);
    }

    return allFunctions;
}

template<typename RETURN_TYPE>
std::vector<std::pair<std::string, std::string const*>> FuncTree<RETURN_TYPE>::getAllVariables(){
    std::vector<std::pair<std::string, std::string const*>> allVariables;
    for (auto const& [name, info] : variables){
        allVariables.emplace_back(name, info.description);
    }

    // Get from inherited FuncTree
    for (auto& inheritedTree : inheritedTrees){
        // Case by case, making sure we do not have duplicates
        for (auto const& [name, description] : inheritedTree->getAllVariables()){
            if (variables.find(name) == variables.end()){
                allVariables.emplace_back(name, description);
            }
        }
    }

    return allVariables;
}

//------------------------------------------
// Parsing and execution

template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::parseStr(std::string const& cmd){
    // Quote-aware tokenization
    std::vector<std::string> tokens = Utility::StringHandler::parseQuotedArguments(cmd);

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
        return _standard;   // Nothing to execute, return standard
    }

    // The first argument left is the new function name
    std::string funcName = argv[0];

    // Check in inherited FuncTrees first
    auto inheritedTree = findInInheritedTrees(funcName);
    if(inheritedTree != nullptr){
        // Function is in inherited tree, parse there
        return inheritedTree->executeFunction(funcName, static_cast<int>(argc), argv);
    }

    // Not found in inherited trees, execute the function the main tree
    return executeFunction(funcName, static_cast<int>(argc), argv);
}

// TODO: Modify to take all types of arguments and passing them to the functions
//       executeFunction(name, args)
template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::executeFunction(std::string const& name, int argc, char* argv[]){
    // Call preParse function if set
    if(preParse != nullptr){
        RETURN_TYPE err = preParse();
        if(err != _standard){
            return err; // Return error if preParse failed
        }
    }

    // Strip whitespaces of name
    std::string function = name;
    function = Utility::StringHandler::lStrip(function, ' ');
    function = Utility::StringHandler::rStrip(function, ' ');

    // Find and execute the function
    auto functionPosition = functions.find(function);
    if (functionPosition != functions.end()){
        auto& [functionPtr, description] = functionPosition->second;
        return std::visit([&]<typename Func>(Func&& func) -> RETURN_TYPE {
            using T = std::decay_t<Func>;

            // Legacy function types
            if constexpr (std::is_same_v<T, std::function<RETURN_TYPE(int, char**)>>){
                return func(argc, argv);
            } else if constexpr (std::is_same_v<T, std::function<RETURN_TYPE(int, char const**)>>){
                std::vector<char const*> argv_const(static_cast<size_t>(argc));
                for (size_t i = 0; i < static_cast<size_t>(argc); ++i) argv_const[i] = argv[i];
                return func(argc, argv_const.data());
            }
            // Modern function types
            else if constexpr (std::is_same_v<T, SpanFn>){
                // Convert argc, argv to std::span<std::string const>
                std::vector<std::string> argsVec(static_cast<size_t>(argc));
                for (size_t i = 0; i < static_cast<size_t>(argc); ++i){
                    argsVec[i] = std::string(argv[i]);
                }
                std::span<std::string const> argsSpan(argsVec.data(), argsVec.size());
                return func(argsSpan);
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
    // Find function name in categories
    if(categories.find(function) != categories.end()){
        std::string cmd;
        for(int i = 0; i < argc; i++){
            cmd += std::string(argv[i]) + " ";
        }
        return categories[function].tree->parseStr(cmd);
    }
    Utility::Capture::cerr() << "Function '" << function << "' not found in FuncTree " << TreeName << " or its SubTrees!\n";
    Utility::Capture::cerr() << "Arguments are:" << Utility::Capture::endl;
    for(int i = 0; i < argc; i++){
        Utility::Capture::cerr() << "argv[" << i << "] = '" << argv[i] << "'\n";
    }
    Utility::Capture::cerr() << "Available functions: " << functions.size() << Utility::Capture::endl;
    Utility::Capture::cerr() << "Available SubTrees:  " << categories.size()  << Utility::Capture::endl;
    return _functionNotFoundError;  // Return error if function not found
}

template<typename RETURN_TYPE>
bool FuncTree<RETURN_TYPE>::hasFunction(std::string const& nameOrCommand){
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
    return  functions.find(function)  != functions.end() ||
            categories.find(function) != categories.end();
}

//------------------------------------------
// Help function and its helpers

namespace SortFunctions {
    // Case-insensitive comparison function
    inline auto caseInsensitiveLess = [](auto const& a, auto const& b){
        std::string const& sa = a.first;
        std::string const& sb = b.first;
        size_t const n = std::min(sa.size(), sb.size());
        for (size_t i = 0; i < n; ++i){
            char const ca = static_cast<char>(std::tolower(static_cast<unsigned char>(sa[i])));
            char const cb = static_cast<char>(std::tolower(static_cast<unsigned char>(sb[i])));
            if (ca < cb) return true;
            if (ca > cb) return false;
        }
        return sa.size() < sb.size();
    };
}   // namespace SortFunctions

template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::help(std::span<std::string const> const& args){
    //------------------------------------------
    // Case 1: Detailed help for a specific function, category or variable
    if(args.size() > 1){
        // Call specific help for each argument, except the first one (which is the binary name or last function name)
        for (auto const& arg : args){
            specificHelp(arg);
        }
        return _standard;
    }

    //------------------------------------------
    // Case 2: General help for all functions, categories and variables
    generalHelp();
    return _standard;
}

template<typename RETURN_TYPE>
void FuncTree<RETURN_TYPE>::specificHelp(std::string funcName){
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
        Utility::Capture::cout() << "\nHelp for function '" << funcName << "':\n" << Utility::Capture::endl;
        Utility::Capture::cout() << *funcIt->second.description << "\n";
    }
    // 2.) Category
    else if(subFound){
        // Found category, display detailed help
        subIt->second.tree->help({}); // Display all functions in the category
    }
    // 3.) Variable
    else if(varFound){
        // Found variable, display detailed help
        Utility::Capture::cout() << "\nHelp for variable '--" << funcName << "':\n" << Utility::Capture::endl;
        Utility::Capture::cout() << *varIt->second.description << "\n";
    }
    // 4.) Not found
    else{
        Utility::Capture::cerr() << "Function or Category '" << funcName << "' not found in FuncTree '" << TreeName << "'.\n";
    }
}

template<typename RETURN_TYPE>
void FuncTree<RETURN_TYPE>::generalHelp(){
    // Padding size for names
    // '<name padded> - <description>'
    uint16_t constexpr namePaddingSize = 25;

    // All info: [name, description]
    std::vector<std::pair<std::string, std::string const*>> allFunctions = getAllFunctions();
    std::vector<std::pair<std::string, std::string const*>> allVariables = getAllVariables();

    // Sort by name
    std::ranges::sort(allFunctions, SortFunctions::caseInsensitiveLess);
    std::ranges::sort(allVariables, SortFunctions::caseInsensitiveLess);

    // Display:
    Utility::Capture::cout() << "\nHelp for " << TreeName << "\nAdd the entries name to the command for more details: " << TreeName << " help <foo>\n";
    Utility::Capture::cout() << "Available functions:\n";
    for (auto const& [name, description] : allFunctions){
        // Only show the first line of the description
        std::string descriptionFirstLine = *description;
        if (size_t const newlinePos = description->find('\n'); newlinePos != std::string::npos){
            descriptionFirstLine = description->substr(0, newlinePos);
        }
        std::string paddedName = name;
        paddedName.resize(namePaddingSize, ' ');
        Utility::Capture::cout() << "  " << paddedName << " - " << descriptionFirstLine << Utility::Capture::endl;
    }

    // Display variables
    Utility::Capture::cout() << "Available variables:\n";
    for (auto const& [name, description] : allVariables){
        std::string paddedName = name;
        paddedName.resize(namePaddingSize, ' ');
        Utility::Capture::cout() << "  " << paddedName << " - " << *description << Utility::Capture::endl;
    }
}

template<typename RETURN_TYPE>
void FuncTree<RETURN_TYPE>::find(std::string const& name, bool& funcFound, auto& funcIt,  bool& subFound, auto& subIt, bool& varFound, auto& varIt){
    // Functions
    if(funcIt != functions.end()){
        funcFound = true;
    }
    else{
        for(auto const& inheritedTree : inheritedTrees){
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
    if(subIt != categories.end()){
        subFound = true;
    }
    else{
        for(auto const& inheritedTree : inheritedTrees){
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
    if(varIt != variables.end()){
        varFound = true;
    }
    else{
        for(auto const& inheritedTree : inheritedTrees){
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

}   // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP