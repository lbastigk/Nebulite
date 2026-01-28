/**
 * @file Casting.hpp
 * @brief Class for casting transformation modules.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_CASTING_HPP
#define NEBULITE_TRANSFORMATION_MODULE_CASTING_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Casting final : public Data::TransformationModule {
public:
    explicit Casting(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool toInt(Core::JsonScope* jsonDoc);
    static auto constexpr toIntName = "toInt";
    static auto constexpr toIntDesc = "Converts the current JSON value to an integer.\n"
        "Never fails, defaults to 0 if the provided value is non-numeric.\n"
        "Usage: |toInt -> {number}\n";

    static bool toString(Core::JsonScope* jsonDoc);
    static auto constexpr toStringName = "toString";
    static auto constexpr toStringDesc = "Converts the current JSON value to a string.\n"
        "Never fails, defaults to an empty string if no conversion is possible.\n"
        "Usage: |toString -> {string}\n";

    static bool toBool(Core::JsonScope* jsonDoc);
    static auto constexpr toBoolName = "toBool";
    static auto constexpr toBoolDesc = "Converts the current JSON value to a boolean.\n"
        "Usage: |toBool -> {bool}\n"
        "'true' values: true, 1, '1', 'true', 'yes', 'on' (case-insensitive)\n"
        "'false' values: false, 0, '0', 'false', 'no', 'off' (case-insensitive)\n"
        "Any other value defaults to false.\n";

    static bool toDouble(Core::JsonScope* jsonDoc);
    static auto constexpr toDoubleName = "toDouble";
    static auto constexpr toDoubleDesc = "Converts the current JSON value to a double.\n"
        "Usage: |toDouble -> {number}\n"
        "Non-numeric values default to 0.0.\n";

    static bool toBoolString(Core::JsonScope* jsonDoc);
    static auto constexpr toBoolStringName = "toBoolString";
    static auto constexpr toBoolStringDesc = "Converts the current JSON value to a boolean string.\n"
        "Usage: |toBoolString -> {string}\n"
        "Either 'true' or 'false'\n";

    // TODO: complicated casting for string to anything via serialization
    //       perhaps named toStringFormat or serialize
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_CASTING_HPP
