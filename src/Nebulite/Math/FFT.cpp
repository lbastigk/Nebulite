//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Math/FFT.hpp"
#include "Nebulite/Utility/Ranges.hpp"

//------------------------------------------
namespace Nebulite::Math {

// TODO: Turn into range-based, pipe-able functions: bitReversalPermutation, applyStages, normalize
namespace {

std::size_t reverseBits(std::size_t input, std::size_t const bitCount) {
    std::size_t result = 0;
    for (std::size_t i = 0; i < bitCount; ++i) {
        result <<= 1u;
        result |= input & 1u;
        input >>= 1u;
    }
    return result;
}

void bitReversalPermutation(auto& a, std::size_t const n) {
    auto const bitCount = static_cast<std::size_t>(std::bit_width(n - 1));
    assert(a.size() == n);
    assert(std::has_single_bit(n)); // n must be a power of two
    for (auto const i : Utility::Ranges::indices(n)) {
        if (auto const b = reverseBits(i, bitCount); i < b) {
            std::swap(a[i], a[b]);
        }
    }
}

void applyStage(auto& a, std::complex<double> const stageTwiddle, std::size_t const stageSize, std::size_t const n) {
    auto const halfStageSize = stageSize / 2;

    for (auto const i : Utility::Ranges::indices(n) | std::views::stride(stageSize)) {
        std::complex w(1.0);

        for (auto const j : Utility::Ranges::indices(halfStageSize)) {
            auto const u = a[i + j];
            auto const v = a[i + j + halfStageSize] * w;

            a[i + j] = u + v;
            a[i + j + halfStageSize] = u - v;

            w *= stageTwiddle;
        }
    }
}

enum class StageType : bool {
    FFT, IFFT
};

template<StageType sign>
constexpr double stageSign() {
    if constexpr (sign == StageType::IFFT) {
        return 2.0 * std::numbers::pi;
    } else if constexpr (sign == StageType::FFT) {
        return -2.0 * std::numbers::pi;
    }
    else {
        std::unreachable();
    }
}

template<StageType sign>
void applyStages(auto& a, auto n) {
    static_assert(sign == StageType::FFT || sign == StageType::IFFT, "sign must be either StageType::FFT or StageType::IFFT");
    for (auto const stageSize : Utility::Ranges::powersOfTwo(n)) {
        double const ang = stageSign<sign>() / static_cast<double>(stageSize);
        std::complex const stageTwiddle(std::cos(ang), std::sin(ang));
        applyStage(a, stageTwiddle, stageSize, n);
    }
}

void normalize(std::vector<std::complex<double>>& a, std::size_t const n) {
    auto const dN = static_cast<double>(n);
    for (auto& v : a) {
        v /= dN;
    }
}

} // namespace

std::vector<std::complex<double>> FFT::fft(std::vector<double> const& data) {
    if (data.empty()) return {};
    auto const n = std::bit_ceil(data.size()); // next power of two
    std::vector<std::complex<double>> a(n); // Initialized to 0.0
    std::copy_n(data.begin(), data.size(), a.begin());
    bitReversalPermutation(a, n);
    applyStages<StageType::FFT>(a, n);
    return a;
}

std::vector<std::complex<double>> FFT::fftInverse(std::vector<std::complex<double>> const& xValues) {
    auto const n = std::bit_ceil(xValues.size()); // next power of two
    if (n == 0) return {};
    std::vector<std::complex<double>> a = xValues;
    a.resize(n);
    bitReversalPermutation(a, n);
    applyStages<StageType::IFFT>(a, n);
    normalize(a, n);
    return a;
}

namespace {
std::complex<double> evaluatePolynomial(std::vector<double> const& coefficients, std::complex<double> const z) {
    std::complex zPow(1.0);
    std::complex result(0.0);
    for (double const c : coefficients | std::views::reverse) { // coefficients hold highest order first
        assert(!std::isnan(c));
        result += c * zPow;
        zPow *= z;
    }
    return result;
}
} // namespace

std::complex<double> FFT::evalTransfer(double const omega, std::vector<double> const& num, std::vector<double> const& den) {
    std::complex<double> const z = std::exp(std::complex(0.0, -omega));
    std::complex const numSum = evaluatePolynomial(num, z);
    std::complex const denSum = evaluatePolynomial(den, z);
    return numSum / denSum;
}

std::vector<double> FFT::applyTransferFunctionFrequencyDomain(std::vector<double> const& data, std::vector<double> const& num, std::vector<double> const& den) {
    if (isZero(den.back())) {
        throw std::domain_error("Denominator has a zero leading coefficient, which is not yet supported.");
    }
    auto xValues = fft(data);
    auto const xSize = static_cast<double>(xValues.size());
    for (auto [k, x] : std::views::enumerate(xValues)) {
        double const omega = 2.0 * std::numbers::pi * static_cast<double>(k) / xSize;
        x *= evalTransfer(omega, num, den);
    }
    return fftInverse(xValues)
        | std::views::transform([](std::complex<double> const& c) {
            assert(!std::isnan(c.real()) && !std::isnan(c.imag()));
            assert(Math::isZero(c.imag())); // Input data and tf is real, so output should be real as well
            return c.real();
        })
        | std::ranges::to<std::vector<double>>();
}

} // namespace Nebulite::Math
