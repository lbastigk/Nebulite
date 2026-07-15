//------------------------------------------
// Includes

// Standard library
#include <functional>
#include <optional>
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Module/Transformation/Compare.hpp"
#include "Nebulite/Utility/TypeConversion.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

namespace {
template<typename Op>
bool compare(Data::ScopedKeyView rootKey, std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc, Op op) {
    if (args.size() != 2) return false; // No value provided
    auto const value = jsonDoc.get<double>(rootKey);
    if (!value.has_value()) return false; // Not convertible to double
    auto const compareValue = Utility::TypeConversion::String::to<double>(args[1]);
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

bool Compare::gt(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc) {
    return compare(rootKey, args, jsonDoc, std::greater());
}

bool Compare::geq(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::greater_equal());
}

bool Compare::lt(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::less());
}

bool Compare::leq(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::less_equal());
}

bool Compare::eq(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::equal_to());
}

bool Compare::neq(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    return compare(rootKey, args, jsonDoc, std::not_equal_to());
}

} // namespace Nebulite::Module::Transformation
