//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Math/FFT.hpp"
#include "Nebulite/Module/Domain/Common/FFT.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/TypeConversion.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

//------------------------------------------
// Update

Constants::Event FFT::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

namespace {

std::string formatComplex(std::complex<double> const& c) {
    if (c.imag() < 0) {
        return std::to_string(c.real()) + "-" + std::to_string(std::abs(c.imag())) + "i";
    }
    return std::to_string(c.real()) + "+" + std::to_string(c.imag()) + "i";
}

template <typename ResultRange>
void printResult(ResultRange const& result, Utility::IO::Capture& capture) {
    std::string str{};
    for (auto& c : result) {
        str += formatComplex(c);
        str += " ";
    }
    if (!str.empty()) {
        str.pop_back(); // Remove the last space
    }
    capture.log.println(str);
}

} // namespace

Constants::Event FFT::fft(std::span<std::string_view const> const& args) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    auto const sampleArgs = args.subspan(1)
        | std::views::transform([](std::string_view const arg) {return Utility::TypeConversion::String::to<double>(arg);})
        | Utility::Ranges::collectOptional;
    if (!sampleArgs) {
        domain.capture.log.println("Invalid argument format for FFT. All arguments must be valid numbers.");
        return Constants::Event::Error;
    }
    auto const result = Math::FFT::fft(sampleArgs.value());
    printResult(result, domain.capture);
    return Constants::Event::Success;
}

Constants::Event FFT::ifft(std::span<std::string_view const> const& args) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() % 2 != 1) { // The first argument is the function name, so we need an even number of additional arguments for real-imaginary pairs
        domain.capture.log.println("Provide a list of real-imaginary pairs for the IFFT.");
        return Constants::Event::Error;
    }
    auto const sampleArgs = args.subspan(1)
        | std::views::chunk(2)
        | std::views::transform([](auto tuple) -> std::optional<std::complex<double>> {
            auto const real = Utility::TypeConversion::String::to<double>(tuple[0]);
            auto const imag = Utility::TypeConversion::String::to<double>(tuple[1]);
            if (!real || !imag) {
                return std::nullopt;
            }
            return std::complex<double>{real.value(), imag.value()};
        })
        | Utility::Ranges::collectOptional;
    if (!sampleArgs) {
        domain.capture.log.println("Invalid argument format for IFFT. All arguments must be valid numbers.");
        return Constants::Event::Error;
    }
    auto const result = Math::FFT::fftInverse(sampleArgs.value());
    printResult(result, domain.capture);
    return Constants::Event::Success;
}

Constants::Event FFT::applyTransferFunction(std::span<std::string_view const> const& args) const {
    auto const numPos = std::ranges::find(args, std::string_view{"--num"});
    auto const denPos = std::ranges::find(args, std::string_view{"--den"});

    if (numPos == args.end() || denPos == args.end()) {
        domain.capture.log.println("Invalid argument format for apply-transfer-function.");
        return Constants::Event::Error;
    }
    if (numPos > denPos) {
        domain.capture.log.println("Invalid argument order for apply-transfer-function. '--num' should come before '--den'.");
        return Constants::Event::Error;
    }

    auto const numIndex = static_cast<std::size_t>(std::distance(args.begin(), numPos));
    auto const denIndex = static_cast<std::size_t>(std::distance(args.begin(), denPos));

    auto const sampleArgs = args.subspan(1, numIndex)
        | std::views::transform([](std::string_view const arg) {return Utility::TypeConversion::String::to<double>(arg);})
        | Utility::Ranges::collectOptional;
    auto const numArgs = args.subspan(numIndex + 1, denIndex)
        | std::views::transform([](std::string_view const arg) {return Utility::TypeConversion::String::to<double>(arg);})
        | Utility::Ranges::collectOptional;
    auto const denArgs = args.subspan(denIndex + 1)
        | std::views::transform([](std::string_view const arg) {return Utility::TypeConversion::String::to<double>(arg);})
        | Utility::Ranges::collectOptional;

    if (!sampleArgs || !numArgs || !denArgs) {
        domain.capture.log.println("Invalid argument format for apply-transfer-function. All arguments must be valid numbers.");
        return Constants::Event::Error;
    }

    auto const result = Math::FFT::applyTransferFunction(sampleArgs.value(), numArgs.value(), denArgs.value());
    printResult(result, domain.capture);
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Common
