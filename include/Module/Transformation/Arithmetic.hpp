/**
 * @file Arithmetic.hpp
 * @brief Class for arithmetic transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_ARITHMETIC_HPP
#define NEBULITE_TRANSFORMATION_MODULE_ARITHMETIC_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Arithmetic final : public Data::TransformationModule {
public:
    explicit Arithmetic(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool add(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr addName = "add";
    static auto constexpr addDesc = "Adds a numeric value to the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |add <number1> <number2> ... -> {number/array}\n";

    static bool mod(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr modName = "mod";
    static auto constexpr modDesc = "Calculates the modulo of the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |mod <number1> <number2> ... -> {number/array}\n";

    static bool multiply(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr multiplyName = "mul";
    static auto constexpr multiplyDesc = "Multiplies the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |multiply <number1> <number2> ... -> {number/array}\n";

    static bool pow(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr powName = "pow";
    static auto constexpr powDesc = "Raises the current JSON value to the power of a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |pow <exponent1> <exponent2> ... -> {number/array}\n";

    static bool subtract(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr subtractName = "sub";
    static auto constexpr subtractDesc = "Subtracts a numeric value from the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |sub <number1> <number2> ... -> {number/array}\n";

    static bool divide(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr divideName = "div";
    static auto constexpr divideDesc = "Divides the current JSON value by a numeric value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |div <number1> <number2> ... -> {number/array}\n";

    static bool root(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr rootName = "root";
    static auto constexpr rootDesc = "Calculates the n-th root of the current JSON value.\n"
        "If multiple numbers are provided, the result is an array, one element for each provided argument.\n"
        "Usage: |root <number1> <number2> ... -> {number/array}\n";

    static bool sqrt(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr sqrtName = "sqrt";
    static auto constexpr sqrtDesc = "Calculates the square root of the current JSON value.\n"
        "No arguments are allowed, as it's an operator with a single operand (the current JSON value).\n"
        "Usage: |sqrt -> {number}\n";

private:
    static bool forall(std::span<std::string const> const& args, std::function<bool(std::string const&, Data::ScopedKeyView const& key)> const& func);
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_ARITHMETIC_HPP
