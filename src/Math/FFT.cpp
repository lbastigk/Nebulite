//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <bit>
#include <cmath>
#include <complex>
#include <cstddef>
#include <numbers>
#include <ranges>
#include <vector>

// Nebulite
#include "Nebulite/Math/FFT.hpp"
#include "Nebulite/Utility/Ranges.hpp"

//------------------------------------------
namespace Nebulite::Math {

namespace {

std::size_t reverseBits(std::size_t input, std::size_t const bitCount) {
    std::size_t result = 0;
    for (std::size_t i = 0; i < bitCount; ++i) {
        result <<= 1;
        result |= input & 1;
        input >>= 1;
    }
    return result;
}

void applyStage(auto& a, std::complex<double> const stageTwiddle, std::size_t const stageSize, std::size_t const N) {
    auto const halfStageSize = stageSize / 2;

    for (auto const i : Utility::Ranges::indices(N) | std::views::stride(stageSize)) {
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

} // namespace

std::vector<std::complex<double>> FFT::fft(std::vector<double> const& data) {
    std::size_t const n = data.size();
    if (n == 0) return {};
    auto const N = std::bit_ceil(n); // next power of two
    auto const bitCount = static_cast<std::size_t>(std::bit_width(N - 1));

    // Initialize the complex array with zero-padding
    std::vector<std::complex<double>> a(N); // Initialized to 0.0
    std::copy_n(data.begin(), n, a.begin());

    // bit-reversal permutation for proper ordering of input data (required for cooley-turkey)
    for (auto const i : Utility::Ranges::indices(N)) {
        if (auto const b = reverseBits(i, bitCount); i < b) {
            std::swap(a[i], a[b]);
        }
    }

    // FFT stages
    for (auto const stageSize : Utility::Ranges::powersOfTwo(N)) {
        double const ang = -2.0 * std::numbers::pi / static_cast<double>(stageSize);
        std::complex const stageTwiddle(std::cos(ang), std::sin(ang));
        applyStage(a, stageTwiddle, stageSize, N);
    }

    return a;
}

std::vector<std::complex<double>> FFT::fftInverse(std::vector<std::complex<double>> const& X) {
    auto const N = std::bit_ceil(X.size()); // next power of two
    auto const bitCount = static_cast<std::size_t>(std::bit_width(N - 1));
    if (N == 0) return {};

    std::vector<std::complex<double>> a = X;
    a.resize(N);

    // bit-reversal permutation for proper ordering of input data (required for cooley-turkey)
    for (auto const i : Utility::Ranges::indices(N)) {
        if (auto const b = reverseBits(i, bitCount); i < b) {
            std::swap(a[i], a[b]);
        }
    }

    // IFFT stages (note sign flip)
    for (auto const stageSize : Utility::Ranges::powersOfTwo(N)) {
        double const ang = 2.0 * std::numbers::pi / static_cast<double>(stageSize);
        std::complex const stageTwiddle(std::cos(ang), std::sin(ang));
        applyStage(a, stageTwiddle, stageSize, N);
    }

    // normalize
    auto const dN = static_cast<double>(N);
    for (auto& v : a)
        v /= dN;

    return a;
}

namespace {
std::complex<double> evaluatePolynomial(std::vector<double> const& coefficients, std::complex<double> const z) {
    std::complex zPow(1.0);
    std::complex result(0.0);
    for (double const c : coefficients | std::views::reverse) { // coefficients hold highest order first
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

std::vector<double> FFT::applyTransferFunction(std::vector<double> const& data, std::vector<double> const& num, std::vector<double> const& den) {
    auto X = fft(data);
    auto const xSize = static_cast<double>(X.size());
    for (auto [k, x] : std::views::enumerate(X)) {
        double const omega = 2.0 * std::numbers::pi * static_cast<double>(k) / xSize;
        x *= evalTransfer(omega, num, den);
    }
    auto const y = fftInverse(X);
    std::vector<double> dataOut(y.size());
    std::ranges::transform(y, dataOut.begin(), [](std::complex<double> const& c) {
        return c.real();
    });
    return dataOut;
}

} // namespace Nebulite::Math
