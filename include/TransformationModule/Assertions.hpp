/**
 * @file Assertions.hpp
 * @brief Class for assertion transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP
#define NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Assertions final : public Data::TransformationModule {
public:
    explicit Assertions(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    // TODO: categorizing would be nice:
    //       assert nonEmpty, assert type object/array/value, assert range, assert regex, etc.
    //       implement bindCategory in TransformationModule base class!

    static bool assertNonEmpty(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr assertNonEmptyName = "assertNonEmpty";
    static auto constexpr assertNonEmptyDesc = "Asserts that the current JSON value is non-empty.\n"
        "If the value is empty, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assertNonEmpty -> {value,<Exception thrown if empty>}\n";

    static bool assertTypeObject(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr assertTypeObjectName = "assertTypeObject";
    static auto constexpr assertTypeObjectDesc = "Asserts that the current JSON value is of type object.\n"
        "If the value is not an object, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assertTypeObject -> {value,<Exception thrown if not object>}\n";

    static bool assertTypeArray(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr assertTypeArrayName = "assertTypeArray";
    static auto constexpr assertTypeArrayDesc = "Asserts that the current JSON value is of type array.\n"
        "If the value is not an array, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assertTypeArray -> {value,<Exception thrown if not array>}\n";

    static bool assertTypeBasicValue(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr assertTypeValueName = "assertTypeBasicValue";
    static auto constexpr assertTypeValueDesc = "Asserts that the current JSON value is a basic value (not object or array or null).\n"
        "If the value is not a basic value, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assertTypeValue -> {value,<Exception thrown if not value>}\n";

    // TODO: assertRegex perhaps?

    // TODO: Another class for silent assertions that do not exit the program, but return false on failure
    //       e.g., requiresNonEmpty, requiresType, requiresRange, requiresRegex, etc.

private:
    /**
     * @brief Prints the message inside its arguments.
     * @details Assumes the first arg is the caller function name.
     * @param args Arguments containing the message parts.
     */
    static void printUserDefinedMessage(std::span<std::string const> const& args);
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP
