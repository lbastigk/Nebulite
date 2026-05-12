/**
 * @file Requirements.hpp
 * @brief Class for requirement transformation functions.
 */

#ifndef MODULE_TRANSFORMATION_REQUIREMENTS_HPP
#define MODULE_TRANSFORMATION_REQUIREMENTS_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Requirements
 * @brief Non-exiting assertion transformation functions
 * @details Instead of exiting, these transformations simply return false, indicating a transformation failure.
 *          Any user-defined error messages are forwarded to the Global capture.
 */
class Requirements final : public Base::TransformationModule {
public:
    explicit Requirements(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Categories

    static auto constexpr requireName = "require";
    static auto constexpr requireDesc = "Requirement transformations that validate JSON values and return false on failure (fallback to default value in get-call, usually 'null' or 0.0).";

    static auto constexpr requireTypeName = "require type";
    static auto constexpr requireTypeDesc = "Requirement transformations that validate JSON value types";

    static auto constexpr requireMatchName = "require match";
    static auto constexpr requireMatchDesc = "Requirement transformations that validate JSON string values with patterns.";

    static auto constexpr requireEqualsName = "require equals";
    static auto constexpr requireEqualsDesc = "Requirement transformations that validate JSON value equality.";

    //------------------------------------------
    // Available Transformations

    static bool requireNonEmpty(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr requireNonEmptyName = "require nonEmpty";
    static auto constexpr requireNonEmptyDesc = "Requires that the current JSON value is non-empty.\n"
        "If the value is empty, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireNonEmpty -> {value,<Returns false if empty>}\n";

    static bool requireEmpty(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr requireEmptyName = "require empty";
    static auto constexpr requireEmptyDesc = "Requires that the current JSON value is empty.\n"
        "If the value is not empty, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireEmpty -> {value,<Returns false if not empty>}\n";

    // [TYPE]

    static bool requireTypeObject(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr requireTypeObjectName = "require type object";
    static auto constexpr requireTypeObjectDesc = "Requires that the current JSON value is of type object.\n"
        "If the value is not an object, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeObject -> {value,<Returns false if not object>}\n";

    static bool requireTypeArray(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr requireTypeArrayName = "require type array";
    static auto constexpr requireTypeArrayDesc = "Requires that the current JSON value is of type array.\n"
        "If the value is not an array, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeArray -> {value,<Returns false if not array>}\n";

    static bool requireTypeBasicValue(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr requireTypeBasicValueName = "require type basicValue";
    static auto constexpr requireTypeBasicValueDesc = "Requires that the current JSON value is a basic value (not object or array or null).\n"
        "If the value is not a basic value, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeValue -> {value,<Returns false if not value>}\n";

    // [MATCH]

    static bool requireMatchRegex(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr requireMatchRegexName = "require match regex";
    static auto constexpr requireMatchesRegexDesc = "Requires that the current JSON string value matches a provided regular expression.\n"
        "If the value does not match the regular expression, the transformation fails.\n"
        "Usage: |require match regex <regex> -> {value,<Returns false if no match>}\n";

    // [EQUALS]

    static bool requireEqualsString(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr requireEqualsStringName = "require equals string";
    static auto constexpr requireEqualsStringDesc = "Requires that the current JSON value is equal to the specified string.\n"
        "If the value is not equal to the specified string, the transformation fails and the program exits.\n"
        "Usage: |require equals string <expected> -> {value,<Returns false if not equal>}\n";

    static bool requireEqualsInt(std::span<std::string const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr requireEqualsIntName = "require equals int";
    static auto constexpr requireEqualsIntDesc = "Requires that the current JSON value is an integer.\n"
        "If the value is not an integer, the transformation fails and the program exits.\n"
        "Usage: |require equals int <expected> -> {value,<Returns false if not equal>}\n";

private:
    /**
     * @brief Prints the message inside its arguments.
     * @details Assumes the first arg is the caller function name.
     * @param args Arguments containing the message parts.
     */
    static void printUserDefinedMessage(std::span<std::string const> const& args);
};
} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_REQUIREMENTS_HPP
