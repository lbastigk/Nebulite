//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <ranges>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Module/Transformation/Generate.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Generate::bindTransformations() {
    bindTransformation(&Generate::iota, iotaName, iotaDesc);
    bindTransformation(&Generate::iotaMod, iotaModName, iotaModDesc);
}

bool Generate::iota(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    if (args.size() != 3) return false;
    auto const start = args.tryGetAs<std::int64_t>(1);
    auto const end = args.tryGetAs<std::int64_t>(2);
    if (!start || !end) return false;
    if (start.value() >= end.value()) {
        jsonDoc.setEmptyArray(rootKey);
        return true;
    }
    jsonDoc.insertArray(rootKey, std::views::iota(start.value(), end.value()));
    return true;
}

bool Generate::iotaMod(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    if (args.size() != 4) return false;
    auto const mod = args.tryGetAs<std::int64_t>(1);
    auto const start = args.tryGetAs<std::int64_t>(2);
    auto const end = args.tryGetAs<std::int64_t>(3);
    if (!mod || !start || !end) return false;
    if (start.value() >= end.value()) {
        jsonDoc.setEmptyArray(rootKey);
        return true;
    }
    jsonDoc.insertArray(
        rootKey,
        std::views::iota(start.value(), end.value())
            | std::views::transform(
                [modValue = mod.value()](auto x) {
                    return x % modValue;
                }
            )
    );
    return true;
}

} // namespace Nebulite::Module::Transformation
