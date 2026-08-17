#ifndef NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP
#define NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <functional>
#include <memory>
#include <ranges>
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"
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
 * @details These transformations perform basic arithmetic operations on the current JSON value, which must be a number.
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

    static bool add(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr addName = "add";
    static auto constexpr addDesc = "Adds a numeric value to the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |add <number1> <number2> ... -> {number/array}\n";

    static bool mod(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr modName = "mod";
    static auto constexpr modDesc = "Calculates the modulo of the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |mod <number1> <number2> ... -> {number/array}\n";

    static bool multiply(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr multiplyName = "mul";
    static auto constexpr multiplyDesc = "Multiplies the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |multiply <number1> <number2> ... -> {number/array}\n";

    static bool pow(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr powName = "pow";
    static auto constexpr powDesc = "Raises the current JSON value to the power of a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |pow <exponent1> <exponent2> ... -> {number/array}\n";

    static bool subtract(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr subtractName = "sub";
    static auto constexpr subtractDesc = "Subtracts a numeric value from the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |sub <number1> <number2> ... -> {number/array}\n";

    static bool divide(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr divideName = "div";
    static auto constexpr divideDesc = "Divides the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |div <number1> <number2> ... -> {number/array}\n";

    static bool root(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr rootName = "root";
    static auto constexpr rootDesc = "Calculates the n-th root of the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |root <number1> <number2> ... -> {number/array}\n";

    static bool sqrt(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr sqrtName = "sqrt";
    static auto constexpr sqrtDesc = "Calculates the square root of the current JSON value.\n"
        "No arguments are allowed, as it's an operator with a single operand (the current JSON value).\n"
        "Usage: |sqrt -> {number}\n";

private:
    template<typename F>
    static bool forall(std::span<std::string_view const> const& args, F&& f) {
        static_assert(std::is_invocable_v<F, std::string_view, Data::ScopedKeyView>, "f must be a function F invokable with f(std::string_view, Data::ScopedKeyView)");

        if (args.size() < 2) {
            return false;
        }
        try {
            if (args.size() == 2) {
                return std::invoke(std::forward<F>(f), args[1], rootKey);
            }
            return std::ranges::all_of(args | std::views::drop(1) | Utility::Ranges::enumerate,
                [&](auto const& item) {
                    auto const& [index, arg] = item;
                    auto const key = rootKey.addIndex(index);
                    return std::invoke(std::forward<F>(f), arg, key.view());
                }
            );
        } catch (...) {
            return false;
        }
    }
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP
