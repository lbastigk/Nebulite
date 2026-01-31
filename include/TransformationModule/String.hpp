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

    static bool toUpper(Core::JsonScope* jsonDoc);
    static auto constexpr toUpperName = "toUpper";
    static auto constexpr toUpperDesc = "Converts the current JSON string value to uppercase.\n"
        "Usage: |toUpper -> {string}\n";

    static bool toLower(Core::JsonScope* jsonDoc);
    static auto constexpr toLowerName = "toLower";
    static auto constexpr toLowerDesc = "Converts the current JSON string value to lowercase.\n"
        "Usage: |toLower -> {string}\n";

    static bool strip(Core::JsonScope* jsonDoc);
    static auto constexpr trimName = "strip";
    static auto constexpr trimDesc = "Strips whitespace from both ends of the current JSON string value.\n"
        "Usage: |trim -> {string}\n";

    // TODO: lstrip, rstrip

    static bool substring(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr substringName = "substring";
    static auto constexpr substringDesc = "Extracts a substring from the current JSON string value.\n"
        "Usage: |substring {start} {length} -> {string}\n"
        "{start}: Starting index (0-based)\n"
        "{length}: Length of the substring\n";

    static bool replace(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr replaceName = "replace";
    static auto constexpr replaceDesc = "Replaces all occurrences of a target substring with a replacement substring in the current JSON string value.\n"
        "Usage: |replace {target} {replacement} -> {string}\n"
        "{target}: Substring to be replaced\n"
        "{replacement}: Substring to replace with\n";

    // TODO: Add comparison functions like strequals, strcontains, strstartswith, strendswith, strcountappearance
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_STRING_HPP
