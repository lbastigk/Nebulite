//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <complex>
#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Math/FFT.hpp"
#include "Nebulite/Module/Transformation/FFT.hpp"
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

bool Fft::applyTransferFunctionFrequencyDomain(std::span<std::string_view const> const args, Data::JsonScope& jsonDoc) {
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
        auto const numPos = std::ranges::find(args, std::string_view{"--num"});
        auto const denPos = std::ranges::find(args, std::string_view{"--den"});
        if (numPos == args.end() || denPos == args.end()) {
            return {std::nullopt, std::nullopt};
        }
        if (numPos > denPos) {
            return {std::nullopt, std::nullopt};
        }

        auto const numIndex = static_cast<std::size_t>(std::distance(args.begin(), numPos));
        auto const denIndex = static_cast<std::size_t>(std::distance(args.begin(), denPos));

        auto constexpr tryDoubleConvert = [](std::string_view const arg) -> std::optional<double> {
            return Utility::Convert::Cast::String::to<double>(arg);
        };

        auto const numV = args.subspan(numIndex + 1, denIndex - numIndex - 1)
            | Utility::Ranges::tryTransform(tryDoubleConvert);
        auto const denV = args.subspan(denIndex + 1)
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
