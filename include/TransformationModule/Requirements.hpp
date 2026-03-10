/**
 * @file Requirements.hpp
 * @brief Class for requirement transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_REQUIREMENTS_HPP
#define NEBULITE_TRANSFORMATION_MODULE_REQUIREMENTS_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Requirements final : public Data::TransformationModule {
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

    //------------------------------------------
    // Available Transformations

    static bool requireNonEmpty(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr requireNonEmptyName = "require nonEmpty";
    static auto constexpr requireNonEmptyDesc = "Requires that the current JSON value is non-empty.\n"
        "If the value is empty, the transformation fails\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireNonEmpty -> {value,<Returns false if empty>}\n";

    static bool requireTypeObject(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr requireTypeObjectName = "require type object";
    static auto constexpr requireTypeObjectDesc = "Requires that the current JSON value is of type object.\n"
        "If the value is not an object, the transformation fails\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeObject -> {value,<Returns false if not object>}\n";

    static bool requireTypeArray(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr requireTypeArrayName = "require type array";
    static auto constexpr requireTypeArrayDesc = "Requires that the current JSON value is of type array.\n"
        "If the value is not an array, the transformation fails\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeArray -> {value,<Returns false if not array>}\n";

    static bool requireTypeBasicValue(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr requireTypeBasicValueName = "require type basicValue";
    static auto constexpr requireTypeBasicValueDesc = "Requires that the current JSON value is a basic value (not object or array or null).\n"
        "If the value is not a basic value, the transformation fails\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeValue -> {value,<Returns false if not value>}\n";

private:
    /**
     * @brief Prints the message inside its arguments.
     * @details Assumes the first arg is the caller function name.
     * @param args Arguments containing the message parts.
     */
    static void printUserDefinedMessage(std::span<std::string const> const& args);
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_REQUIREMENTS_HPP
