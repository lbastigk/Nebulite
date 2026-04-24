#include "Math/FFT.hpp"

#include <algorithm>
#include <ranges>

// TODO: Large refactor + testing required!

namespace Nebulite::Math {

std::vector<std::complex<double>> FFT::fft(std::vector<double> const& data) {
    size_t const n = data.size();
    if (n == 0) return {};

    // next power of two
    size_t N = 1;
    while (N < n) N <<= 1;

    std::vector<std::complex<double>> a(N);

    for (size_t i = 0; i < n; ++i)
        // NOLINTNEXTLINE
        a[i] = static_cast<double>(data[i]);
    for (size_t i = n; i < N; ++i)
        a[i] = 0.0;

    // bit-reversal permutation
    size_t b = 0;
    for (size_t i = 1; i < N; ++i) {
        size_t bit = N >> 1;
        while (b & bit) {
            b ^= bit;
            bit >>= 1;
        }
        b |= bit;

        if (i < b)
            std::swap(a[i], a[b]);
    }

    // FFT stages
    for (size_t len = 2; len <= N; len <<= 1) {
        double const ang = -2.0 * M_PI / static_cast<double>(len);
        std::complex const wLen(std::cos(ang), std::sin(ang));

        for (size_t i = 0; i < N; i += len) {
            std::complex w(1.0);

            for (size_t j = 0; j < len / 2; ++j) {
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
    size_t const N = X.size();
    if (N == 0) return {};

    std::vector<std::complex<double>> a = X;

    // bit-reversal permutation
    size_t b = 0;
    for (size_t i = 1; i < N; ++i) {
        size_t bit = N >> 1;
        while (b & bit) {
            b ^= bit;
            bit >>= 1;
        }
        b |= bit;

        if (i < b)
            std::swap(a[i], a[b]);
    }

    // IFFT stages (note sign flip)
    for (size_t len = 2; len <= N; len <<= 1) {
        double const ang = 2.0 * M_PI / static_cast<double>(len);
        std::complex const wLen(std::cos(ang), std::sin(ang));

        for (size_t i = 0; i < N; i += len) {
            std::complex w(1.0);

            for (size_t j = 0; j < len / 2; ++j) {
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

std::complex<double> FFT::evalTransfer(double const& omega, std::vector<double> const& num, std::vector<double> const& den) {
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

    if (std::abs(denSum) < 1e-12)
        return 0.0;

    return numSum / denSum;
}

std::vector<double> FFT::applyTransferFunction(std::vector<double> const& data, std::vector<double> const& num, std::vector<double> const& den) {
    auto X = fft(data);
    for (auto [k, x] : std::views::zip(std::views::iota(size_t{0}), X)) {
        double omega = 2.0 * M_PI * static_cast<double>(k) / static_cast<double>(X.size());
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
