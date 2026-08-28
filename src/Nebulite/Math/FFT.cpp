//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <complex>
#include <concepts>
#include <cstddef>
#include <numbers>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Math/FFT.hpp"
#include "Nebulite/Utility/Convert/Bits.hpp"
#include "Nebulite/Utility/Generate.hpp"
#include "Nebulite/Utility/Ranges.hpp"

//------------------------------------------
// Concepts

template<class R>
using index_reference_t = decltype(std::declval<R&>()[std::declval<std::ranges::range_size_t<R>>()]);

template<class R>
concept IndexableRange = std::ranges::range<R>
    && requires(R& r, std::ranges::range_size_t<R> i){
        r[i];
    };

template<class R>
concept MutableRange = IndexableRange<R>
    && requires(R& r, std::ranges::range_size_t<R> i) {
        requires std::is_lvalue_reference_v<index_reference_t<R>>;
        requires std::assignable_from<index_reference_t<R>, std::ranges::range_value_t<R>>;
        requires std::swappable<index_reference_t<R>>;
    };

//------------------------------------------
namespace Nebulite::Math::Fft {

namespace {

struct BitReversalPermutation {
    struct Closure : std::ranges::range_adaptor_closure<Closure> {
        std::size_t n;

        Closure(std::size_t rangeSize) : n(rangeSize) {}

        template<MutableRange R>
        static void apply(R& r, std::size_t const n) {
            auto const bitCount = static_cast<std::size_t>(std::bit_width(n - 1));
            assert(r.size() == n);
            assert(std::has_single_bit(n)); // n must be a power of two
            for (auto const i : Utility::Generate::indices(n)) {
                if (auto const b = Utility::Convert::Bits::reverse(i, bitCount); i < b) {
                    std::ranges::swap(r[i], r[b]);
                }
            }
        }

        template<MutableRange R>
        auto operator()(R&& r) const {
            apply(r, n);
            return std::forward<R>(r);
        }
    };

    auto operator()(std::size_t const n) const {
        return Closure(n);
    }
} constexpr bitReversalPermutation;

struct ApplyStages {
    struct Closure : std::ranges::range_adaptor_closure<Closure> {
        std::size_t const n;
        double const fullAngle;

        Closure(std::size_t const rangeSize, double const a) : n(rangeSize), fullAngle(a) {}

        template<IndexableRange I, MutableRange O>
        static void applySingleStage(I& input, O& output, std::complex<double> const stageTwiddle, std::size_t const stageSize, std::size_t const n) {
            auto const halfStageSize = stageSize / 2;

            for (auto const i : Utility::Generate::indices(n) | std::views::stride(stageSize)) {
                std::complex w(1.0);

                for (auto const j : Utility::Generate::indices(halfStageSize)) {
                    auto const u = input[i + j];
                    auto const v = input[i + j + halfStageSize] * w;

                    output[i + j] = u + v;
                    output[i + j + halfStageSize] = u - v;

                    w *= stageTwiddle;
                }
            }
        }

        template<IndexableRange I, MutableRange O>
        static void apply(I& input, O& output, std::size_t const n, double const fullAngle) {
            for (auto const stageSize : Utility::Generate::powersOfTwo(n)) {
                double const ang = fullAngle / static_cast<double>(stageSize);
                std::complex const stageTwiddle(std::cos(ang), std::sin(ang));
                applySingleStage(input, output, stageTwiddle, stageSize, n);
            }
        }

        template<MutableRange R>
        auto operator()(R&& r) const {
            apply(r, r, n, fullAngle);
            return std::forward<R>(r);
        }
    };

    double fullAngle;

    auto operator()(std::size_t const n) const {
        return Closure(n, fullAngle);
    }
};

inline constexpr ApplyStages applyStagesForward{ -2.0 * std::numbers::pi };
inline constexpr ApplyStages applyStagesInverse{ 2.0 * std::numbers::pi };

} // namespace

std::vector<std::complex<double>> fft(std::vector<double> const& data) {
    if (data.empty()) return {};
    auto const n = std::bit_ceil(data.size()); // next power of two
    std::vector<std::complex<double>> a(n); // Initialized to 0.0
    std::copy_n(data.begin(), data.size(), a.begin());
    return a
        | bitReversalPermutation(n)
        | applyStagesForward(n)
        | std::ranges::to<std::vector<std::complex<double>>>();
}

std::vector<std::complex<double>> fftInverse(std::vector<std::complex<double>> const& xValues) {
    auto const n = std::bit_ceil(xValues.size()); // next power of two
    if (n == 0) return {};
    std::vector<std::complex<double>> a = xValues;
    a.resize(n);
    return a
        | bitReversalPermutation(n)
        | applyStagesInverse(n)
        | Utility::Ranges::normalize(n)
        | std::ranges::to<std::vector<std::complex<double>>>();
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

std::complex<double> evalTransfer(double const omega, std::vector<double> const& num, std::vector<double> const& den) {
    std::complex<double> const z = std::exp(std::complex(0.0, -omega));
    std::complex const numSum = evaluatePolynomial(num, z);
    std::complex const denSum = evaluatePolynomial(den, z);
    return numSum / denSum;
}

std::vector<double> applyTransferFunctionFrequencyDomain(std::vector<double> const& data, std::vector<double> const& num, std::vector<double> const& den) {
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

} // namespace Nebulite::Math::Fft
