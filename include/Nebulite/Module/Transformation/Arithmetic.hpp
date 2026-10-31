#ifndef NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP
#define NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <ranges>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Ranges.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Arithmetic
 * @brief Various arithmetic transformations for numeric JSON values.
 * @details These transformations perform basic arithmetic operations on the current JSON value.
 *          Invalid existing values propagate as NaN; provided arguments must be valid numbers.
 *          The transformations can take one or more numeric arguments, and the result is either a single number (if one argument is provided)
 *          or an array of numbers (if multiple arguments are provided).
 */
class Arithmetic final : public Base::TransformationModule {
public:
    explicit Arithmetic(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool add(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr addName = "add";
    static auto constexpr addDesc = "Adds a numeric value to the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |add <number1> <number2> ... -> {number/array}\n";

    static bool mod(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr modName = "mod";
    static auto constexpr modDesc = "Calculates the modulo of the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |mod <number1> <number2> ... -> {number/array}\n";

    static bool multiply(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr multiplyName = "mul";
    static auto constexpr multiplyDesc = "Multiplies the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |mul <number1> <number2> ... -> {number/array}\n";

    static bool pow(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr powName = "pow";
    static auto constexpr powDesc = "Raises the current JSON value to the power of a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |pow <exponent1> <exponent2> ... -> {number/array}\n";

    static bool subtract(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr subtractName = "sub";
    static auto constexpr subtractDesc = "Subtracts a numeric value from the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |sub <number1> <number2> ... -> {number/array}\n";

    static bool divide(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr divideName = "div";
    static auto constexpr divideDesc = "Divides the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |div <number1> <number2> ... -> {number/array}\n";

    static bool root(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr rootName = "root";
    static auto constexpr rootDesc = "Calculates the n-th root of the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |root <number1> <number2> ... -> {number/array}\n";

    static bool sqrt(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr sqrtName = "sqrt";
    static auto constexpr sqrtDesc = "Calculates the square root of the current JSON value.\n"
        "No arguments are allowed, as it's an operator with a single operand (the current JSON value).\n"
        "Usage: |sqrt -> {number}\n";

private:
    template<typename F>
    static bool forall(Utility::Args::SegmentedStringView const& args, F&& f) {
        static_assert(std::is_invocable_v<F, double, Data::ScopedKeyView>, "f must be invocable with f(double, Data::ScopedKeyView)");

        if (args.size() < 2) {
            return false;
        }
        if (args.size() == 2) {
            auto const arg = args.tryGetAs<double>(1);
            return arg.has_value() && std::isfinite(arg.value())
                && std::invoke(std::forward<F>(f), arg.value(), rootKey);
        }

        return std::ranges::all_of(args | std::views::drop(1) | Utility::Ranges::enumerate,
            [&](auto const& item) {
                auto const index = item.first;
                auto const arg = args.tryGetAs<double>(index + 1);
                if (!arg || !std::isfinite(arg.value())) {
                    return false;
                }
                return std::invoke(std::forward<F>(f), arg.value(), rootKey.addIndex(index).view());
            }
        );
    }
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP
