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

    // TODO: push, pop, insert, remove, clear, etc.
    //       but first, add these functionalities to JSON class

    // TODO: dedupe, subarray, sort, sortby, pick, omit
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_ARITHMETIC_HPP
