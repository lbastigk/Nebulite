/**
* @file Array.hpp
 * @brief Class for sorting transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_SORT_HPP
#define NEBULITE_TRANSFORMATION_MODULE_SORT_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Sort
 * @brief Array modification utilities
 */
class Sort final : public Base::TransformationModule {
public:
    explicit Sort(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Categories

    static auto constexpr sortName = "sort";
    static auto constexpr sortDesc = "Sorting transformation functions";

    //------------------------------------------
    // Available Transformations

    static bool sortCaseSensitive(Data::JsonScope* jsonDoc);
    static auto constexpr sortCaseSensitiveName = "sort case-sensitive";
    static auto constexpr sortCaseSensitiveDesc = "Sorts the array in the current JSON value case-sensitive.\n"
        "Upper case letters first.\n"
        "If the current value is not an array, the transformation fails.\n"
        "Usage: |sort case-sensitive -> {sorted array}\n";

    static bool sortCaseInsensitive(Data::JsonScope* jsonDoc);
    static auto constexpr sortCaseInsensitiveName = "sort case-insensitive";
    static auto constexpr sortCaseInsensitiveDesc = "Sorts the array in the current JSON value case-insensitive.\n"
        "If the current value is not an array, the transformation fails.\n"
        "Usage: |sort alphabetically -> {sorted array}\n";

    static bool sortNumerically(Data::JsonScope* jsonDoc);
    static auto constexpr sortNumericallyName = "sort numerically";
    static auto constexpr sortNumericallyDesc = "Sorts the array in the current JSON value numerically.\n"
        "If the current value is not an array, the transformation fails.\n"
        "Usage: |sort numerically -> {sorted array}\n";

    // TODO: provide a custom sort function, e.g. $(gt({self:|length},{other:|length}))

private:

    template <typename T>
    static void sort(Data::JsonScope* jsonDoc, T const& fallbackValue, std::function<bool(std::pair<T, Data::JSON> const&, std::pair<T, Data::JSON> const&)> comparator) {
        std::vector<std::pair<T, Data::JSON>> values;
        for (auto const idx : std::views::iota(std::size_t{0}, jsonDoc->memberSize(rootKey))) {
            auto const key = rootKey + "[" + std::to_string(idx) + "]";
            values.emplace_back(jsonDoc->get<T>(key).value_or(fallbackValue), jsonDoc->getSubDoc(key));
        }
        std::ranges::sort(values.begin(), values.end(), comparator);
        for (auto [idx, value] : values | std::views::enumerate) {
            auto const key = rootKey + "[" + std::to_string(idx) + "]";
            jsonDoc->setSubDoc(key, value.second);
        }
    }
};

} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_TRANSFORMATION_MODULE_SORT_HPP
