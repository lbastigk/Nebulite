/**
 * @file Assertions.hpp
 * @brief Class for assertion transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP
#define NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Assertions
 * @brief Various assertion transformations that validate JSON values and throw exceptions on failure.
 */
class Assertions final : public Base::TransformationModule {
public:
    explicit Assertions(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Categories

    static auto constexpr assertName = "assert";
    static auto constexpr assertDesc = "Assertion transformations that validate JSON values and throw exceptions on failure.";

    static auto constexpr assertTypeName = "assert type";
    static auto constexpr assertTypeDesc = "Assertion transformations that validate JSON value types and throw exceptions on failure.";

    static auto constexpr assertMatchName = "assert match";
    static auto constexpr assertMatchDesc = "Assertion transformation that validate JSON string values with patterns and throw exceptions on failure.";

    static auto constexpr assertEqualsName = "assert equals";
    static auto constexpr assertEqualsDesc = "Assertion transformations that validate JSON value equality and throw exceptions on failure.";

    //------------------------------------------
    // Available Transformations

    static bool assertNonEmpty(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr assertNonEmptyName = "assert nonEmpty";
    static auto constexpr assertNonEmptyDesc = "Asserts that the current JSON value is non-empty.\n"
        "If the value is empty, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assert nonEmpty -> {value,<Exception thrown if empty>}\n";

    // [TYPE]

    static bool assertTypeObject(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr assertTypeObjectName = "assert type object";
    static auto constexpr assertTypeObjectDesc = "Asserts that the current JSON value is of type object.\n"
        "If the value is not an object, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assert type object -> {value,<Exception thrown if not object>}\n";

    static bool assertTypeArray(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr assertTypeArrayName = "assert type array";
    static auto constexpr assertTypeArrayDesc = "Asserts that the current JSON value is of type array.\n"
        "If the value is not an array, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assert type array -> {value,<Exception thrown if not array>}\n";

    static bool assertTypeBasicValue(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr assertTypeBasicValueName = "assert type basicValue";
    static auto constexpr assertTypeBasicValueDesc = "Asserts that the current JSON value is a basic value (not object or array or null).\n"
        "If the value is not a basic value, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assert type value -> {value,<Exception thrown if not value>}\n";

    // [MATCH]

    static bool assertMatchRegex(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr assertMatchesRegexName = "assert match regex";
    static auto constexpr assertMatchesRegexDesc = "Asserts that the current JSON string value matches a provided regular expression.\n"
        "If the value does not match the regular expression, the transformation fails and the program exits\n"
        "Usage: |assert match regex <regex> -> {value,<Exception thrown if not matches>}\n";

    // [EQUALS]

    static bool assertEqualsString(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr assertEqualsStringName = "assert equals string";
    static auto constexpr assertEqualsStringDesc = "Asserts that the current JSON value is equal to the specified string.\n"
        "If the value is not equal to the specified string, the transformation fails and the program exits\n"
        "Usage: |assert equals string <expected> -> {value,<Exception thrown if not equal>}\n";

    static bool assertEqualsInt(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr assertEqualsIntName = "assert equals int";
    static auto constexpr assertEqualsIntDesc = "Asserts that the current JSON value is an integer.\n"
        "If the value is not an integer, the transformation fails and the program exits\n"
        "Usage: |assert equals int <expected> -> {value,<Exception thrown if not equal>}\n";

private:
    /**
     * @brief Prints the message inside its arguments.
     * @details Assumes the first arg is the caller function name.
     * @param args Arguments containing the message parts.
     */
    static void printUserDefinedMessage(std::span<std::string const> const& args);
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP
