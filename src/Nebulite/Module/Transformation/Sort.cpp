//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <functional>
#include <ranges>
#include <string>
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
#include "Nebulite/Utility/Promise.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/Sort.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

namespace {
/**
 * @brief Custom JSON sort function
 * @tparam T The value to compare
 * @param jsonDoc The scope to sort. Must be an array at scope root!
 * @param fallbackValue Fallback value for the get call
 * @param comparator The custom comparator function, taking both T and the JSON as possible value.
 */
template <typename T>
void arraySort(Data::JsonScope& jsonDoc, T const& fallbackValue, std::function<bool(std::pair<T, Data::Json>&, std::pair<T, Data::Json>&)> const& comparator) {
    std::vector<std::pair<T, Data::Json>> values;
    for (auto const idx : std::views::iota(std::size_t{0}, jsonDoc.memberSize(Base::TransformationModule::rootKey))) {
        auto const key = Base::TransformationModule::rootKey.addIndex(idx);
        values.emplace_back(jsonDoc.get<T>(key).value_or(fallbackValue), jsonDoc.getSubDoc(key));
    }
    std::ranges::sort(values, comparator);
    for (auto [idx, value] : values | Utility::Ranges::enumerate) {
        auto const key = Base::TransformationModule::rootKey.addIndex(idx);
        jsonDoc.setSubDoc(key, value.second);
    }
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
    arraySort<std::string>(jsonDoc, "", [](auto const& a, auto const& b) {
        return Utility::Sort::caseSensitiveLess(a.first, b.first);
    });
    return true;
}

bool Sort::sortCaseInsensitive(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    arraySort<std::string>(jsonDoc, "", [](auto const& a, auto const& b) {
        return Utility::Sort::caseInsensitiveLess(a.first, b.first);
    });
    return true;
}

bool Sort::sortNumerically(Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    arraySort<double>(jsonDoc,0, [](auto const& a, auto const& b) {
        return a.first < b.first;
    });
    return true;
}

bool Sort::sortCustom(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    if (jsonDoc.memberType(rootKey) != Data::KeyType::array) return false; // Not an array, cannot sort
    if (args.size() < 2) return false;
    Interaction::Logic::Expression const expression('$' + args.recombineSubspan(1));
    if (!expression.isReturnableAsBool()) return false;
    arraySort<bool>(jsonDoc, false, [&](auto& a, auto& b) {
        auto& slf = a.second.shareManagedScope("");
        auto& otr = b.second.shareManagedScope("");
        Interaction::ContextScope const ctxScope{
            {
                .self = slf,
                .other = otr,
                .global = slf,
            },
        };
        auto constexpr promise = Utility::Promise<Utility::PromiseType::FunctionVerified, &Interaction::Logic::Expression::isReturnableAsBool>{};
        return expression.evalAsBool(ctxScope, promise);
    });
    return true;
}

} // namespace Nebulite::Module::Transformation
