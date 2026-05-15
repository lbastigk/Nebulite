/**
 * @file Array.hpp
 * @brief Class for array transformation functions.
 */

#ifndef MODULE_TRANSFORMATION_ARRAY_HPP
#define MODULE_TRANSFORMATION_ARRAY_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Array
 * @brief Array modification utilities
 */
class Array final : public Base::TransformationModule {
public:
    explicit Array(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    // Ranges

    static bool at(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr atName = "at";
    static auto constexpr atDesc = "Gets the element at the specified index from the array in the current JSON value.\n"
        "If the index is out of bounds, the transformation fails.\n"
        "Usage: |at <index> -> {value}\n";

    static bool length(Data::JsonScope* jsonDoc);
    static auto constexpr lengthName = "length";
    static auto constexpr lengthDesc = "Gets the length of the array in the current JSON value.\n"
        "Usage: |length -> {number}\n";

    static bool first(Data::JsonScope* jsonDoc);
    static auto constexpr firstName = "first";
    static auto constexpr firstDesc = "Gets the first element of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |first -> {value}\n";

    static bool last(Data::JsonScope* jsonDoc);
    static auto constexpr lastName = "last";
    static auto constexpr lastDesc = "Gets the last element of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |last -> {value}\n";

    static bool subspan(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr subspanName = "subspan";
    static auto constexpr subspanDesc = "Gets a subarray from the array in the current JSON value.\n"
        "Usage: |subspan <start> [<length>] -> {array}\n";

    // TODO: dedupe, pick, omit

    // Modify

    static bool reverse(Data::JsonScope* jsonDoc);
    static auto constexpr reverseName = "reverse";
    static auto constexpr reverseDesc = "Reverses the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |reverse -> {array}\n";

    static bool ensureArray(Data::JsonScope* jsonDoc);
    static auto constexpr ensureArrayName = "ensureArray";
    static auto constexpr ensureArrayDesc = "Ensures the current JSON value is an array.\n"
        "If the current value is not an array, it is wrapped into a single-element array.\n"
        "Usage: |ensureArray -> {array}\n";

    static bool push(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr pushName = "push";
    static auto constexpr pushDesc = "Pushes a string value to the end of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |push <value> -> {array}\n";

    static bool pushNumber(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr pushNumberName = "pushNumber";
    static auto constexpr pushNumberDesc = "Pushes a numeric value to the end of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |pushNumber <value> -> {array}\n";

    static bool enumerate(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr enumerateName = "enumerate";
    static auto constexpr enumerateDesc = "Enumerates the array in the current JSON value.\n"
        "Usage: |enumerate <indexKey> -> {array}\n"
        "Where indexKey is the key of each array element to populate with the index of the element in the array.\n";

    static bool iota(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr iotaName = "iota";
    static auto constexpr iotaDesc = "Generates an array of values.\n"
        "Usage: |iota <start> <end> -> {array}\n"
        "Generates an array containing the values from start (inclusive) to end (exclusive).\n";
};
} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_ARRAY_HPP
