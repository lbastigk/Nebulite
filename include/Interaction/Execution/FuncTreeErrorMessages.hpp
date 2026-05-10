#ifndef NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_ERROR_MESSAGES_HPP
#define NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_ERROR_MESSAGES_HPP

//------------------------------------------
// Includes

// Standard library
#include <string_view>

// Nebulite
#include "Utility/IO/Capture.hpp"

//------------------------------------------
// Collection of error messages for FuncTree binding issues

namespace Nebulite::Interaction::Execution {

class BindErrorMessage {
public:
    [[noreturn]] static void invalidFunctionName(Utility::IO::Capture& c, std::string_view const& name);

    [[noreturn]] static void missingCategory(Utility::IO::Capture& c, std::string_view const& tree, std::string_view const& category, std::string_view const& function);

    [[noreturn]] static void functionShadowsCategory(Utility::IO::Capture& c, std::string_view const& function);

    [[noreturn]] static void functionShadowsVariable(Utility::IO::Capture& c, std::string_view const& function);

    [[noreturn]] static void functionExistsInInheritedTree(Utility::IO::Capture& c, std::string_view const& tree, std::string_view const& inheritedTree, std::string_view const& function);

    [[noreturn]] static void parentCategoryDoesNotExist(Utility::IO::Capture& c, std::string_view const& name, std::string_view const& currentCategoryName);

    [[noreturn]] static void categoryExists(Utility::IO::Capture& c, std::string_view const& name);

    [[noreturn]] static void functionExists(Utility::IO::Capture& c, std::string_view const& tree, std::string_view const& function);

    [[noreturn]] static void variableHasWhitespace(Utility::IO::Capture& c, std::string_view const& tree, std::string_view const& variable);

    [[noreturn]] static void variableExists(Utility::IO::Capture& c, std::string_view const& tree, std::string_view const& variable);

    [[noreturn]] static void commonFunctionHasWhitespace(Utility::IO::Capture& c, std::string_view const& function);

    [[noreturn]] static void categoryShadowsFunction(Utility::IO::Capture& c, std::string_view const& category);

    [[noreturn]] static void categoryShadowsVariable(Utility::IO::Capture& c, std::string_view const& category);
};

class ExecutionErrorMessage {
public:
    static void unknownVariable(Utility::IO::Capture& c, std::string_view const& tree, std::string_view const& variable);

    static void functionNotFound(Utility::IO::Capture& c, std::string_view const& tree, std::string_view const& function, std::string_view const& arguments);
};

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_FUNC_TREE_ERROR_MESSAGES_HPP
