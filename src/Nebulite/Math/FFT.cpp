//------------------------------------------
// Includes

// Standard library
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
#include "Nebulite/Utility/Generate.hpp"
#include "Nebulite/Utility/Ranges.hpp"

//------------------------------------------
namespace Nebulite::Math::Fft {

namespace {

struct InplaceApplyStages {
    struct Closure : std::ranges::range_adaptor_closure<Closure> {
        std::size_t const n;
        double const fullAngle;

        Closure(std::size_t const rangeSize, double const a) : n(rangeSize), fullAngle(a) {}

        template<Utility::Ranges::MutableRange R>
        static void applySingleStage(R& r, std::complex<double> const stageTwiddle, std::size_t const stageSize, std::size_t const n) {
            assert(r.size() >= n);

            // Since we verified the size, we can access directly without bounds checking. This is a performance optimization.
            auto* data = std::ranges::data(r);

            // Perform the FFT butterfly operation for the current stage
            auto const halfStageSize = stageSize / 2;
            for (auto const i : Utility::Generate::indices(n) | std::views::stride(stageSize)) {
                std::complex w(1.0); // Preallocation of ws is about 100% slower than just computing them on the fly for large arrays, so we do that instead

                auto* lo = data + i;
                auto* hi = data + i + halfStageSize;

                for (auto const j : Utility::Generate::indices(halfStageSize)) {
                    assert(i + j + halfStageSize < n);
                    auto const u = lo[j];
                    auto const v = hi[j] * w;
                    lo[j] = u + v;
                    hi[j] = u - v;
                    w *= stageTwiddle;
                }
            }
        }

        template<Utility::Ranges::MutableRange R>
        static void apply(R& r, std::size_t const n, double const fullAngle) {
            for (auto const stageSize : Utility::Generate::powersOfTwo(n)) {
                double const ang = fullAngle / static_cast<double>(stageSize);
                std::complex const stageTwiddle(std::cos(ang), std::sin(ang));
                applySingleStage(r, stageTwiddle, stageSize, n);
            }
        }

        template<Utility::Ranges::MutableRange R>
        auto operator()(R&& r) const {
            apply(r, n, fullAngle);
            return std::forward<R>(r);
        }
    };

    double fullAngle;

    auto operator()(std::size_t const n) const {
        return Closure(n, fullAngle);
    }
};

inline constexpr InplaceApplyStages inplaceApplyStagesForward{ -2.0 * std::numbers::pi };
inline constexpr InplaceApplyStages inplaceApplyStagesInverse{ 2.0 * std::numbers::pi };

} // namespace

std::vector<std::complex<double>> fft(std::vector<double> const& data) {
    if (data.empty()) return {};
    auto const n = std::bit_ceil(data.size()); // next power of two
    return data
        | Utility::Ranges::copyBitReversalPermutation(n, std::complex{0.0})
        | inplaceApplyStagesForward(n);
}

std::vector<std::complex<double>> fftInverse(std::vector<std::complex<double>> const& xValues) {
    auto const n = std::bit_ceil(xValues.size()); // next power of two
    if (n == 0) return {};

    return xValues
        | Utility::Ranges::copyBitReversalPermutation(n, std::complex{0.0})
        | inplaceApplyStagesInverse(n)
        | Utility::Ranges::normalize(n);
}

namespace {
template <std::ranges::input_range R>
std::complex<double> evaluatePolynomial(R&& coefficients, std::complex<double> const z) {
    std::complex zPow(1.0);
    std::complex result(0.0);
    for (double const c : std::forward<R>(coefficients) | std::views::reverse) { // coefficients hold highest order first
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
