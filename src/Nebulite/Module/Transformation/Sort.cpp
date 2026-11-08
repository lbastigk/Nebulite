//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Module/Transformation/Sort.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Sort.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

namespace {
/**
 * @brief Custom JSON sort function
 * @tparam T The value to compare
 * @param jsonDoc The scope to sort. Must be an array at scope root!
 * @param comparator Comparison between valid keys.
 */
template <typename T, typename Comparator>
void arraySort(Data::JsonScope& jsonDoc, Comparator const& comparator) {
    std::vector<std::pair<std::optional<T>, Data::Json>> values;
    for (auto const idx : std::views::iota(std::size_t{0}, jsonDoc.memberSize(Base::TransformationModule::rootKey))) {
        auto const key = Base::TransformationModule::rootKey.addIndex(idx);
        auto const retrieved = jsonDoc.get<T>(key);
        std::optional<T> sortKey;
        if (retrieved) {
            if constexpr (std::is_same_v<T, double>) {
                if (!std::isnan(*retrieved)) sortKey = *retrieved;
            } else {
                sortKey = *retrieved;
            }
        }
        values.emplace_back(std::move(sortKey), jsonDoc.getSubDoc(key));
    }
    std::stable_sort(values.begin(), values.end(), [&](auto const& a, auto const& b) {
        if (!a.first) return false;
        if (!b.first) return true;
        return comparator(*a.first, *b.first);
    });
    auto const documents = values | std::views::transform([]([[clang::lifetimebound]] auto const& value) -> Data::Json const& {
        return value.second;
    });
    jsonDoc.insertArray(Base::TransformationModule::rootKey, documents);
}
} // anonymous namespace

void Sort::bindTransformations(){
    bindCategory(sortName, sortDesc);
    bindTransformation(&Sort::sortCaseSensitive,sortCaseSensitiveName, sortCaseSensitiveDesc);
    bindTransformation(&Sort::sortCaseInsensitive, sortCaseInsensitiveName, sortCaseInsensitiveDesc);
    bindTransformation(&Sort::sortNumerically, sortNumericallyName, sortNumericallyDesc);
    bindTransformation(&Sort::sortCustom, sortCustomName, sortCustomDesc);
}

bool Sort::sortCaseSensitive(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    arraySort<std::string>(jsonDoc, Utility::Sort::caseSensitiveLess);
    return true;
}

bool Sort::sortCaseInsensitive(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    arraySort<std::string>(jsonDoc, Utility::Sort::caseInsensitiveLess);
    return true;
}

bool Sort::sortNumerically(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    arraySort<double>(jsonDoc, [](double const a, double const b) {
        return a < b;
    });
    return true;
}

bool Sort::sortCustom(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    // Validate input
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    if (args.size() < 2) return false;
    Interaction::Logic::Expression const expression('$' + args.recombineSubspan(1));
    if (!expression.isReturnableAsBool() || !expression.isCompiled()) return false;

    // Get values
    std::size_t const size = jsonDoc.memberSize(rootKey);
    std::vector<Data::Json> values;
    values.reserve(size);
    for (std::size_t idx = 0; idx < size; ++idx) {
        values.push_back(jsonDoc.getSubDoc(rootKey.addIndex(idx)));
    }

    // Comparator function
    auto compare = [&](std::size_t a, std::size_t b) {
        auto& slf = values[a].shareManagedScope("");
        auto& otr = values[b].shareManagedScope("");
        Interaction::ContextScope const ctxScope{
            {
                .self = slf,
                .other = otr,
                .global = slf,
            },
        };
        return expression.tryEvalAsBool(ctxScope);
    };

    auto const order = Utility::Sort::stableIndexOrder(size, compare);
    if (!order) return false;
    auto const documents = *order | std::views::transform([&values](std::size_t const index) -> Data::Json const& {
        return values[index];
    });
    jsonDoc.insertArray(rootKey, documents);
    return true;
}

} // namespace Nebulite::Module::Transformation
