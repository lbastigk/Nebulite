/**
 * @file String.hpp
 * @brief Class for string transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_STRING_HPP
#define NEBULITE_TRANSFORMATION_MODULE_STRING_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class String final : public Data::TransformationModule {
public:
    explicit String(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    /*
    static bool toUpper(Core::JsonScope* jsonDoc);
    static auto constexpr toUpperName = "toUpper";
    static auto constexpr toUpperDesc = "Converts the current JSON string value to uppercase.\n"
        "Usage: |toUpper -> {string}\n";

    static bool toLower(Core::JsonScope* jsonDoc);
    static auto constexpr toLowerName = "toLower";
    static auto constexpr toLowerDesc = "Converts the current JSON string value to lowercase.\n"
        "Usage: |toLower -> {string}\n";

    static bool trim(Core::JsonScope* jsonDoc);
    static auto constexpr trimName = "trim";
    static auto constexpr trimDesc = "Trims whitespace from both ends of the current JSON string value.\n"
        "Usage: |trim -> {string}\n";
    */

    // TODO: trim, upper, lower, substring, replace, etc.

    // TODO: Add comparison functions like strequals, strcontains, strstartswith, strendswith, strcountappearance
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_STRING_HPP
