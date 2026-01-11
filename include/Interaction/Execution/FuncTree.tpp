/**
 * @file FuncTree.tpp
 * @brief Implementation file for the Basic FuncTree class template functions.
 */

#ifndef NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP
#define NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP

//------------------------------------------
// Includes

// Standard library
#include <cxxabi.h>
#include <memory>
#include <cstdlib>
#include <string>

// Nebulite
#include "Utility/StringHandler.hpp"  // Using StringHandler for easy argument splitting
#include "Interaction/Execution/FuncTree.hpp"
#include "Interaction/Execution/FuncTreeErrorMessages.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Execution {

//------------------------------------------
// Constructor implementation

template <typename returnValue, typename... additionalArgs>
FuncTree<returnValue, additionalArgs...>::FuncTree(std::string_view const& treeName, returnValue const& valDefault, returnValue const& valFunctionNotFound)
    : TreeName(treeName),
      standardReturn{valDefault, valFunctionNotFound} {

    // Add help function for displaying help information
    bindingContainer.functions.emplace(
        helpName,
        FunctionInfo{
            FunctionPtr(
                std::in_place_type<typename SupportedFunctions::Modern::NoAddArgsConstRef>,
                std::function<returnValue(std::span<std::string const> const&)>(
                    [this](std::span<std::string const> const& args) {
                        return this->help(args);
                    }
                )
            ),
            helpDesc
        }
    );

    // Add __complete__ function for command completion
    bindingContainer.functions.emplace(
        "__complete__",
        FunctionInfo{
            FunctionPtr(
                std::in_place_type<typename SupportedFunctions::Modern::NoAddArgsConstRef>,
                std::function<returnValue(std::span<std::string const> const&)>(
                    [this](std::span<std::string const> const& args) {
                        return this->complete(args);
                    }
                )
            ),
            completeDesc
        }
    );
}

//------------------------------------------
// Template comparison

// TODO: Move to private part of FuncTree

template <typename returnValue, typename... additionalArgs>
template <typename T> bool FuncTree<returnValue, additionalArgs...>::isEqual(T const& a, T const& b) {
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

template <typename returnValue, typename... additionalArgs>
void FuncTree<returnValue, additionalArgs...>::bindFunction(FunctionPtr const& func, std::string_view const& name, std::string_view const& helpDescription) {
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
        std::string const functionName = pathStructure.back();
        targetTree->bindFunction(func, functionName, helpDescription);
        return;
    }

    // Check for name conflicts, then bind directly
    if (conflictCheck(name)) {
        bindingContainer.functions.emplace(
            std::string(name),
            FunctionInfo{
                func, helpDescription
            }
        );
    }
}

template <typename returnValue, typename... additionalArgs>
bool FuncTree<returnValue, additionalArgs...>::bindCategory(std::string_view const& name, std::string_view const& helpDescription) {
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

template <typename returnValue, typename... additionalArgs>
void FuncTree<returnValue, additionalArgs...>::bindVariable(bool* varPtr, std::string_view const& name, std::string_view const& helpDescription) {
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

    // Use the variable pointer once to silence "can be made const" warnings
    bool const val = *varPtr;
    *varPtr = false;
    *varPtr = val;
}

//------------------------------------------
// Binding helper functions

template <typename returnValue, typename... additionalArgs>
bool FuncTree<returnValue, additionalArgs...>::conflictCheck(std::string_view const& name) {
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


//------------------------------------------
// Binding helper

namespace {
    template<typename...> inline constexpr bool always_false = false;

    constexpr bool breakBuild = false;

    enum class FunctionShape {
        Unknown,

        // Member shapes
        Member_Legacy_IntChar,
        Member_Legacy_IntConstChar,

        Member_Modern_NoAddArgs,
        Member_Modern_NoAddArgsConstRef,

        Member_Modern_Full,
        Member_Modern_FullConstRef,

        Member_NoArgs,
        Member_NoCmdArgs,

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

    // Extract return, class and parameter list from member-function pointer types
    template <typename T> struct mfp_traits; // primary

    template <typename R, typename C, typename... Ps>
    struct mfp_traits<R(C::*)(Ps...)> {
        using return_t = R;
        using class_t = C;
        using params  = std::tuple<Ps...>;
        static constexpr bool is_const = false;
    };

    template <typename R, typename C, typename... Ps>
    struct mfp_traits<R(C::*)(Ps...) const> {
        using return_t = R;
        using class_t = C;
        using params  = std::tuple<Ps...>;
        static constexpr bool is_const = true;
    };

    // Classify function pointers
    template <typename FunctionPointer, typename returnValue, typename... additionalArgs>
    constexpr FunctionShape classifyFunctionPtr() {
        using M = std::decay_t<FunctionPointer>;
        using Traits = mfp_traits<M>;
        using Params = Traits::params;

        using Span = FuncTree<returnValue, additionalArgs...>::CmdArgs::Span;
        using SpanConstRef = FuncTree<returnValue, additionalArgs...>::CmdArgs::SpanConstRef;

        //------------------------------------------
        if constexpr      (std::is_same_v<Params, std::tuple<int, char**>>)
            return FunctionShape::Member_Legacy_IntChar;
        else if constexpr (std::is_same_v<Params, std::tuple<int, char const**>>)
            return FunctionShape::Member_Legacy_IntConstChar;
        else if constexpr (std::is_same_v<Params, std::tuple<Span>> || std::is_same_v<Params, std::tuple<SpanConstRef>>)
            return FunctionShape::Member_Modern_NoAddArgs;
        else if constexpr (std::is_same_v<Params, std::tuple<SpanConstRef>>)
            return FunctionShape::Member_Modern_NoAddArgsConstRef;
        else if constexpr (std::is_same_v<Params, std::tuple<Span, additionalArgs...>>)
            return FunctionShape::Member_Modern_Full;
        else if constexpr (std::is_same_v<Params, std::tuple<SpanConstRef, additionalArgs...>>)
            return FunctionShape::Member_Modern_FullConstRef;
        else if constexpr (std::is_same_v<Params, std::tuple<additionalArgs...>>)
            return FunctionShape::Member_NoCmdArgs;
        else if constexpr (std::is_same_v<Params, std::tuple<>>)
            return FunctionShape::Member_NoArgs;
        else
            return FunctionShape::Unknown;
    }

    // Classify free/static function pointers
    template <typename FunctionPointer, typename returnValue, typename... additionalArgs>
    constexpr FunctionShape classifyFreeFunction() {
        using F = std::decay_t<FunctionPointer>;
        if constexpr (std::is_same_v<F, returnValue(*)(int, char**)>) {
            return FunctionShape::Free_Legacy_IntChar;
        } else if constexpr (std::is_same_v<F, returnValue(*)(int, char const**)>) {
            return FunctionShape::Free_Legacy_IntConstChar;
        } else if constexpr (std::is_same_v<F, returnValue(*)(typename FuncTree<returnValue, additionalArgs...>::CmdArgs::Span)>) {
            return FunctionShape::Free_Modern_NoAddArgs;
        } else if constexpr (std::is_same_v<F, returnValue(*)(typename FuncTree<returnValue, additionalArgs...>::CmdArgs::SpanConstRef)>) {
            return FunctionShape::Free_Modern_NoAddArgsConstRef;
        } else if constexpr (std::is_same_v<F, returnValue(*)(typename FuncTree<returnValue, additionalArgs...>::CmdArgs::Span, additionalArgs...)>) {
            return FunctionShape::Free_Modern_Full;
        } else if constexpr (std::is_same_v<F, returnValue(*)(typename FuncTree<returnValue, additionalArgs...>::CmdArgs::SpanConstRef, additionalArgs...)>) {
            return FunctionShape::Free_Modern_FullConstRef;
        } else if constexpr (std::is_same_v<F, returnValue(*)()>) {
            return FunctionShape::Free_NoArgs;
        } else if constexpr (std::is_same_v<F, returnValue(*)(additionalArgs...)>) {
            return FunctionShape::Free_NoCmdArgs;
        } else {
            return FunctionShape::Unknown;
        }
    }

    // Unified classifier that dispatches based on pointer category
    template <typename FunctionPointer, typename returnValue, typename... additionalArgs>
    constexpr FunctionShape classifyFunction() {
        if constexpr (std::is_member_function_pointer_v<FunctionPointer>) {
            return classifyFunctionPtr<FunctionPointer, returnValue, additionalArgs...>();
        } else if constexpr (std::is_pointer_v<FunctionPointer> &&
                             std::is_function_v<std::remove_pointer_t<FunctionPointer>>) {
            return classifyFreeFunction<FunctionPointer, returnValue, additionalArgs...>();
        } else {
            static_assert(always_false<FunctionPointer>, "classifyFunction received an unsupported function pointer type.");
            return FunctionShape::Unknown;
        }
    }

} // anonymous namespace

template <typename returnValue, typename... additionalArgs>
template <typename Func>
FuncTree<returnValue, additionalArgs...>::FunctionPtr
FuncTree<returnValue, additionalArgs...>::makeFunctionPtr(Func functionPtr) {
    using This = FuncTree;
    using FunctionPtrT = FunctionPtr;
    using DecayF = std::decay_t<Func>;

    // Helpful compile-time error for pointer-to-member functions passed without an object
    if constexpr (std::is_member_function_pointer_v<DecayF>) {
        static_assert(always_false<Func>,
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
        constexpr FunctionShape shape = classifyFunction<DecayF, returnValue, additionalArgs...>();
        if constexpr (shape == FunctionShape::Free_Legacy_IntChar) {
            return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Legacy::IntChar>,
                                std::function<returnValue(int, char**)>(functionPtr));
        }
        else if constexpr (shape == FunctionShape::Free_Legacy_IntConstChar) {
            return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Legacy::IntConstChar>,
                                std::function<returnValue(int, char const**)>(functionPtr));
        }
        else if constexpr (shape == FunctionShape::Free_Modern_NoAddArgs) {
            return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::NoAddArgs>,
                                std::function<returnValue(typename This::CmdArgs::Span)>(functionPtr));
        }
        else if constexpr (shape == FunctionShape::Free_Modern_NoAddArgsConstRef) {
            return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::NoAddArgsConstRef>,
                                std::function<returnValue(typename This::CmdArgs::SpanConstRef)>(functionPtr));
        }
        else if constexpr (shape == FunctionShape::Free_Modern_Full) {
            return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::Full>,
                                std::function<returnValue(typename This::CmdArgs::Span, additionalArgs...)>(functionPtr));
        }
        else if constexpr (shape == FunctionShape::Free_Modern_FullConstRef) {
            return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::FullConstRef>,
                                std::function<returnValue(typename This::CmdArgs::SpanConstRef, additionalArgs...)>(functionPtr));
        }
        else if constexpr (shape == FunctionShape::Free_NoArgs) {
            return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::NoArgs>,
                                std::function<returnValue()>(functionPtr));
        }
        else if constexpr (shape == FunctionShape::Free_NoCmdArgs) {
            return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::NoCmdArgs>,
                                std::function<returnValue(additionalArgs...)>(functionPtr));
        }
        else {
            static_assert(always_false<Func>, "makeFunctionPtr(func) received an unknown free/static function pointer type");
        }
    }

    // If it's a callable object (lambda/std::function), try to pick a sensible alternative
    if constexpr (std::is_invocable_v<Func, typename This::CmdArgs::Span, additionalArgs...>) {
        return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::Full>,
                            std::function<returnValue(typename This::CmdArgs::Span, additionalArgs...)>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func, typename This::CmdArgs::SpanConstRef, additionalArgs...>) {
        return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::FullConstRef>,
                            std::function<returnValue(typename This::CmdArgs::SpanConstRef, additionalArgs...)>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func, typename This::CmdArgs::Span>) {
        return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::NoAddArgs>,
                            std::function<returnValue(typename This::CmdArgs::Span)>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func, typename This::CmdArgs::SpanConstRef>) {
        return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::NoAddArgsConstRef>,
                            std::function<returnValue(typename This::CmdArgs::SpanConstRef)>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func>) {
        return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::NoArgs>,
                            std::function<returnValue()>(functionPtr));
    }
    else if constexpr (std::is_invocable_v<Func, additionalArgs...>) {
        return FunctionPtrT(std::in_place_type<typename This::SupportedFunctions::Modern::NoCmdArgs>,
                            std::function<returnValue(additionalArgs...)>(functionPtr));
    }
    else {
        static_assert(always_false<Func>, "makeFunctionPtr(func) could not deduce a supported function shape");
        std::abort();
    }
}

// Member-binding helper: obj + member function pointer -> FunctionPtr
template <typename returnValue, typename... additionalArgs>
template <typename Obj, typename MemFunc>
FuncTree<returnValue, additionalArgs...>::FunctionPtr
FuncTree<returnValue, additionalArgs...>::makeFunctionPtr(Obj* objectPtr, MemFunc memberFunctionPtr) {
    using FunctionPtrT = FunctionPtr;
    static_assert(std::is_member_function_pointer_v<MemFunc>, "makeFunctionPtr(Obj, MemFunc) requires a member function pointer");
    using MemDecay = std::decay_t<MemFunc>;

    // Choose appropriate variant and wrap with a lambda that invokes member on objectPtr
    if constexpr (constexpr FunctionShape shape = classifyFunction<MemDecay, returnValue, additionalArgs...>(); shape == FunctionShape::Member_Legacy_IntChar) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Legacy::IntChar>,
            [objectPtr, memberFunctionPtr](int argc, char** argv) {
                return std::invoke(memberFunctionPtr, objectPtr, argc, argv);
        });
    }
    else if constexpr (shape == FunctionShape::Member_Legacy_IntConstChar) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Legacy::IntConstChar>,
            [objectPtr, memberFunctionPtr](int argc, char const** argv) {
                return std::invoke(memberFunctionPtr, objectPtr, argc, argv);
        });
    }
    else if constexpr (shape == FunctionShape::Member_Modern_NoAddArgs) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::NoAddArgs>,
            [objectPtr, memberFunctionPtr](typename CmdArgs::Span args) {
                return std::invoke(memberFunctionPtr, objectPtr, args);
        });
    }
    else if constexpr (shape == FunctionShape::Member_Modern_NoAddArgsConstRef) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::NoAddArgsConstRef>,
            [objectPtr, memberFunctionPtr](typename CmdArgs::SpanConstRef args) {
                return std::invoke(memberFunctionPtr, objectPtr, args);
        });
    }
    else if constexpr (shape == FunctionShape::Member_Modern_Full) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::Full>,
            [objectPtr, memberFunctionPtr](typename CmdArgs::Span args, additionalArgs... rest) {
                return std::invoke(memberFunctionPtr, objectPtr, args, std::forward<additionalArgs>(rest)...);
        });
    }
    else if constexpr (shape == FunctionShape::Member_Modern_FullConstRef) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::FullConstRef>,
            [objectPtr, memberFunctionPtr](typename CmdArgs::SpanConstRef args, additionalArgs... rest) {
                return std::invoke(memberFunctionPtr, objectPtr, args, std::forward<additionalArgs>(rest)...);
        });
    }
    else if constexpr (shape == FunctionShape::Member_NoCmdArgs) {
        return FunctionPtrT(
            std::in_place_type<typename SupportedFunctions::Modern::NoCmdArgs>,
            [objectPtr, memberFunctionPtr](additionalArgs... rest) {
                return std::invoke(memberFunctionPtr, objectPtr, std::forward<additionalArgs>(rest)...);
        });
    }
    else if constexpr (shape == FunctionShape::Member_NoArgs) {
        if constexpr (sizeof...(additionalArgs) == 0) {
            // No extra args in FuncTree -> store as NoArgs
            return FunctionPtrT(
                std::in_place_type<typename SupportedFunctions::Modern::NoArgs>,
                [objectPtr, memberFunctionPtr]() {
                    return std::invoke(memberFunctionPtr, objectPtr);
            });
        } else {
            // FuncTree expects additionalArgs...: wrap the no-arg member into a callable that accepts (and ignores) those args
            return FunctionPtrT(
                std::in_place_type<typename SupportedFunctions::Modern::NoCmdArgs>,
                [objectPtr, memberFunctionPtr](additionalArgs... rest) {
                    (void)sizeof...(rest); // silence unused-warning pattern (optional)
                    return std::invoke(memberFunctionPtr, objectPtr);
            });
        }
    }
    else {
        static_assert(always_false<MemFunc>, "makeFunctionPtr(Obj, MemFunc) received an unsupported member function pointer type");
        return FunctionPtrT{}; // Unreachable
    }
}

//------------------------------------------
// Getter

template <typename returnValue, typename... additionalArgs>
std::vector<std::pair<std::string, std::string_view>> FuncTree<returnValue, additionalArgs...>::getAllFunctions() {
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

template <typename returnValue, typename... additionalArgs>
std::vector<std::pair<std::string, std::string_view>> FuncTree<returnValue, additionalArgs...>::getAllVariables() {
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

template <typename returnValue, typename... additionalArgs>
returnValue FuncTree<returnValue, additionalArgs...>::parseStr(std::string const& cmd, additionalArgs... addArgs) {
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

template <typename returnValue, typename... additionalArgs>
returnValue FuncTree<returnValue, additionalArgs...>::executeFunction(std::string const& name, int argc, char** argv, std::span<std::string const> const& args, additionalArgs... addArgs) {
    // Call preParse function if set
    if (preParse != nullptr) {
        if (returnValue err = preParse(); !isEqual(err, standardReturn.valDefault)) {
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
            if constexpr (std::is_same_v<T, std::function<returnValue(int, char**)>>) {
                // Convert to argc/argv
                return func(argc, argv);
            } else if constexpr (std::is_same_v<T, std::function<returnValue(int, char const**)>>) {
                // Convert char** to char const**
                std::vector<char const*> argv_const(static_cast<size_t>(argc));
                for (size_t i = 0; i < static_cast<size_t>(argc); ++i)
                    argv_const[i] = argv[i];
                return func(argc, argv_const.data());
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
            // Unknown function type
            else {
                static_assert(always_false<T>, "Unknown function signature in FuncTree::executeFunction");
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

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_FUNCTREE_TPP
