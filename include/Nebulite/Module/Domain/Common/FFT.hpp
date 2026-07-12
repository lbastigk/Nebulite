#ifndef NEBULITE_MODULE_DOMAIN_COMMON_FFT_HPP
#define NEBULITE_MODULE_DOMAIN_COMMON_FFT_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
class FFT final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override ;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // TODO: Should be a transformation instead of a domainmodule

    // Ideas:
    // array<number>|fft -> array<object:{real: number, imag: number}> (also allow for complex input)
    // array<object:{real: number, imag: number}>|ifft -> array<object:{real: number, imag: number}>

    // Also add transformation module for complex numbers
    // Maybe a helper class to represent a jsonscope/key as complex number is helpful

    [[nodiscard]] Constants::Event fft(std::span<std::string_view const> const& args) const ;
    static auto constexpr fft_name = "fft";
    static auto constexpr fft_desc = "Print the fft of a given real-number series\n"
        "Usage: fft <sample1> <sample2> ... <sampleN>\n";

    [[nodiscard]] Constants::Event ifft(std::span<std::string_view const> const& args) const ;
    static auto constexpr ifft_name = "ifft";
    static auto constexpr ifft_desc = "Print the inverse fft of a given complex-number series\n"
        "Usage: ifft <real1> <imag1> <real2> <imag2> ... <realN> <imagN>\n";

    [[nodiscard]] Constants::Event applyTransferFunctionFrequencyDomain(std::span<std::string_view const> const& args) const ;
    static auto constexpr applyTransferFunction_name = "apply-transfer-function-frequency-domain";
    static auto constexpr applyTransferFunction_desc = "Print the result of applying a transfer function on the frequency domain to a given real-number series\n"
        "Usage: apply-transfer-function-frequency-domain <sample1> <sample2> ... <sampleN> --num <num-series> --den <den-series>\n"
        "Where the num-series and den-series start withe the highest order coefficient and end with the lowest order coefficient.\n"
        "Example: 4 -1 0 1 -> 4z^-3 - z^-2 + 1\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit FFT(ConstructorParams const& params) : DomainModule(params) {
        bindFunction(&FFT::fft, fft_name, fft_desc);
        bindFunction(&FFT::ifft, ifft_name, ifft_desc);
        bindFunction(&FFT::applyTransferFunctionFrequencyDomain, applyTransferFunction_name, applyTransferFunction_desc);
    }
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_MODULE_DOMAIN_COMMON_FFT_HPP
