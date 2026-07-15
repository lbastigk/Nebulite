#ifndef NEBULITE_UTILITY_ARGS_FUNCTREE_TPP
#define NEBULITE_UTILITY_ARGS_FUNCTREE_TPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Utility/Args/CmdArgs.hpp"
#include "Nebulite/Utility/Args/FuncTreeErrorMessages.hpp"
#include "Nebulite/Utility/Args/ShapeClassifier.hpp"
#include "Nebulite/Utility/CompileTimeEvaluate.hpp"
#include "Nebulite/Utility/Coordination/RecursionSecure.hpp"
#include "Nebulite/Utility/FunctionIdentity.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"
#include "Nebulite/Utility/Sort.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_UTILITY_ARGS_FUNCTREE_HPP
#include "Nebulite/Utility/Args/FuncTree.hpp"
#endif // NEBULITE_UTILITY_ARGS_FUNCTREE_HPP

//------------------------------------------
namespace Nebulite::Utility::Args {

//------------------------------------------
// Constructor implementation

template <typename ReturnValue, typename... AdditionalArgs>
FuncTree<ReturnValue, AdditionalArgs...>::FuncTree(std::string_view const treeName, ReturnValue const& valDefault, ReturnValue const& valFunctionNotFound, IO::Capture& captureInstance)
    : TreeName(treeName)
    , capture(captureInstance)
    , standardReturn{valDefault, valFunctionNotFound}
{
    // Add help function for displaying help information
    bindingContainer.functions.emplace(
        helpName,
        FunctionInfo{
            {
                makeFunctionPtr(this, &FuncTree::help),
                FunctionIdentity{this, &FuncTree::help}
            },
        helpDesc
        }
    );

    // Add __complete__ function for command completion
    bindingContainer.functions.emplace(
        "__complete__",
        FunctionInfo{
            {
                makeFunctionPtr(this, &FuncTree::complete),
                FunctionIdentity{this, &FuncTree::complete}
            },
        completeDesc
        }
    );
}

//------------------------------------------
// Binding (Functions, Categories, Variables)

// For generating the Wrapper

// Non-static overload: member-function pointer (non-const)
template <typename ReturnValue, typename... AdditionalArgs>
template <typename R, typename C, typename... Ps>
void FuncTree<ReturnValue, AdditionalArgs...>::bindFunction(
    R (C::*functionPtr)(Ps...),
    std::string_view name,
    std::string_view helpDescription
) {
    auto fp = makeFunctionPtr(functionPtr);
    auto fp_identity = FunctionIdentity(static_cast<const C*>(this), functionPtr);
    bindFunction({fp, fp_identity}, name, helpDescription);
}

// Non-static overload: member-function pointer (const)
template <typename ReturnValue, typename... AdditionalArgs>
template <typename R, typename C, typename... Ps>
void FuncTree<ReturnValue, AdditionalArgs...>::bindFunction(
    R (C::*functionPtr)(Ps...) const,
    std::string_view name,
    std::string_view helpDescription
) {
    auto fp = makeFunctionPtr(functionPtr);
    auto fp_identity = FunctionIdentity(static_cast<const C*>(this), functionPtr);
    bindFunction({fp, fp_identity}, name, helpDescription);
}

// Non-static overload: generic free/static/callable
template <typename ReturnValue, typename... AdditionalArgs>
template <typename Func>
void FuncTree<ReturnValue, AdditionalArgs...>::bindFunction(
    Func functionPtr,
    std::string_view name,
    std::string_view helpDescription
) {
    auto fp = makeFunctionPtr(functionPtr);
    auto fp_identity = FunctionIdentity(functionPtr);
    bindFunction({fp, fp_identity}, name, helpDescription);
}

template <typename ReturnValue, typename... AdditionalArgs>
void FuncTree<ReturnValue, AdditionalArgs...>::bindFunction(WrappedFunction const& func, std::string_view name, std::string_view const helpDescription) {
    // If the name has a whitespace, the function has to be bound to a category hierarchically
    if (name.contains(' ')) {
        auto const pathStructure = StringHandler::split(name, ' ');
        if (pathStructure.size() < 2) {
            BindErrorMessage::invalidFunctionName(capture, name);
        }
        absl::flat_hash_map<std::string, CategoryInfo>* currentCategoryMap = &bindingContainer.categories;
        FuncTree* targetTree = this;
        for (std::size_t idx = 0; idx < pathStructure.size() - 1; idx++) {
            auto const& currentCategoryName = pathStructure[idx];
            if (currentCategoryMap->find(currentCategoryName) == currentCategoryMap->end()) {
                BindErrorMessage::missingCategory(capture, TreeName, currentCategoryName, std::string(name));
            }
            targetTree = (*currentCategoryMap)[currentCategoryName].tree.get();
            currentCategoryMap = &targetTree->bindingContainer.categories;
        }
        auto const& functionName = pathStructure.back();
        targetTree->bindFunction(func, functionName, helpDescription);
        return;
    }

    if (auto searchResult = find(std::string(name)); searchResult.has_value()) {
        bool const shouldReturn = std::visit([&]<typename T>(T& iterator) -> bool {
            using Decayed = std::decay_t<T>;

            if constexpr (std::is_same_v<Decayed, categoryIterator>) {
                BindErrorMessage::functionShadowsCategory(capture, name);
            }
            else if constexpr (std::is_same_v<Decayed, functionIterator>) {
                if (func.identity == iterator->second.function.identity) {
                    return true; // signal: exit outer function
                }

                auto conflictIt = std::find_if(
                    inheritedTrees.begin(), inheritedTrees.end(),
                    [&](auto const& inheritedTree) {
                        return inheritedTree && inheritedTree->hasFunction(name);
                    }
                );

                if (conflictIt != inheritedTrees.end()) {
                    auto const& conflictTree = *conflictIt;
                    BindErrorMessage::functionExistsInInheritedTree(capture, TreeName, conflictTree->TreeName, name);
                }

                BindErrorMessage::functionExists(capture, TreeName, name);
            }
            else if constexpr (std::is_same_v<Decayed, variableIterator>) {
                BindErrorMessage::functionShadowsVariable(capture, name);
            }

            return false;
        }, searchResult.value());

        if (shouldReturn) {
            return; // Same function already exists
        }
    }

    // All checks passed, bind the function
    bindingContainer.functions.emplace(
        std::string(name),
        FunctionInfo{
            func,
            std::string(helpDescription)
        }
    );
}

template <typename ReturnValue, typename... AdditionalArgs>
void FuncTree<ReturnValue, AdditionalArgs...>::bindCategory(std::string_view const name, std::string_view const helpDescription) {
    // Check for shadowing issues
    if (BindingSearchResult const searchResult = find(std::string(name)); searchResult.has_value()) {
        std::visit([&]<typename T>(T&&) {
            using Decayed = std::decay_t<T>;
            if constexpr (std::is_same_v<Decayed, categoryIterator>) {
                BindErrorMessage::categoryExists(capture, name);
            } else if constexpr (std::is_same_v<Decayed, functionIterator>) {
                BindErrorMessage::functionShadowsCategory(capture, name);
            }
        }, searchResult.value());
    }

    // Category traversal
    auto const categoryStructure = StringHandler::split(name, ' ');
    absl::flat_hash_map<std::string, CategoryInfo>* currentCategoryMap = &bindingContainer.categories;
    for (auto const& currentCategoryName : categoryStructure | std::views::take(categoryStructure.size() - 1)) {
        if (currentCategoryMap->find(currentCategoryName) != currentCategoryMap->end()) {
            // Category exists, go deeper
            currentCategoryMap = &(*currentCategoryMap)[currentCategoryName].tree->bindingContainer.categories;
        } else {
            // Category does not exist, throw error
            BindErrorMessage::parentCategoryDoesNotExist(capture, std::string(name), currentCategoryName);
        }
    }
    // Last category, create it, if it doesn't exist yet
    auto const& functionName = categoryStructure.back();
    if (currentCategoryMap->find(functionName) != currentCategoryMap->end()) {
        // Final category we wish to create already exists
        BindErrorMessage::categoryExists(capture, std::string(name));
    }
    // Create category
    (*currentCategoryMap)[functionName] = {
        std::make_unique<FuncTree>(
            functionName,
            standardReturn.valDefault,
            standardReturn.valFunctionNotFound,
            capture
        ),
        std::string(helpDescription)
    };
}

template <typename ReturnValue, typename... AdditionalArgs>
void FuncTree<ReturnValue, AdditionalArgs...>::bindVariable(bool* varPtr, std::string_view name, std::string_view const helpDescription) {
    // Make sure there are no whitespaces in the variable name
    if (name.contains(' ')) {
        BindErrorMessage::variableHasWhitespace(capture, TreeName, name);
    }

    // Make sure the variable isn't bound yet
    if (bindingContainer.variables.find(name) != bindingContainer.variables.end()) {
        BindErrorMessage::variableExists(capture, TreeName, name);
    }

    // Bind the variable
    bindingContainer.variables.emplace(name, VariableInfo{varPtr, std::string(helpDescription)});

    // Use the variable pointer once to silence "can be made const" warnings
    bool const val = *varPtr;
    *varPtr = false;
    *varPtr = val;
}

//------------------------------------------
// Binding helper

template <typename ReturnValue, typename... AdditionalArgs>
template <typename Func>
FuncTree<ReturnValue, AdditionalArgs...>::FunctionPtr
FuncTree<ReturnValue, AdditionalArgs...>::makeFunctionPtr(Func functionPtr) {
    using FunctionPtrT = FunctionPtr;
    using DecayF = std::decay_t<Func>;

    // Helpful compile-time error for pointer-to-member functions passed without an object
    if constexpr (std::is_member_function_pointer_v<DecayF>) {
        static_assert(CompileTimeEvaluate::always_false(),
                      "makeFunctionPtr(func) received a pointer-to-member-function. "
                      "Pass an object + member pointer using makeFunctionPtr(objPtr, &Class::mem) "
                      "or provide a free/static function or callable (lambda/std::function).");
    }

    // If already a FunctionPtr, forward
    if constexpr (std::is_same_v<DecayF, FunctionPtrT>) {
        return functionPtr;
    }

    // If raw function pointer (free/static)
    if constexpr (std::is_pointer_v<DecayF> && std::is_function_v<std::remove_pointer_t<DecayF>>) {
        if constexpr (constexpr ShapeClassifier::FunctionShape shape = ShapeClassifier::classifyFunction<DecayF, ReturnValue, AdditionalArgs...>(); shape == ShapeClassifier::FunctionShape::Free_Legacy_IntChar) {
            return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Legacy::IntChar>,
                                std::function<ReturnValue(int, char**)>(functionPtr));
        }
        else if constexpr (shape == ShapeClassifier::FunctionShape::Free_Legacy_IntConstChar) {
            return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Legacy::IntConstChar>,
                                std::function<ReturnValue(int, char const**)>(functionPtr));
        }
        else if constexpr (shape == ShapeClassifier::FunctionShape::Free_Modern_NoAddArgs) {
            return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::NoAddArgs>,
                                std::function<ReturnValue(CmdArgs::Span)>(functionPtr));
        }
        else if constexpr (shape == ShapeClassifier::FunctionShape::Free_Modern_NoAddArgsConstRef) {
            return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::NoAddArgsConstRef>,
                                std::function<ReturnValue(CmdArgs::SpanConstRef)>(functionPtr));
        }
        else if constexpr (shape == ShapeClassifier::FunctionShape::Free_Modern_Full) {
            return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::Full>,
                                std::function<ReturnValue(CmdArgs::Span, AdditionalArgs...)>(functionPtr));
        }
        else if constexpr (shape == ShapeClassifier::FunctionShape::Free_Modern_FullConstRef) {
            return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::FullConstRef>,
                                std::function<ReturnValue(CmdArgs::SpanConstRef, AdditionalArgs...)>(functionPtr));
        }
        else if constexpr (shape == ShapeClassifier::FunctionShape::Free_NoArgs) {
            return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::NoArgs>,
                                std::function<ReturnValue()>(functionPtr));
        }
        else if constexpr (shape == ShapeClassifier::FunctionShape::Free_NoCmdArgs) {
            return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::NoCmdArgs>,
                                std::function<ReturnValue(AdditionalArgs...)>(functionPtr));
        }
        else {
            static_assert(CompileTimeEvaluate::always_false(), "makeFunctionPtr(func) received an unknown free/static function pointer type");
        }
    }

    // If it's a callable object (lambda/std::function), try to pick a sensible alternative
    if constexpr (std::is_invocable_v<Func, CmdArgs::Span, AdditionalArgs...>) {
        return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::Full>,
                            std::function<ReturnValue(CmdArgs::Span, AdditionalArgs...)>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func, CmdArgs::SpanConstRef, AdditionalArgs...>) {
        return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::FullConstRef>,
                            std::function<ReturnValue(CmdArgs::SpanConstRef, AdditionalArgs...)>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func, CmdArgs::Span>) {
        return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::NoAddArgs>,
                            std::function<ReturnValue(CmdArgs::Span)>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func, CmdArgs::SpanConstRef>) {
        return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::NoAddArgsConstRef>,
                            std::function<ReturnValue(CmdArgs::SpanConstRef)>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func>) {
        return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::NoArgs>,
                            std::function<ReturnValue()>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func, AdditionalArgs...>) {
        return FunctionPtrT(std::in_place_type<typename SupportedFunctions::Modern::NoCmdArgs>,
                            std::function<ReturnValue(AdditionalArgs...)>(functionPtr));
    }
    else {
        static_assert(CompileTimeEvaluate::always_false(), "makeFunctionPtr(func) could not deduce a supported function shape");
        std::unreachable();
    }
}

// Member-binding helper: obj + member function pointer -> FunctionPtr
template <typename ReturnValue, typename... AdditionalArgs>
template <typename Obj, typename MemFunc>
FuncTree<ReturnValue, AdditionalArgs...>::FunctionPtr
FuncTree<ReturnValue, AdditionalArgs...>::makeFunctionPtr(Obj* objectPtr, MemFunc memberFunctionPtr) {
    using FunctionPtrT = FunctionPtr;
    static_assert(std::is_member_function_pointer_v<MemFunc>, "makeFunctionPtr(Obj, MemFunc) requires a member function pointer");
    using MemDecay = std::decay_t<MemFunc>;

    // Choose appropriate variant and wrap with a lambda that invokes member on objectPtr
    if constexpr (constexpr ShapeClassifier::FunctionShape shape = ShapeClassifier::classifyFunction<MemDecay, ReturnValue, AdditionalArgs...>(); shape == ShapeClassifier::FunctionShape::Member_Legacy_IntConstChar) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Legacy::IntConstChar>,
            [objectPtr, memberFunctionPtr](int argc, char const** argv) {
                return std::invoke(memberFunctionPtr, objectPtr, argc, argv);
        });
    }
    else if constexpr (shape == ShapeClassifier::FunctionShape::Member_Modern_NoAddArgs) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::NoAddArgs>,
            [objectPtr, memberFunctionPtr](CmdArgs::Span args) { // NOLINT(readability-redundant-typename)
                return std::invoke(memberFunctionPtr, objectPtr, args);
            }
        );
    }
    else if constexpr (shape == ShapeClassifier::FunctionShape::Member_Modern_NoAddArgsConstRef) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::NoAddArgsConstRef>,
            [objectPtr, memberFunctionPtr](CmdArgs::SpanConstRef args) { // NOLINT(readability-redundant-typename)
                return std::invoke(memberFunctionPtr, objectPtr, args);
            }
        );
    }
    else if constexpr (shape == ShapeClassifier::FunctionShape::Member_Modern_Full) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::Full>,
            [objectPtr, memberFunctionPtr](CmdArgs::Span args, AdditionalArgs... rest) { // NOLINT(readability-redundant-typename)
                return std::invoke(memberFunctionPtr, objectPtr, args, std::forward<AdditionalArgs>(rest)...);
            }
        );
    }
    else if constexpr (shape == ShapeClassifier::FunctionShape::Member_Modern_FullConstRef) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::FullConstRef>,
            [objectPtr, memberFunctionPtr](CmdArgs::SpanConstRef args, AdditionalArgs... rest) { // NOLINT(readability-redundant-typename)
                return std::invoke(memberFunctionPtr, objectPtr, args, std::forward<AdditionalArgs>(rest)...);
            }
        );
    }
    else if constexpr (shape == ShapeClassifier::FunctionShape::Member_NoCmdArgs) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::NoCmdArgs>,
            [objectPtr, memberFunctionPtr](AdditionalArgs... rest) {
                return std::invoke(memberFunctionPtr, objectPtr, std::forward<AdditionalArgs>(rest)...);
            }
        );
    }
    else if constexpr (shape == ShapeClassifier::FunctionShape::Member_NoArgs) {
        if constexpr (sizeof...(AdditionalArgs) == 0) {
            // No extra args in FuncTree -> store as NoArgs
            return FunctionPtrT(
                std::in_place_type<typename SupportedFunctions::Modern::NoArgs>,
                [objectPtr, memberFunctionPtr] {
                    return std::invoke(memberFunctionPtr, objectPtr);
            });
        } else {
            // FuncTree expects AdditionalArgs...: wrap the no-arg member into a callable that accepts (and ignores) those args
            return FunctionPtrT(
                std::in_place_type<typename SupportedFunctions::Modern::NoCmdArgs>,
                [objectPtr, memberFunctionPtr](AdditionalArgs... rest) {
                    (void)sizeof...(rest); // silence unused-warning pattern (optional)
                    return std::invoke(memberFunctionPtr, objectPtr);
            });
        }
    }
    else {
        static_assert(CompileTimeEvaluate::always_false(), "makeFunctionPtr(Obj, MemFunc) received an unsupported member function pointer type");
        std::unreachable();
    }
}

//------------------------------------------
// Getter

template <typename ReturnValue, typename... AdditionalArgs>
std::vector<std::pair<std::string, std::string_view>> FuncTree<ReturnValue, AdditionalArgs...>::getAllFunctions() {
    using Pair = std::pair<std::string, std::string_view>;
    std::vector<Pair> allFunctions;
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

    std::ranges::sort(allFunctions, Sort::caseInsensitiveLess, &Pair::first);
    return allFunctions;
}

template <typename ReturnValue, typename... AdditionalArgs>
std::vector<std::pair<std::string, std::string_view>> FuncTree<ReturnValue, AdditionalArgs...>::getAllVariables() {
    using Pair = std::pair<std::string, std::string_view>;
    std::vector<Pair> allVariables;
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
    std::ranges::sort(allVariables, Sort::caseInsensitiveLess, &Pair::first);
    return allVariables;
}

//------------------------------------------
// Parsing and execution

template <typename ReturnValue, typename... AdditionalArgs>
ReturnValue FuncTree<ReturnValue, AdditionalArgs...>::parseStr(std::string_view cmd, AdditionalArgs... addArgs) {
    StringHandler::rStrip(cmd);
    if (cmd.empty()) {
        return standardReturn.valDefault;
    }
    thread_local Coordination::RecursionSecure<std::vector<std::string_view>, ReturnValue> argsV; // Pre-allocated recursion-safe storage for parsed arguments
    return argsV.use(
        [](std::vector<std::string_view>& args) noexcept {
            args.clear();
        },
        [&](auto& argsView) -> ReturnValue {
            return parseWithPrefix(argsView, cmd, addArgs...);
        }
    );
}

template <typename ReturnValue, typename... AdditionalArgs>
ReturnValue FuncTree<ReturnValue, AdditionalArgs...>::parse(std::span<std::string_view const> const& args, AdditionalArgs... addArgs) {
    auto actualArgs = args.subspan(1); // First arg is caller, remove
    processVariableArguments(actualArgs);
    if (actualArgs.empty()) {
        return standardReturn.valDefault; // Nothing to execute, return standard
    }
    // Call function
    auto funcName = actualArgs.front();
    auto inheritedTree = findInInheritedTrees(funcName);
    if (inheritedTree != nullptr) {
        // Function is in inherited tree, call there
        return inheritedTree->executeFunction(funcName, actualArgs, addArgs...);
    }

    // Not found in inherited trees, execute the function in main tree
    return executeFunction(funcName, actualArgs, addArgs...);
}

template <typename ReturnValue, typename... AdditionalArgs>
ReturnValue FuncTree<ReturnValue, AdditionalArgs...>::parse(std::vector<std::string_view> const& args, AdditionalArgs... addArgs) {
    // Turn into span
    std::span const argsSpan(args.data(), args.size());
    return parse(argsSpan, addArgs...);
}

template <typename ReturnValue, typename... AdditionalArgs>
ReturnValue FuncTree<ReturnValue, AdditionalArgs...>::parse(std::vector<std::string> const& args, AdditionalArgs... addArgs) {
    std::vector<std::string_view> vecView;
    vecView.reserve(args.size());
    std::ranges::transform(
        args,
        std::back_inserter(vecView),
        [](std::string const& str) {
            return std::string_view(str);
        }
    );
    std::span const argsView(vecView);
    return parse(argsView, addArgs...);
}

template <typename ReturnValue, typename ... AdditionalArgs>
ReturnValue FuncTree<ReturnValue, AdditionalArgs...>::parseWithPrefix(std::vector<std::string_view>& existingArgs, std::string_view cmd, AdditionalArgs... addArgs){
    // Optimize to use parseQuotedArguments(existingArgs,cmd) once it supports vec string_view as output!
    // Quote-aware tokenization
    auto const [args, unclosedQuote] = StringHandler::parseQuotedArguments(cmd);
    std::ranges::transform(args, std::back_inserter(existingArgs), [](const std::string& str) { return std::string_view(str); });
    if (unclosedQuote) {
        capture.error.println("Warning: Unclosed quote in command: ", cmd);
    }
    return parse(existingArgs, addArgs...);
}

template <typename ReturnValue, typename... AdditionalArgs>
ReturnValue FuncTree<ReturnValue, AdditionalArgs...>::executeFunction(std::string_view const name, std::span<std::string_view const> const& args, AdditionalArgs... addArgs) {
    // Call preParse function if set
    if (preParse != nullptr) {
        if (ReturnValue err = preParse(); !Math::isEqual(err, standardReturn.valDefault)) {
            return err; // Return error if preParse failed
        }
    }

    // Strip whitespaces of name
    std::string_view function = name;
    StringHandler::strip(function);

    // Find and execute the function
    auto functionPosition = bindingContainer.functions.find(function);
    if (functionPosition != bindingContainer.functions.end()) {
        auto& [functionPtr, description] = functionPosition->second.function;
        return std::visit([&]<typename Func>(Func& func) {
            using T = std::decay_t<Func>;

            // Legacy function types
            if constexpr (std::is_same_v<T, std::function<ReturnValue(int, char const**)>>) {
                // Convert to argc/argv
                std::size_t const argc = args.size();
                std::vector<char const*> argv_vec;
                argv_vec.reserve(argc + 1);
                std::vector<std::string> argsOwned;
                std::transform(
                    args.begin(),
                    args.end(),
                    std::back_inserter(argsOwned),
                    [](std::string_view const str) { return std::string(str); }
                );
                std::transform(
                    argsOwned.begin(),
                    argsOwned.end(),
                    std::back_inserter(argv_vec),
                    [](std::string const& str) { return str.c_str(); }
                );
                argv_vec.push_back(nullptr); // Null-terminate
                return func(static_cast<int>(argc), argv_vec.data());
            }
            // Modern function types
            else if constexpr (std::is_same_v<T, typename SupportedFunctions::Modern::Full> || std::is_same_v<T, typename SupportedFunctions::Modern::FullConstRef>) {
                return func(args, addArgs...);
            }
            else if constexpr (std::is_same_v<T, typename SupportedFunctions::Modern::NoCmdArgs>) {
                return func(addArgs...);
            }
            else if constexpr (std::is_same_v<T, typename SupportedFunctions::Modern::NoAddArgs> || std::is_same_v<T, typename SupportedFunctions::Modern::NoAddArgsConstRef>) {
                return func(args);
            }
            else if constexpr (std::is_same_v<T, typename SupportedFunctions::Modern::NoArgs>) {
                return func();
            }
            // Unsupported function type
            else {
                static_assert(CompileTimeEvaluate::always_false(), "Unsupported function signature in FuncTree::executeFunction. Check if you just need to remove some const/ref qualifiers.");
            }
        }, functionPtr);
    }
    // Find function name in bindingContainer.categories
    if (bindingContainer.categories.find(function) != bindingContainer.categories.end()) {
        return bindingContainer.categories[function].tree->parseStr(StringHandler::recombineArgs(args), addArgs...);
    }

    // Return error if function not found
    auto const arguments = std::ranges::fold_left(args | std::views::enumerate, std::string{}, [](std::string const& acc, auto indexedArg) {
        auto [i, arg] = indexedArg;
        return acc + std::string("argv[") + std::to_string(i) + "] = '" + arg + "'\n";
    });
    ExecutionErrorMessage::functionNotFound(capture, TreeName, function, arguments);
    return standardReturn.valFunctionNotFound;
}

//------------------------------------------
// Argument processing helper

template <typename ReturnValue, typename... AdditionalArgs>
void FuncTree<ReturnValue, AdditionalArgs...>::processVariable(std::string_view varName) {
    bool found = false;
    auto& vars = bindingContainer.variables;
    if (auto const& varIt = vars.find(varName); varIt != vars.end()) {
        if (auto const& varInfo = varIt->second; varInfo.pointer) {
            *varInfo.pointer = true;
            found = true;
        }
    }
    else {
        for (auto const& inheritedTree : inheritedTrees) {
            auto& inheritedVars = inheritedTree->bindingContainer.variables;
            if (auto const& inheritedVarIt = inheritedVars.find(varName); inheritedVarIt != inheritedVars.end()) {
                if (auto const& varInfo = inheritedVarIt->second; varInfo.pointer) {
                    *varInfo.pointer = true;
                    found = true;
                    break;
                }
            }
        }
    }

    // Print error if not found
    if (!found) ExecutionErrorMessage::unknownVariable(capture, TreeName, varName);
}

template <typename ReturnValue, typename... AdditionalArgs>
void FuncTree<ReturnValue, AdditionalArgs...>::processVariableArguments(std::span<std::string_view const>& args) {
    while (!args.empty()) {
        if (auto const& arg = args[0]; arg.length() >= 2 && arg.starts_with("--")) {
            processVariable(arg.substr(2));
            args = args.subspan(1);
        } else {
            // no more vars to parse
            return;
        }
    }
}

template <typename ReturnValue, typename... AdditionalArgs>
std::shared_ptr<FuncTree<ReturnValue, AdditionalArgs...>> FuncTree<ReturnValue, AdditionalArgs...>::findInInheritedTrees(std::string_view funcName) {
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

} // namespace Nebulite::Utility::Args
#include "Nebulite/Utility/Args/FuncTreeArgumentCompletion.tpp" // NOLINT
#endif // NEBULITE_UTILITY_ARGS_FUNCTREE_TPP
