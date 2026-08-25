#ifndef NEBULITE_MODULE_TRANSFORMATION_ARRAY_HPP
#define NEBULITE_MODULE_TRANSFORMATION_ARRAY_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Array
 * @brief Array modification utilities
 */
class Array final : public Base::TransformationModule {
public:
    explicit Array(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    // Pick

    static bool at(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr atName = "at";
    static auto constexpr atDesc = "Gets the element at the specified index from the array in the current JSON value.\n"
        "If the index is out of bounds, the transformation fails.\n"
        "Usage: |at <index> -> {value}\n";

    static bool first(Data::JsonScope& jsonDoc);
    static auto constexpr firstName = "first";
    static auto constexpr firstDesc = "Gets the first element of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |first -> {value}\n";

    static bool last(Data::JsonScope& jsonDoc);
    static auto constexpr lastName = "last";
    static auto constexpr lastDesc = "Gets the last element of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |last -> {value}\n";

    static bool length(Data::JsonScope& jsonDoc);
    static auto constexpr lengthName = "length";
    static auto constexpr lengthDesc = "Gets the length of the array in the current JSON value.\n"
        "Usage: |length -> {number}\n";

    static bool subspan(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr subspanName = "subspan";
    static auto constexpr subspanDesc = "Gets a subarray from the array in the current JSON value.\n"
        "Usage: |subspan <start> [<length>] -> {array}\n";

    // TODO: dedupe, pick, omit

    // Dimensionality

    static bool flatten(Data::JsonScope& jsonDoc);
    static auto constexpr flattenName = "flatten";
    static auto constexpr flattenDesc = "Gets the flattened array of values.\n";

    static bool reverse(Data::JsonScope& jsonDoc);
    static auto constexpr reverseName = "reverse";
    static auto constexpr reverseDesc = "Reverses the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |reverse -> {array}\n";

    static bool batch(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr batchName = "batch";
    static auto constexpr batchDesc = "Batches the array in the current JSON value into subarrays of the specified size.\n"
        "Usage: |batch <size> -> {array}\n";

    static bool batchPadded(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr batchPaddedName = "batchPadded";
    static auto constexpr batchPaddedDesc = "Batches the array in the current JSON value into subarrays of the specified size, padding the last batch with empty objects if necessary.\n"
            "Usage: |batchPadded <size> -> {array}\n";

    static bool stride(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr strideName = "stride";
    static auto constexpr strideDesc = "Strides the array in the current JSON value by the specified size.\n"
        "Usage: |stride <size> -> {array}\n";

    static bool slide(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr slideName = "slide";
    static auto constexpr slideDesc = "Slides the array in the current JSON value by the specified size, generating overlapping windows.\n"
        "Usage: |slide <size> -> {array}\n";

    // Generate

    static bool iota(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr iotaName = "iota";
    static auto constexpr iotaDesc = "Generates an array of values.\n"
        "Usage: |iota <start> <end> -> {array}\n"
        "Generates an array containing the values from start (inclusive) to end (exclusive).\n"
        "If end is not greater than start, the result is an empty array.\n";

    // Other

    static bool ensureArray(Data::JsonScope& jsonDoc);
    static auto constexpr ensureArrayName = "ensureArray";
    static auto constexpr ensureArrayDesc = "Ensures the current JSON value is an array.\n"
        "If the current value is not an array, it is wrapped into a single-element array.\n"
        "Usage: |ensureArray -> {array}\n";

    static bool push(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr pushName = "push";
    static auto constexpr pushDesc = "Pushes a string value to the end of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |push <value> -> {array}\n";

    static bool pushNumber(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr pushNumberName = "pushNumber";
    static auto constexpr pushNumberDesc = "Pushes a numeric value to the end of the array in the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |pushNumber <value> -> {array}\n";

    static bool pad(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr padName = "pad";
    static auto constexpr padDesc = "Pads the array in the current JSON value to the specified length with empty objects.\n"
        "If the current value is larger than the specified length, it is not modified.\n"
        "Usage: |pad <length> -> {array}\n";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_ARRAY_HPP
