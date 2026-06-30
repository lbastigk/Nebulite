#ifndef INTERACTION_EXECUTION_FUNCTREEERRORMESSAGES_HPP
#define INTERACTION_EXECUTION_FUNCTREEERRORMESSAGES_HPP

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
    [[noreturn]] static void invalidFunctionName(Utility::IO::Capture& c, std::string_view name);

    [[noreturn]] static void missingCategory(Utility::IO::Capture& c, std::string_view tree, std::string_view category, std::string_view function);

    [[noreturn]] static void functionShadowsCategory(Utility::IO::Capture& c, std::string_view function);

    [[noreturn]] static void functionShadowsVariable(Utility::IO::Capture& c, std::string_view function);

    [[noreturn]] static void functionExistsInInheritedTree(Utility::IO::Capture& c, std::string_view tree, std::string_view inheritedTree, std::string_view function);

    [[noreturn]] static void parentCategoryDoesNotExist(Utility::IO::Capture& c, std::string_view name, std::string_view currentCategoryName);

    [[noreturn]] static void categoryExists(Utility::IO::Capture& c, std::string_view name);

    [[noreturn]] static void functionExists(Utility::IO::Capture& c, std::string_view tree, std::string_view function);

    [[noreturn]] static void variableHasWhitespace(Utility::IO::Capture& c, std::string_view tree, std::string_view variable);

    [[noreturn]] static void variableExists(Utility::IO::Capture& c, std::string_view tree, std::string_view variable);

    [[noreturn]] static void commonFunctionHasWhitespace(Utility::IO::Capture& c, std::string_view function);

    [[noreturn]] static void categoryShadowsFunction(Utility::IO::Capture& c, std::string_view category);

    [[noreturn]] static void categoryShadowsVariable(Utility::IO::Capture& c, std::string_view category);
};

class ExecutionErrorMessage {
public:
    static void unknownVariable(Utility::IO::Capture& c, std::string_view tree, std::string_view variable);

    static void functionNotFound(Utility::IO::Capture& c, std::string_view tree, std::string_view function, std::string_view arguments);
};

} // namespace Nebulite::Interaction::Execution
#endif // INTERACTION_EXECUTION_FUNCTREEERRORMESSAGES_HPP
