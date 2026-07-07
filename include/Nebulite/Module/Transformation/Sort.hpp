#ifndef NEBULITE_MODULE_TRANSFORMATION_SORT_HPP
#define NEBULITE_MODULE_TRANSFORMATION_SORT_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/Execution/FuncTree.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"

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
    explicit Sort(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope&>> const& funcTree)
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

    static bool sortCustom(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr sortCustomName = "sort custom";
    static auto constexpr sortCustomDesc = "Sorts the array in the current JSON value using a custom comparator expression.\n"
        "The comparator function uses the context self for the first element and other for the second element.\n"
        "Provide a sort expression without the evaluation key '$'!"
        "For example: ( gt({self:|length},{other:|length}) )\n"
        "Usage: |sort custom <expression> -> {sorted array}\n";

private:
    /**
     * @brief Custom JSON sort function
     * @tparam T The value to compare
     * @param jsonDoc The scope to sort. Must be an array at scope root!
     * @param fallbackValue Fallback value for the get call
     * @param comparator The custom comparator function, taking both T and the JSON as possible value.
     */
    template <typename T>
    static void arraySort(Data::JsonScope& jsonDoc, T const& fallbackValue, std::function<bool(std::pair<T, Data::JSON>&, std::pair<T, Data::JSON>&)> const& comparator) {
        std::vector<std::pair<T, Data::JSON>> values;
        for (auto const idx : std::views::iota(std::size_t{0}, jsonDoc.memberSize(rootKey))) {
            auto const key = rootKey.addIndex(idx);
            values.emplace_back(jsonDoc.get<T>(key).value_or(fallbackValue), jsonDoc.getSubDoc(key));
        }
        std::ranges::sort(values.begin(), values.end(), comparator);
        for (auto [idx, value] : values | std::views::enumerate) {
            auto const key = rootKey.addIndex(static_cast<std::size_t>(idx));
            jsonDoc.setSubDoc(key, value.second);
        }
    }
};

} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_SORT_HPP
