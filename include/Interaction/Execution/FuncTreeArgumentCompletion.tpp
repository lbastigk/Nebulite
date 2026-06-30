#ifndef INTERACTION_EXECUTION_FUNCTREEARGUMENTCOMPLETION_TPP
#define INTERACTION_EXECUTION_FUNCTREEARGUMENTCOMPLETION_TPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef> // NOLINT
#include <cstdint> // NOLINT
#include <iterator>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// Nebulite
#include "Utility/StringHandler.hpp"

//------------------------------------------
// Conditional includes

#ifndef INTERACTION_EXECUTION_FUNCTREE_HPP
#include "Interaction/Execution/FuncTree.hpp"
#endif // INTERACTION_EXECUTION_FUNCTREE_HPP

//------------------------------------------
namespace Nebulite::Interaction::Execution {

template <typename ReturnValue, typename... AdditionalArgs>
bool FuncTree<ReturnValue, AdditionalArgs...>::hasFunction(std::string_view const nameOrCommand) {
    // Make sure only the command name is used
    auto tokens = Utility::StringHandler::split(nameOrCommand, ' ');

    // Remove all tokens starting with "--" or empty tokens
    std::erase_if(tokens, [](std::string_view const token) {
        return token.empty() || token.starts_with("--");
    });

    if (tokens.empty()) {
        return false; // No command provided
    }

    // Depending on token count, function name is at different positions
    auto const& function = [tokens] {
        if (tokens.size() == 1) {
            // Case 1:
            // Is a single function name.
            // e.g.: "set"
            return tokens[0];
        }
        // Case 2:
        // Is a full command
        // e.g.: <whereCommandComesFrom> set key value
        return tokens[1];
    }();

    // See if the function is linked
    return bindingContainer.functions.contains(function) || bindingContainer.categories.contains(function);
}

template <typename ReturnValue, typename... AdditionalArgs>
ReturnValue FuncTree<ReturnValue, AdditionalArgs...>::help(std::span<std::string_view const> const& args) {
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
    // Case 2: General help for all functions, categories and variables
    generalHelp();
    return standardReturn.valDefault;
}

template <typename ReturnValue, typename... AdditionalArgs>
void FuncTree<ReturnValue, AdditionalArgs...>::specificHelp(std::string_view funcName) {
    if (BindingSearchResult const searchResult = find(funcName); searchResult.has_value()) {
        std::visit([&]<typename T>(T& iterator) {
            using Decayed = std::decay_t<T>;

            if constexpr (std::is_same_v<Decayed, categoryIterator>) {
                iterator->second.tree->help({});
            }
            else if constexpr (std::is_same_v<Decayed, functionIterator>) {
                capture.log.println();
                capture.log.println("Help for function '", funcName, "':");
                capture.log.println();
                capture.log.println(iterator->second.description);
            }
            else if constexpr (std::is_same_v<Decayed, variableIterator>) {
                capture.log.println();
                capture.log.println("Help for variable '--", funcName, "':");
                capture.log.println();
                capture.log.println(iterator->second.description);
            }
        }, searchResult.value());
    } else {
        capture.error.println("Function or Category '", funcName, "' not found in FuncTree '", TreeName, "'.");
    }
}

template <typename ReturnValue, typename... AdditionalArgs>
void FuncTree<ReturnValue, AdditionalArgs...>::generalHelp() {
    // Padding size for names
    // '<name padded> - <description>'
    std::uint16_t constexpr namePaddingSize = 25;

    // Define a lambda to process each member
    auto displayMember = [&](std::string_view const name, std::string_view description) -> void {
        // Only show the first line of the description
        auto descriptionFirstLine = std::string(description);
        if (std::size_t const newlinePos = description.find('\n'); newlinePos != std::string::npos) {
            descriptionFirstLine = description.substr(0, newlinePos);
        }
        auto paddedName = std::string(name);
        paddedName.resize(namePaddingSize, ' ');
        capture.log.println("  ", paddedName, " - ", descriptionFirstLine);
    };

    // All info: [name, description]
    auto allFunctions = getAllFunctions(); // includes categories
    auto allVariables = getAllVariables();

    // Filter duplicates
    std::ranges::sort(allFunctions, {}, &std::pair<std::string, std::string_view>::first);
    std::erase_if(allFunctions, [seen = std::string{}](auto const& item) mutable {
        bool const duplicate = item.first == seen;
        seen = item.first;
        return duplicate;
    });
    std::ranges::sort(allVariables, {}, &std::pair<std::string, std::string_view>::first);
    std::erase_if(allVariables, [seen = std::string{}](auto const& item) mutable {
        bool const duplicate = item.first == seen;
        seen = item.first;
        return duplicate;
    });

    // Display:
    capture.log.println();
    capture.log.println("Help for ", TreeName);
    capture.log.println("Add the entries name to the command for more details: ", TreeName, " help <foo>");
    capture.log.println("Available functions:");
    std::ranges::for_each(allFunctions, [&](auto const& funcInfo) {
        auto const& [name, description] = funcInfo;
        displayMember(name, description);
    });
    capture.log.println("Available variables:");
    std::ranges::for_each(allVariables, [&](auto const& varInfo) {
        auto const& [name, description] = varInfo;
        displayMember(name, description);
    });
}

template <typename ReturnValue, typename... AdditionalArgs>
FuncTree<ReturnValue, AdditionalArgs...>::BindingSearchResult
FuncTree<ReturnValue, AdditionalArgs...>::find(std::string_view name) {
    // Helper lambda to search in inherited trees
    auto searchInInherited = [&](auto mapMember, auto& iteratorMember) -> bool {
        for (auto const& inheritedTree : inheritedTrees) {
            if (inheritedTree) {
                iteratorMember = (inheritedTree->bindingContainer.*mapMember).find(name);
                if (iteratorMember != (inheritedTree->bindingContainer.*mapMember).end()) {
                    return true;
                }
            }
        }
        return false;
    };

    // --- Categories ---
    categoryIterator catIt = bindingContainer.categories.find(name);
    if (catIt == bindingContainer.categories.end()) {
        if (searchInInherited(&BindingContainer::categories, catIt)) {
            return catIt;
        }
    }
    else {
        return catIt;
    }

    // --- Functions ---
    functionIterator funIt = bindingContainer.functions.find(name);
    if (funIt == bindingContainer.functions.end()) {
        if (searchInInherited(&BindingContainer::functions, funIt)) {
            return funIt;
        }
    }
    else {
        return funIt;
    }

    // --- Variables ---
    variableIterator varIt = bindingContainer.variables.find(name);
    if (varIt == bindingContainer.variables.end()) {
        if (searchInInherited(&BindingContainer::variables, varIt)) {
            return varIt;
        }
    }
    else {
        return varIt;
    }

    return std::nullopt;
}

template <typename ReturnValue, typename ... AdditionalArgs>
ReturnValue FuncTree<ReturnValue, AdditionalArgs...>::complete(std::span<std::string_view const> const& args){
    // Traverse into categories based on args, get pattern to complete
    auto const [pattern, ftree] = [&]() -> std::pair<std::string_view, FuncTree*> {
        auto argsSpan = args.subspan(1); // Skip binary name or last function name
        if (argsSpan.empty()) {
            // No pattern provided
            return {"", this};
        }
        FuncTree* innerTree = this;
        auto const lastArg = argsSpan.back();
        argsSpan = argsSpan.subspan(0, argsSpan.size() - 1); // Remove pattern from argsSpan

        // Traverse into categories
        std::ranges::for_each(argsSpan, [&](auto const& arg) {
            if (innerTree) innerTree = traverseIntoCategory(arg, innerTree);
        });
        return {lastArg, innerTree}; // innerTree is potentially nullptr
    }();

    // Return if traversal failed -> no completions
    if (ftree == nullptr) {
        return standardReturn.valDefault;
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

    // Filter out __complete__ from and any argument that is exactly equal to the pattern provided
    std::ranges::sort(completions);
    std::erase_if(completions, [&](auto const& completion) {
        return completion == pattern; // NOLINT
    });
    std::erase(completions, "__complete__");

    // Output completions to stdout
    std::ranges::for_each(completions, [&](auto const& completion){
        capture.log.println(completion);
    });
    return standardReturn.valDefault;
}

template <typename ReturnValue, typename ... AdditionalArgs>
FuncTree<ReturnValue, AdditionalArgs...>* FuncTree<ReturnValue, AdditionalArgs...>::traverseIntoCategory(std::string_view categoryName, FuncTree const* ftree) {
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

template <typename ReturnValue, typename ... AdditionalArgs>
std::vector<std::string> FuncTree<ReturnValue, AdditionalArgs...>::findCompletionForFullCommand(std::string_view const patternStr) {
    auto [argsVec, _] = Utility::StringHandler::parseQuotedArguments(patternStr);

    // Traverse into categories based on args, get pattern to complete
    auto const [pattern, ftree] = [&]() -> std::pair<std::string, FuncTree*> {
        auto args = std::span(argsVec.data(), argsVec.size());
        if (args.empty()) {
            // No pattern provided, assume root
            return {"", this};
        }
        FuncTree* innerTree = this;
        std::string const lastArg = args.back();
        args = args.subspan(0, args.size() - 1); // Remove pattern from argsSpan

        // Traverse into categories
        std::ranges::for_each(args, [&](std::string_view const arg) {
            if (innerTree) innerTree = traverseIntoCategory(arg, innerTree);
        });
        return {lastArg, innerTree}; // innerTree is potentially nullptr
    }();

    // Return if traversal failed -> no completions
    if (ftree == nullptr) {
        return {};
    }

    // If patternStr ends with a whitespace, we cannot use the root tree. Otherwise, input like "error " is completed with "error-log"
    if (patternStr.ends_with(' ') && ftree == this) {
        return {};
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

    // Filter out __complete__ from and any argument that is exactly equal to the pattern provided
    std::ranges::sort(completions);
    std::erase_if(completions, [&]<typename T>(T const& completion) {
        return completion == pattern; // NOLINT
    });
    std::erase(completions, "__complete__");

    return completions;
}

template <typename ReturnValue, typename ... AdditionalArgs>
std::vector<std::string> FuncTree<ReturnValue, AdditionalArgs...>::findCompletions(std::string_view pattern) {
    std::vector<std::string> completions;
    auto collect = [&](auto const& map, std::string_view const prefix = "") {
        for (auto const& [name, _] : map) {
            if (std::string const full = std::string(prefix) + name; full.starts_with(pattern)) {
                completions.push_back(full);
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

    std::ranges::sort(completions);
    std::erase_if(completions, [seen = std::string{}](auto const& item) mutable {
        bool const duplicate = item == seen;
        seen = item;
        return duplicate;
    });

    return completions;
}

} // namespace Nebulite::Interaction::Execution
#endif // INTERACTION_EXECUTION_FUNCTREEARGUMENTCOMPLETION_TPP
