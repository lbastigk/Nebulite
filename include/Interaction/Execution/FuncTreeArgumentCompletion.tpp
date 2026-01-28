/**
 * @file FuncTree.tpp
 * @brief Implementation file for the FuncTree argument completion and help functionality.
 * @details Contains all functions related to:
 *          - Argument completion
 *          - Help display
 *          - Searching for bindings
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_FUNCTREE_ARGUMENT_COMPLETION_TPP
#define NEBULITE_INTERACTION_EXECUTION_FUNCTREE_ARGUMENT_COMPLETION_TPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <string>

// Nebulite
// NOLINTNEXTLINE
#include "Interaction/Execution/FuncTree.hpp" // Why does clang-tidy complain about this include being unnecessary?

//------------------------------------------
namespace Nebulite::Interaction::Execution {

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

template <typename returnValue, typename... additionalArgs>
bool FuncTree<returnValue, additionalArgs...>::hasFunction(std::string_view const& nameOrCommand) {
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

template <typename returnValue, typename... additionalArgs>
returnValue FuncTree<returnValue, additionalArgs...>::help(std::span<std::string const> const& args) {
    //------------------------------------------
    // Case 1: Detailed help for a specific function, category or variable
    if (args.size() > 1) {
        // Call specific help for each argument, except the first one (which is the binary name or last function name)
        for (auto const& arg : args.subspan(1)) {
            specificHelp(arg);
        }
        return standardReturn.valDefault;
    }

    //------------------------------------------
    // Case 2: General help for all functions, bindingContainer.categories and variables
    generalHelp();
    return standardReturn.valDefault;
}

template <typename returnValue, typename... additionalArgs>
void FuncTree<returnValue, additionalArgs...>::specificHelp(std::string const& funcName) {
    if (BindingSearchResult const searchResult = find(funcName); searchResult.any) {
        // 1.) Function
        if (searchResult.function) {
            // Found function, display detailed help
            Utility::Capture::cout() << "\nHelp for function '" << funcName << "':\n" << Utility::Capture::endl;
            Utility::Capture::cout() << searchResult.funIt->second.description << "\n";
        }
        // 2.) Category
        else if (searchResult.category) {
            // Found category, display detailed help
            searchResult.catIt->second.tree->help({}); // Display all functions in the category
        }
        // 3.) Variable
        else if (searchResult.variable) {
            // Found variable, display detailed help
            Utility::Capture::cout() << "\nHelp for variable '--" << funcName << "':\n" << Utility::Capture::endl;
            Utility::Capture::cout() << searchResult.varIt->second.description << "\n";
        }
    } else {
        Utility::Capture::cerr() << "Function or Category '" << funcName << "' not found in FuncTree '" << TreeName << "'.\n";
    }
}

template <typename returnValue, typename... additionalArgs>
void FuncTree<returnValue, additionalArgs...>::generalHelp() {
    // Padding size for names
    // '<name padded> - <description>'
    uint16_t constexpr namePaddingSize = 25;

    // Define a lambda to process each member
    auto displayMember = [](std::string const& name, std::string_view const& description) -> void {
        // Only show the first line of the description
        auto descriptionFirstLine = std::string(description);
        if (size_t const newlinePos = description.find('\n'); newlinePos != std::string::npos) {
            descriptionFirstLine = description.substr(0, newlinePos);
        }
        std::string paddedName = name;
        paddedName.resize(namePaddingSize, ' ');
        Utility::Capture::cout() << "  " << paddedName << " - " << descriptionFirstLine << Utility::Capture::endl;
    };

    // All info: [name, description]
    auto allFunctions = getAllFunctions(); // includes categories
    auto allVariables = getAllVariables();

    // Sort by name
    std::ranges::sort(allFunctions, SortFunctions::caseInsensitiveLess);
    std::ranges::sort(allVariables, SortFunctions::caseInsensitiveLess);

    // Display:
    Utility::Capture::cout() << "\nHelp for " << TreeName << "\nAdd the entries name to the command for more details: " << TreeName << " help <foo>\n";
    Utility::Capture::cout() << "Available functions:\n";

    // Use lambda with for_each on all functions and variables
    // TODO: using structured bindings here would be nice, but that won't compile for some reason
    std::ranges::for_each(allFunctions, [&](auto const& pair) {
        displayMember(pair.first, pair.second);
    });
    Utility::Capture::cout() << "Available variables:\n";
    std::ranges::for_each(allVariables, [&](auto const& pair) {
        displayMember(pair.first, pair.second);
    });
}

template <typename returnValue, typename... additionalArgs>
FuncTree<returnValue, additionalArgs...>::BindingSearchResult
FuncTree<returnValue, additionalArgs...>::find(std::string const& name) {
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
returnValue FuncTree<returnValue, additionalArgs...>::complete(std::span<std::string const> const& args){
    auto argsSpan = args.subspan(1); // Skip binary name or last function name
    FuncTree* ftree = this;
    std::string pattern;
    if (argsSpan.empty()) {
        // No pattern provided
        pattern = "";
    }
    else {
        pattern = argsSpan.back();
        argsSpan = argsSpan.subspan(0, argsSpan.size() - 1); // Remove pattern from argsSpan

        // Traverse into categories
        std::ranges::for_each(argsSpan, [&](std::string const& arg) {
            if (ftree) ftree = traverseIntoCategory(arg, ftree);
        });

        // Return if traversal failed -> no completions
        if (ftree == nullptr) {
            return standardReturn.valDefault;
        }
    }

    // Find completions for the pattern in the current FuncTree
    auto completions = ftree->findCompletions(pattern);

    // If there is only one completion, it might be a category, so we traverse into it
    if (bool const lastWordIsLikelyCategory = completions.size() == 1 && completions.front() == pattern; lastWordIsLikelyCategory) {
        if (auto newTree = traverseIntoCategory(pattern, ftree); newTree) {
            completions = newTree->findCompletions("");
        }
        else{
            completions.clear(); // No completions found
        }
    }

    // Sort and remove duplicates, filter out __complete__ from completions
    std::ranges::sort(completions);
    completions.erase(std::unique(completions.begin(), completions.end()), completions.end());
    completions.erase(std::remove(completions.begin(), completions.end(), "__complete__"), completions.end());

    // Remove any argument that is exactly equal to the pattern provided
    std::erase_if(completions, [&](std::string const& completion){
        return completion == pattern;
    });

    // Output completions to stdout
    std::ranges::for_each(completions, [](std::string const& completion){
        Utility::Capture::cout().println(completion);
    });

    return standardReturn.valDefault;
}

template <typename returnValue, typename ... additionalArgs>
FuncTree<returnValue, additionalArgs...>* FuncTree<returnValue, additionalArgs...>::traverseIntoCategory(std::string const& categoryName, FuncTree const* ftree) {
    // Check direct categories first
    if (auto catIt = ftree->bindingContainer.categories.find(categoryName); catIt != ftree->bindingContainer.categories.end()) {
        return catIt->second.tree.get();
    }

    // Category not found, check inherited trees
    auto it = std::ranges::find_if(ftree->inheritedTrees, [&](auto const& inheritedTree) {
        return inheritedTree && inheritedTree->bindingContainer.categories.contains(categoryName);
    });
    if (it != ftree->inheritedTrees.end()) {
        return (*it)->bindingContainer.categories.find(categoryName)->second.tree.get();
    }

    // No category found
    return nullptr;
}

template <typename returnValue, typename ... additionalArgs>
std::vector<std::string> FuncTree<returnValue, additionalArgs...>::findCompletions(std::string const& pattern) {
    std::vector<std::string> completions;
    auto collect = [&](auto const& map, std::string_view const prefix = "") {
        for (auto const& [name, _] : map) {
            if (std::string const full = std::string(prefix) + name; full.starts_with(pattern)) {
                completions.push_back(std::move(full));
            }
        }
    };

    collect(bindingContainer.functions);
    collect(bindingContainer.categories);
    collect(bindingContainer.variables, "--");
    std::ranges::for_each(inheritedTrees, [&](auto const& inheritedTree){
        if (inheritedTree) {
            auto inheritedCompletions = inheritedTree->findCompletions(pattern);
            std::ranges::move(inheritedCompletions, std::back_inserter(completions));
        }
    });
    return completions;
}

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_FUNCTREE_ARGUMENT_COMPLETION_TPP
