#ifndef NEBULITE_UTILITY_ARGS_FUNCTREEERRORMESSAGES_HPP
#define NEBULITE_UTILITY_ARGS_FUNCTREEERRORMESSAGES_HPP

//------------------------------------------
// Includes

// Standard library
#include <string_view>

//------------------------------------------
// Forward declarations

namespace Nebulite::Utility::Io {
class Capture;
} // namespace Nebulite::Utility::Io

//------------------------------------------
// Collection of error messages for FuncTree binding issues

namespace Nebulite::Utility::Args {

class BindErrorMessage {
public:
    [[noreturn]] static void invalidFunctionName(Io::Capture& c, std::string_view name);

    [[noreturn]] static void missingCategory(Io::Capture& c, std::string_view tree, std::string_view category, std::string_view function);

    [[noreturn]] static void functionShadowsCategory(Io::Capture& c, std::string_view function);

    [[noreturn]] static void functionShadowsVariable(Io::Capture& c, std::string_view function);

    [[noreturn]] static void functionExistsInInheritedTree(Io::Capture& c, std::string_view tree, std::string_view inheritedTree, std::string_view function);

    [[noreturn]] static void parentCategoryDoesNotExist(Io::Capture& c, std::string_view name, std::string_view currentCategoryName);

    [[noreturn]] static void categoryExists(Io::Capture& c, std::string_view name);

    [[noreturn]] static void functionExists(Io::Capture& c, std::string_view tree, std::string_view function);

    [[noreturn]] static void variableHasWhitespace(Io::Capture& c, std::string_view tree, std::string_view variable);

    [[noreturn]] static void variableExists(Io::Capture& c, std::string_view tree, std::string_view variable);

    [[noreturn]] static void commonFunctionHasWhitespace(Io::Capture& c, std::string_view function);

    [[noreturn]] static void categoryShadowsFunction(Io::Capture& c, std::string_view category);

    [[noreturn]] static void categoryShadowsVariable(Io::Capture& c, std::string_view category);
};

class ExecutionErrorMessage {
public:
    static void unknownVariable(Io::Capture& c, std::string_view tree, std::string_view variable);

    static void functionNotFound(Io::Capture& c, std::string_view tree, std::string_view function, std::string_view arguments);
};

} // namespace Nebulite::Utility::Args
#endif // NEBULITE_UTILITY_ARGS_FUNCTREEERRORMESSAGES_HPP
