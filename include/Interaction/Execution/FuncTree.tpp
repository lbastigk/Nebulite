/**
 * @file FuncTree.tpp
 * @brief Implementation file for the FuncTree class template.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP
#define NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP

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

//------------------------------------------
namespace Nebulite::Interaction::Execution{


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

    // Use std::visit to bind the correct function type
    std::visit([&]<typename MethodPointer>(MethodPointer&& methodPointer){
        using MethodType = std::decay_t<MethodPointer>;

        // 1.) Bind classic int, char** method
        if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(int, char**)>){
            functions[name] = FunctionInfo{
                [obj, methodPointer](int argc, char** argv){
                    return (obj->*methodPointer)(argc, argv);
                },
                helpDescription
            };
        // 2.) Bind int, char const** method
        } else if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(int, char const**)>){
            functions[name] = FunctionInfo{
                [obj, methodPointer](int argc, char** argv){
                    std::vector<char const*> argv_const(static_cast<size_t>(argc));
                    for (size_t i = 0; i < static_cast<size_t>(argc); ++i) argv_const[i] = argv[i];
                    return (obj->*methodPointer)(argc, argv_const.data());
                },
                helpDescription
            };
        // 3.) Bind std::span<std::string const&> method
        } else if constexpr (std::is_same_v<MethodType, RETURN_TYPE (ClassType::*)(std::span<std::string const>)>){
            functions[name] = FunctionInfo{
                std::function<RETURN_TYPE(std::span<std::string const>)>(
                    [obj, methodPointer](std::span<std::string const> args) {
                        return (obj->*methodPointer)(args);
                    }
                ),
                helpDescription
            };
        } else {
            bindErrorMessage::UnknownMethodPointerType(TreeName, name);
        }
    }, method);
}

// Using noLint, as varPtr would be flagged as it's not const.
// But this causes issues with binding variables.
template<typename RETURN_TYPE>
// NOLINTNEXTLINE
void Nebulite::Interaction::Execution::FuncTree<RETURN_TYPE>::bindVariable(bool* varPtr, std::string const& name, std::string const* helpDescription){
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
    variables[name] = VariableInfo{varPtr, helpDescription};
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
// Constructor implementation

template <typename RETURN_TYPE>
FuncTree<RETURN_TYPE>::FuncTree(std::string treeName, RETURN_TYPE standard, RETURN_TYPE functionNotFoundError)
: _standard(standard), _functionNotFoundError(functionNotFoundError), TreeName(std::move(treeName))
{
    // Attach the help function to read out the description of all attached functions
    functions["help"] = FunctionInfo{
        std::function<RETURN_TYPE(int, char const**)>(
            [this](int const argc, char const** argv){
                return this->help(argc, argv);
            }
        ),
    &help_desc};
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
            if constexpr (std::is_same_v<T, std::function<RETURN_TYPE(int, char**)>>){
                return func(argc, argv);
            } else if constexpr (std::is_same_v<T, std::function<RETURN_TYPE(int, char const**)>>){
                std::vector<char const*> argv_const(static_cast<size_t>(argc));
                for (size_t i = 0; i < static_cast<size_t>(argc); ++i) argv_const[i] = argv[i];
                return func(argc, argv_const.data());
            } else {
                Utility::Capture::cerr() << "Error: Unknown function type for function '" << function << "' in FuncTree '" << TreeName << "'." << Utility::Capture::endl;
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

template<typename RETURN_TYPE>
RETURN_TYPE FuncTree<RETURN_TYPE>::help(int const argc, char const* argv[]){
    //------------------------------------------
    // Case 1: Detailed help for a specific function, category or variable
    if(argc > 1){
        for(int i = 1; i < argc; i++){
            std::string const funcName = argv[i];
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
        subIt->second.tree->help(0, nullptr); // Display all functions in the category
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

    // Case-insensitive comparison function
    auto caseInsensitiveLess = [](auto const& a, auto const& b){
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

    // Sort by name
    std::sort(allFunctions.begin(), allFunctions.end(), caseInsensitiveLess);
    std::sort(allVariables.begin(), allVariables.end(), caseInsensitiveLess);

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