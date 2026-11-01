//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint> // NOLINT
#include <limits>
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
    bindTransformation(&Generate::iotaStep, iotaStepName, iotaStepDesc);
    bindTransformation(&Generate::repeat, repeatName, repeatDesc);
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
    if (!mod || !start || !end || mod.value() == 0) return false;
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

bool Generate::iotaStep(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    if (args.size() != 4) return false;
    auto const start = args.tryGetAs<std::int64_t>(1);
    auto const step = args.tryGetAs<std::int64_t>(2);
    auto const end = args.tryGetAs<std::int64_t>(3);
    if (!start || !step || !end) return false;

    auto const s = start.value();
    auto const d = step.value();
    auto const e = end.value();

    // Zero step is invalid; the lowest step can't be negated and the highest start can't be shifted by one
    if (d == 0 || d == std::numeric_limits<std::int64_t>::min()) return false;
    if (d < 0 && s == std::numeric_limits<std::int64_t>::max()) return false;

    if ((d > 0 && s >= e) || (d < 0 && s <= e)) {
        jsonDoc.setEmptyArray(rootKey);
        return true;
    }

    if (d > 0) {
        jsonDoc.insertArray(rootKey, std::views::iota(s, e) | std::views::stride(d));
    } else {
        // Descending: walk [e+1, s+1) backwards, starting at s
        jsonDoc.insertArray(rootKey, std::views::iota(e + 1, s + 1) | std::views::reverse | std::views::stride(-d));
    }
    return true;
}

bool Generate::repeat(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    if (args.size() < 3) return false;
    auto const count = args.tryGetAs<std::size_t>(1);
    if (!count) return false;

    jsonDoc.setEmptyArray(rootKey);
    std::size_t index = 0;
    for (std::size_t round = 0; round < count.value(); ++round) {
        for (std::size_t i = 2; i < args.size(); ++i) {
            auto const key = rootKey.addIndex(index++);
            if (auto const asInt = args.tryGetAs<std::int64_t>(i)) {
                jsonDoc.set(key, asInt.value());
            } else if (auto const asDouble = args.tryGetAs<double>(i)) {
                jsonDoc.set(key, asDouble.value());
            } else {
                jsonDoc.set(key, args.at(i));
            }
        }
    }
    return true;
}

} // namespace Nebulite::Module::Transformation
