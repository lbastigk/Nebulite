#ifndef NEBULITE_MODULE_TRANSFORMATION_COMPLEX_HPP
#define NEBULITE_MODULE_TRANSFORMATION_COMPLEX_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Interaction/Execution/FuncTree.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Complex
 * @brief Various transformations for complex numbers
 */
class Complex final : public Base::TransformationModule {
public:
    explicit Complex(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool numberToComplex(Data::JsonScope& jsonDoc);
    static auto constexpr numberToComplexName = "numberToComplex";
    static auto constexpr numberToComplexDesc = "Converts a given number to a complex number.\n"
        "Usage: |numberToComplex -> {complex-number}\n"
        "Allowed values are:\n"
        "  - Integers"
        "  - Real numbers (e.g., 3.14, -2.5)\n"
        "  - Scientific notation (e.g., 1e-3, -2.5e+4)\n"
        "  - Complex numbers in the form a+bi (e.g., 1+2i, -3-4i)\n"
        "The transformation fails if the current JSON value is not convertible to a complex number.\n";

    static bool complexAbs(Data::JsonScope& jsonDoc);
    static auto constexpr complexAbsName = "complexAbs";
    static auto constexpr complexAbsDesc = "Calculates the absolute value (magnitude) of a complex number.\n"
        "Usage: |complexAbs -> {absolute-value}\n"
        "The transformation fails if the current JSON value is not a complex number.\n";

    static bool complexArg(Data::JsonScope& jsonDoc);
    static auto constexpr complexArgName = "complexArg";
    static auto constexpr complexArgDesc = "Calculates the argument (phase angle) of a complex number in radians.\n"
        "Usage: |complexArg -> {argument}\n"
        "The transformation fails if the current JSON value is not a complex number.\n";

    static bool complexToString(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr complexToStringName = "complexToString";
    static auto constexpr complexToStringDesc = "Formats the contained complex number object to a string.\n"
        "If the stored value is not a complex number, the value is not modified.\n"
        "An optional format specifier can be provided to control the formatting of the real and imaginary parts.\n"
        "Example formatters: 04.2f, 5i, 06i\n"
        "Usage: |complexToString [formatter] -> {string}\n";

    static bool formatComplexNumberString(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr formatComplexNumberStringName = "formatComplexNumberString";
    static auto constexpr formatComplexNumberStringDesc = "Formats the contained complex number string to another string\n"
        "If the stored value is not a simple value, the transformation fails.\n"
        "If the stored value is a non-numeric string, the value is not modified.\n"
        "Valid numbers: 1+1i, 1e-20-500i, etc.\n"
        "A formatter must be provided.\n"
        "Example formatters: 04.2f, 5i, 06i\n"
        "Usage: |formatComplexNumberString <formatter> -> {string}";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_COMPLEX_HPP
