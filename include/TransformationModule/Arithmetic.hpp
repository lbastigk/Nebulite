/**
 * @file Arithmetic.hpp
 * @brief Class for arithmetic transformation modules.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_ARITHMETIC_HPP
#define NEBULITE_TRANSFORMATION_MODULE_ARITHMETIC_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Arithmetic final : public Data::TransformationModule {
public:
    explicit Arithmetic(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    // TODO: For all arithmetic functions: using multiple numbers should output array?
    //       For array inputs, we may use map

    static bool add(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr addName = "add";
    static auto constexpr addDesc = "Adds a numeric value to the current JSON value.\n"
        "Usage: |add <number1> <number2> ... -> {number}\n";

    static bool mod(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr modName = "mod";
    static auto constexpr modDesc = "Calculates the modulo of the current JSON value by a numeric value.\n"
        "Usage: |mod <number> -> {number}\n";

    static bool multiply(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr multiplyName = "mul";
    static auto constexpr multiplyDesc = "Multiplies the current JSON value by a numeric value.\n"
        "Usage: |multiply <number1> <number2> ...\n";

    static bool pow(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr powName = "pow";
    static auto constexpr powDesc = "Raises the current JSON value to the power of a numeric value.\n"
        "Usage: |pow <exponent> -> {number}\n";

    static bool subtract(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr subtractName = "sub";
    static auto constexpr subtractDesc = "Subtracts a numeric value from the current JSON value.\n"
        "Usage: |sub <number1> <number2> ... -> {number}\n";

    static bool divide(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr divideName = "div";
    static auto constexpr divideDesc = "Divides the current JSON value by a numeric value.\n"
        "Usage: |div <number1> <number2> ... -> {number}\n";

    static bool sqrt(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr sqrtName = "sqrt";
    static auto constexpr sqrtDesc = "Calculates the square root of the current JSON value.\n"
                "Usage: |sqrt -> {number}\n";

    static bool root(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr rootName = "root";
    static auto constexpr rootDesc = "Calculates the n-th root of the current JSON value.\n"
                        "Usage: |root <n> -> {number}\n";
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_ARITHMETIC_HPP
