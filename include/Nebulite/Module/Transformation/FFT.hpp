#ifndef NEBULITE_MODULE_TRANSFORMATION_FFT_HPP
#define NEBULITE_MODULE_TRANSFORMATION_FFT_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::FFT
 * @brief Various FFT transformations
 */
class FFT final : public Base::TransformationModule {
public:
    explicit FFT(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    [[nodiscard]] static bool fft(Data::JsonScope& jsonDoc);
    static auto constexpr fft_name = "fft";
    static auto constexpr fft_desc = "Stores the fft of a given real-number series as complex numbers\n"
        "Usage: fft <sample1> <sample2> ... <sampleN>\n";

    [[nodiscard]] static bool ifft(Data::JsonScope& jsonDoc);
    static auto constexpr ifft_name = "ifft";
    static auto constexpr ifft_desc = "Stores the inverse fft of a given complex-number or real-number series (mixable)\n"
        "Usage: ifft\n";

    [[nodiscard]] static bool applyTransferFunctionFrequencyDomain(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr applyTransferFunction_name = "applyTfDomainF";
    static auto constexpr applyTransferFunction_desc = "Stores the result of applying a transfer function on the frequency domain to a given real-number series\n"
        "Usage: applyTfFDomain --num <num-series> --den <den-series>\n"
        "Where the num-series and den-series start withe the highest order coefficient and end with the lowest order coefficient.\n"
        "Example: 4 -1 0 1 -> 4z^-3 - z^-2 + 1\n";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_FFT_HPP
