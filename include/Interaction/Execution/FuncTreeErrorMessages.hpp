#ifndef NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_ERROR_MESSAGES_HPP
#define NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_ERROR_MESSAGES_HPP

//------------------------------------------
// Includes
#include <string_view>

//------------------------------------------
// Collection of error messages for FuncTree binding issues

namespace Nebulite::Interaction::Execution {

class BindErrorMessage {
public:
    [[noreturn]] static void invalidFunctionName(std::string_view const& name);

    [[noreturn]] static void missingCategory(std::string_view const& tree, std::string_view const& category, std::string_view const& function);

    [[noreturn]] static void functionShadowsCategory(std::string_view const& function);

    [[noreturn]] static void functionShadowsVariable(std::string_view const& function);

    [[noreturn]] static void functionExistsInInheritedTree(std::string_view const& tree, std::string_view const& inheritedTree, std::string_view const& function);

    [[noreturn]] static void parentCategoryDoesNotExist(std::string_view const& name, std::string_view const& currentCategoryName);

    [[noreturn]] static void categoryExists(std::string_view const& name);

    [[noreturn]] static void functionExists(std::string_view const& tree, std::string_view const& function);

    [[noreturn]] static void variableHasWhitespace(std::string_view const& tree, std::string_view const& variable);

    [[noreturn]] static void variableExists(std::string_view const& tree, std::string_view const& variable);

    [[noreturn]] static void commonFunctionHasWhitespace(std::string_view const& function);

    [[noreturn]] static void categoryShadowsFunction(std::string_view const& category);

    [[noreturn]] static void categoryShadowsVariable(std::string_view const& category);
};

class ExecutionErrorMessage {
public:
    static void unknownVariable(std::string_view const& tree, std::string_view const& variable);

    static void functionNotFound(std::string_view const& tree, std::string_view const& function, std::string_view const& arguments);
};

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_ERROR_MESSAGES_HPP
