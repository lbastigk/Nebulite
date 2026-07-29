//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <complex>
#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/Logic/Formatter.hpp"
#include "Nebulite/Module/Transformation/Complex.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

namespace {

struct ComplexView {
    std::string_view re;
    std::string_view im;
};

std::optional<ComplexView> parseComplexNumberString(std::string_view s){
    if (!s.ends_with("i")) return std::nullopt; // Not a complex number
    s.remove_suffix(1); // remove trailing 'i'
    std::size_t split = std::string_view::npos;

    // There should be only one + or - in the string not after an 'e' or 'E'
    for (std::size_t i = 1; i < s.size(); ++i) {
        if ((s[i] == '+' || s[i] == '-') && s[i - 1] != 'e' && s[i - 1] != 'E') {
            if (split != std::string_view::npos) {
                // More than one top-level +/- separator.
                return std::nullopt; // Not a complex number.
            }
            split = i;
        }
    }
    std::string_view imag = s;
    std::string_view real;

    if (split != std::string_view::npos) {
        real = s.substr(0, split);
        imag = s.substr(split);
    }
    return ComplexView{.re=real, .im=imag};
}

std::string complexFormatter(std::complex<double> const& c, Interaction::Logic::Formatter const& fmt) {
    auto const fmtReal = fmt.format(c.real());
    auto const fmtImag = fmt.format(c.imag());
    return fmtReal + (fmtImag[0] == '-' ? "" : "+") + fmtImag + "i";
}

} // namespace

void Complex::bindTransformations() {
    bindTransformation(&Complex::numberToComplex, numberToComplexName, numberToComplexDesc);
    bindTransformation(&Complex::complexAbs, complexAbsName, complexAbsDesc);
    bindTransformation(&Complex::complexArg, complexArgName, complexArgDesc);
    bindTransformation(&Complex::complexToString, complexToStringName, complexToStringDesc);
    bindTransformation(&Complex::formatComplexNumberString, formatComplexNumberStringName,formatComplexNumberStringDesc);
}

bool Complex::numberToComplex(Data::JsonScope& jsonDoc){
    auto num = jsonDoc.get<double>(rootKey);
    if (!num) {
        return false;
    }
    jsonDoc.setComplex(rootKey, std::complex{num.value(), 0.0});
    return true;
}

bool Complex::complexAbs(Data::JsonScope& jsonDoc){
    auto complexNum = jsonDoc.get<double>(rootKey);
    if (!complexNum) {
        return false;
    }
    jsonDoc.set(rootKey, std::abs(complexNum.value()));
    return true;
}

bool Complex::complexArg(Data::JsonScope& jsonDoc){
    auto complexNum = jsonDoc.get<double>(rootKey);
    if (!complexNum) {
        return false;
    }
    jsonDoc.set(rootKey, std::arg(complexNum.value()));
    return true;
}

bool Complex::complexToString(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    auto num = jsonDoc.getComplex(rootKey);
    if (!num) return true;
    try {
        auto formatter = Interaction::Logic::Formatter::readFormatter(Utility::StringHandler::recombineArgs(args.subspan(1)));
        jsonDoc.set(rootKey, complexFormatter(num.value(), formatter));
    } catch (...) {
        return false;
    }
    return true;
}

bool Complex::formatComplexNumberString(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc){
    if (args.size() != 2) return false; // No formatter provided
    auto const value = jsonDoc.get<std::string>(rootKey);
    if (!value.has_value()) return false; // Not convertible to string
    auto const split = parseComplexNumberString(value.value());
    if (!split.has_value()) return true; // Not a complex number
    auto num = std::complex{
        std::stod(std::string(split.value().re)),
        std::stod(std::string(split.value().im))
    };
    auto formatter = Interaction::Logic::Formatter::readFormatter(args[1]);
    jsonDoc.set(rootKey, complexFormatter(num,formatter));
    return true;
}

} // namespace Nebulite::Module::Transformation
