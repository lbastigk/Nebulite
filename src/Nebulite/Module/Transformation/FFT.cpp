//------------------------------------------
// Includes

// Standard library
#include <complex>
#include <optional>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Math/FFT.hpp"
#include "Nebulite/Module/Transformation/FFT.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Convert/Cast.hpp"
#include "Nebulite/Utility/Ranges.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

void Fft::bindTransformations() {
    bindTransformation(&Fft::applyFft, applyFftName, applyFftDesc);
    bindTransformation(&Fft::applyIfft, applyIfftName, applyIfftDesc);
    bindTransformation(&Fft::applyTransferFunctionFrequencyDomain, applyTransferFunctionName, applyTransferFunctionDesc);
}

bool Fft::applyFft(Data::JsonScope& jsonDoc) {
    auto const samples = jsonDoc.arrayKeys(rootKey)
        | std::views::transform([&jsonDoc](auto const& key) -> std::optional<double> {
            auto value = jsonDoc.get<double>(key);
            if (!value) {
                return std::nullopt;
            }
            return value.value();
        })
        | Utility::Ranges::collectOptional;

    if (!samples) {
        return false;
    }
    if (samples.value().empty()) {
        return true;
    }
    auto const result = Math::Fft::fft(samples.value());
    jsonDoc.setArray(rootKey, result);
    return true;
}

bool Fft::applyIfft(Data::JsonScope& jsonDoc) {
    auto const samples = jsonDoc.arrayKeys(rootKey)
        | std::views::transform([&jsonDoc](auto const& key) -> std::optional<std::complex<double>> {
            // Try to retrieve value as real value first (simplest to handle), if not, try to retrieve as complex value
            if (auto value = jsonDoc.get<double>(key); value) {
                return std::complex<double>(value.value(), 0.0);
            }
            return jsonDoc.getComplex(key); // Potentially nullopt
        })
        | Utility::Ranges::collectOptional;

    if (!samples) {
        return false;
    }
    auto const result = Math::Fft::fftInverse(samples.value());
    jsonDoc.setArray(rootKey, result);
    return true;
}

bool Fft::applyTransferFunctionFrequencyDomain(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    auto const samples = jsonDoc.arrayKeys(rootKey)
        | std::views::transform([&jsonDoc](auto const& key) -> std::optional<double> {
            // Try to retrieve value as real value first (simplest to handle), if not, try to retrieve as complex value
            if (auto value = jsonDoc.get<double>(key); value) {
                return value.value();
            }
            return std::nullopt;
        })
        | Utility::Ranges::collectOptional;

    if (!samples) {
        return false;
    }

    // Get num/den polynomial
    using OptVec = std::optional<std::vector<double>>;
    auto [num, den] = [&args] -> std::pair<OptVec, OptVec> {
        auto constexpr tryDoubleConvert = [](std::string_view const arg) -> std::optional<double> {
            return Utility::Convert::Cast::String::to<double>(arg);
        };

        auto const numV = args.getNamedSpannedArgument<"--num">()
            | Utility::Ranges::tryTransform(tryDoubleConvert);
        auto const denV = args.getNamedSpannedArgument<"--den">()
            | Utility::Ranges::tryTransform(tryDoubleConvert);

        return std::make_pair(numV, denV);
    }();

    if (!samples || !num || !den) {
        return false;
    }

    auto const result = Math::Fft::applyTransferFunctionFrequencyDomain(samples.value(), num.value(), den.value());
    jsonDoc.setArray(rootKey, result);
    return true;
}

} // namespace Nebulite::Module::Transformation
