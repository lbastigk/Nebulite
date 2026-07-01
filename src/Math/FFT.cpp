//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <math.h> // NOLINT
#include <ranges>
#include <vector>

// Nebulite
#include "Math/FFT.hpp"

//------------------------------------------
namespace Nebulite::Math {

// TODO: Large refactor + testing required!

std::vector<std::complex<double>> FFT::fft(std::vector<double> const& data) {
    std::size_t const n = data.size();
    if (n == 0) return {};

    // next power of two
    std::size_t N = 1;
    while (N < n) N <<= 1;

    std::vector<std::complex<double>> a(N);

    for (std::size_t i = 0; i < n; ++i)
        // NOLINTNEXTLINE
        a[i] = static_cast<double>(data[i]);
    for (std::size_t i = n; i < N; ++i)
        a[i] = 0.0;

    // bit-reversal permutation
    std::size_t b = 0;
    for (std::size_t i = 1; i < N; ++i) {
        std::size_t bit = N >> 1;
        while (static_cast<bool>(b & bit)) {
            b ^= bit;
            bit >>= 1;
        }
        b |= bit;

        if (i < b)
            std::swap(a[i], a[b]);
    }

    // FFT stages
    for (std::size_t len = 2; len <= N; len <<= 1) {
        double const ang = -2.0 * M_PI / static_cast<double>(len);
        std::complex const wLen(std::cos(ang), std::sin(ang));

        for (std::size_t i = 0; i < N; i += len) {
            std::complex w(1.0);

            for (std::size_t j = 0; j < len / 2; ++j) {
                auto u = a[i + j];
                auto v = a[i + j + len / 2] * w;

                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;

                w *= wLen;
            }
        }
    }

    return a;
}

std::vector<std::complex<double>> FFT::fftInverse(std::vector<std::complex<double>> const& X) {
    std::size_t const N = X.size();
    if (N == 0) return {};

    std::vector<std::complex<double>> a = X;

    // bit-reversal permutation
    std::size_t b = 0;
    for (std::size_t i = 1; i < N; ++i) {
        std::size_t bit = N >> 1;
        while (static_cast<bool>(b & bit)) {
            b ^= bit;
            bit >>= 1;
        }
        b |= bit;

        if (i < b)
            std::swap(a[i], a[b]);
    }

    // IFFT stages (note sign flip)
    for (std::size_t len = 2; len <= N; len <<= 1) {
        double const ang = 2.0 * M_PI / static_cast<double>(len);
        std::complex const wLen(std::cos(ang), std::sin(ang));

        for (std::size_t i = 0; i < N; i += len) {
            std::complex w(1.0);

            for (std::size_t j = 0; j < len / 2; ++j) {
                auto u = a[i + j];
                auto v = a[i + j + len / 2] * w;

                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;

                w *= wLen;
            }
        }
    }

    // normalize
    for (auto& v : a)
        v /= static_cast<double>(N);

    return a;
}

std::complex<double> FFT::evalTransfer(double const omega, std::vector<double> const& num, std::vector<double> const& den) {
    std::complex<double> const z = std::exp(std::complex(0.0, -omega));
    std::complex numSum(0.0);
    std::complex denSum(0.0);
    std::complex zPow(1.0);

    // numerator
    for (double const b : num) {
        numSum += b * zPow;
        zPow *= z;
    }

    zPow = 1.0;

    // denominator
    for (double const a : den) {
        denSum += a * zPow;
        zPow *= z;
    }

    static auto constexpr eps = 1e-12;
    if (std::abs(denSum) < eps)
        return 0.0;

    return numSum / denSum;
}

std::vector<double> FFT::applyTransferFunction(std::vector<double> const& data, std::vector<double> const& num, std::vector<double> const& den) {
    auto X = fft(data);
    for (auto [k, x] : std::views::enumerate(X)) {
        double const omega = 2.0 * M_PI * static_cast<double>(k) / static_cast<double>(X.size());
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
