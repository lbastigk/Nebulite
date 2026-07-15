#ifndef NEBULITE_UTILITY_ARGS_FUNCTREEERRORMESSAGES_HPP
#define NEBULITE_UTILITY_ARGS_FUNCTREEERRORMESSAGES_HPP

//------------------------------------------
// Includes

// Standard library
#include <string_view>

//------------------------------------------
// Forward declarations

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

//------------------------------------------
// Collection of error messages for FuncTree binding issues

namespace Nebulite::Utility::Args {

class BindErrorMessage {
public:
    [[noreturn]] static void invalidFunctionName(IO::Capture& c, std::string_view name);

    [[noreturn]] static void missingCategory(IO::Capture& c, std::string_view tree, std::string_view category, std::string_view function);

    [[noreturn]] static void functionShadowsCategory(IO::Capture& c, std::string_view function);

    [[noreturn]] static void functionShadowsVariable(IO::Capture& c, std::string_view function);

    [[noreturn]] static void functionExistsInInheritedTree(IO::Capture& c, std::string_view tree, std::string_view inheritedTree, std::string_view function);

    [[noreturn]] static void parentCategoryDoesNotExist(IO::Capture& c, std::string_view name, std::string_view currentCategoryName);

    [[noreturn]] static void categoryExists(IO::Capture& c, std::string_view name);

    [[noreturn]] static void functionExists(IO::Capture& c, std::string_view tree, std::string_view function);

    [[noreturn]] static void variableHasWhitespace(IO::Capture& c, std::string_view tree, std::string_view variable);

    [[noreturn]] static void variableExists(IO::Capture& c, std::string_view tree, std::string_view variable);

    [[noreturn]] static void commonFunctionHasWhitespace(IO::Capture& c, std::string_view function);

    [[noreturn]] static void categoryShadowsFunction(IO::Capture& c, std::string_view category);

    [[noreturn]] static void categoryShadowsVariable(IO::Capture& c, std::string_view category);
};

class ExecutionErrorMessage {
public:
    static void unknownVariable(IO::Capture& c, std::string_view tree, std::string_view variable);

    static void functionNotFound(IO::Capture& c, std::string_view tree, std::string_view function, std::string_view arguments);
};

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_FUNCTREEERRORMESSAGES_HPP
