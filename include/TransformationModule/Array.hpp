/**
 * @file Array.hpp
 * @brief Class for array transformation modules.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_ARRAY_HPP
#define NEBULITE_TRANSFORMATION_MODULE_ARRAY_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Array final : public Data::TransformationModule {
public:
    explicit Array(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool ensureArray(Core::JsonScope* jsonDoc);
    static auto constexpr ensureArrayName = "ensureArray";
    static auto constexpr ensureArrayDesc = "Ensures the current JSON value is an array.\n"
        "If the current value is not an array, it is wrapped into a single-element array.\n"
        "Usage: |ensureArray -> {array}\n";

    static bool at(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr atName = "at";
    static auto constexpr atDesc = "Gets the element at the specified index from the array in the current JSON value.\n"
        "If the index is out of bounds, the transformation fails.\n"
        "Usage: |at <index> -> {value}\n";

    static bool length(Core::JsonScope* jsonDoc);
    static auto constexpr lengthName = "length";
    static auto constexpr lengthDesc = "Gets the length of the array in the current JSON value.\n"
        "Usage: |length -> {number}\n";

    static bool reverse(Core::JsonScope* jsonDoc);
    static auto constexpr reverseName = "reverse";
    static auto constexpr reverseDesc = "Reverses the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |reverse -> {array}\n";

    static bool first(Core::JsonScope* jsonDoc);
    static auto constexpr firstName = "first";
    static auto constexpr firstDesc = "Gets the first element of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |first -> {value}\n";

    static bool last(Core::JsonScope* jsonDoc);
    static auto constexpr lastName = "last";
    static auto constexpr lastDesc = "Gets the last element of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |last -> {value}\n";
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_ARRAY_HPP
