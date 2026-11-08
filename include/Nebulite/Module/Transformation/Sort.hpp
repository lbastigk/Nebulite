#ifndef NEBULITE_MODULE_TRANSFORMATION_SORT_HPP
#define NEBULITE_MODULE_TRANSFORMATION_SORT_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Sort
 * @brief Array modification utilities
 */
class Sort final : public Base::TransformationModule {
public:
    explicit Sort(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Categories

    static auto constexpr sortName = "sort";
    static auto constexpr sortDesc = "Sorting transformation functions";

    //------------------------------------------
    // Available Transformations

    static bool sortCaseSensitive(Data::JsonScope& jsonDoc);
    static auto constexpr sortCaseSensitiveName = "sort case-sensitive";
    static auto constexpr sortCaseSensitiveDesc = "Sorts the array in the current JSON value case-sensitive.\n"
        "Upper case letters first.\n"
        "If the current value is not an array, the transformation fails.\n"
        "Usage: |sort case-sensitive -> {sorted array}\n";

    static bool sortCaseInsensitive(Data::JsonScope& jsonDoc);
    static auto constexpr sortCaseInsensitiveName = "sort case-insensitive";
    static auto constexpr sortCaseInsensitiveDesc = "Sorts the array in the current JSON value case-insensitive.\n"
        "If the current value is not an array, the transformation fails.\n"
        "Usage: |sort alphabetically -> {sorted array}\n";

    static bool sortNumerically(Data::JsonScope& jsonDoc);
    static auto constexpr sortNumericallyName = "sort numerically";
    static auto constexpr sortNumericallyDesc = "Sorts the array in the current JSON value numerically.\n"
        "If the current value is not an array, the transformation fails.\n"
        "Usage: |sort numerically -> {sorted array}\n";

    static bool sortCustom(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr sortCustomName = "sort custom";
    static auto constexpr sortCustomDesc = "Sorts the array in the current JSON value using a custom comparator expression.\n"
        "The comparator function uses the context self for the first element and other for the second element.\n"
        "Provide a sort expression without the evaluation key '$'!"
        "For example: ( gt({self:|length},{other:|length}) )\n"
        "Usage: |sort custom <expression> -> {sorted array}\n";
};

} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_SORT_HPP
