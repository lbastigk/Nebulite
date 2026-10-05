//------------------------------------------
// Includes

// Standard library
#include <functional>
#include <optional>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Module/Transformation/Compare.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Convert/Cast.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

namespace {
template<typename Op>
bool compare(Data::ScopedKeyView rootKey, Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc, Op op) {
    if (args.size() != 2) return false; // No value provided
    auto const value = jsonDoc.get<double>(rootKey);
    if (!value.has_value()) return false; // Not convertible to double
    auto const compareValue = Utility::Convert::Cast::String::to<double>(args[1]);
    if (!compareValue.has_value()) return false; // Not convertible to double
    jsonDoc.set(rootKey, op(value.value(), compareValue.value()));
    return true;
}
} // namespace

void Compare::bindTransformations(){
    bindTransformation(&Compare::gt, gtName, gtDesc);
    bindTransformation(&Compare::geq, geqName, geqDesc);
    bindTransformation(&Compare::lt, ltName, ltDesc);
    bindTransformation(&Compare::leq, leqName, leqDesc);
    bindTransformation(&Compare::eq, eqName, eqDesc);
    bindTransformation(&Compare::neq, neqName, neqDesc);
}

bool Compare::eq(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::equal_to());
}

bool Compare::neq(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::not_equal_to());
}

bool Compare::gt(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return compare(rootKey, args, jsonDoc, std::greater());
}

bool Compare::geq(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::greater_equal());
}

bool Compare::lt(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::less());
}

bool Compare::leq(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::less_equal());
}

} // namespace Nebulite::Module::Transformation
