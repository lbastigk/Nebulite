#ifndef NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP
#define NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP

//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

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
 *          or an array of results computed independently from the original value (if multiple arguments are provided).
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
        "Each argument is applied independently to the original value; multiple arguments produce an array.\n"
        "Arguments must be finite numbers; an unusable input produces NaN.\n"
        "Usage: |add <number1> <number2> ... -> {number/array}\n";

    static bool mod(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr modName = "mod";
    static auto constexpr modDesc = "Calculates the modulo of the current JSON value by a numeric value.\n"
        "Each argument is applied independently to the original value; multiple arguments produce an array.\n"
        "Arguments must be finite numbers; an unusable input produces NaN.\n"
        "Moduli with magnitude below double epsilon are rejected.\n"
        "Usage: |mod <number1> <number2> ... -> {number/array}\n";

    static bool multiply(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr multiplyName = "mul";
    static auto constexpr multiplyDesc = "Multiplies the current JSON value by a numeric value.\n"
        "Each argument is applied independently to the original value; multiple arguments produce an array.\n"
        "Arguments must be finite numbers; an unusable input produces NaN.\n"
        "Usage: |mul <number1> <number2> ... -> {number/array}\n";

    static bool pow(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr powName = "pow";
    static auto constexpr powDesc = "Raises the current JSON value to the power of a numeric value.\n"
        "Each argument is applied independently to the original value; multiple arguments produce an array.\n"
        "Arguments must be finite numbers; an unusable input produces NaN.\n"
        "Usage: |pow <exponent1> <exponent2> ... -> {number/array}\n";

    static bool subtract(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr subtractName = "sub";
    static auto constexpr subtractDesc = "Subtracts a numeric value from the current JSON value.\n"
        "Each argument is applied independently to the original value; multiple arguments produce an array.\n"
        "Arguments must be finite numbers; an unusable input produces NaN.\n"
        "Usage: |sub <number1> <number2> ... -> {number/array}\n";

    static bool divide(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr divideName = "div";
    static auto constexpr divideDesc = "Divides the current JSON value by a numeric value.\n"
        "Each argument is applied independently to the original value; multiple arguments produce an array.\n"
        "Arguments must be finite numbers; an unusable input produces NaN.\n"
        "Divisors with magnitude below double epsilon are rejected.\n"
        "Usage: |div <number1> <number2> ... -> {number/array}\n";

    static bool root(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr rootName = "root";
    static auto constexpr rootDesc = "Calculates the n-th root of the current JSON value.\n"
        "Each argument is applied independently to the original value; multiple arguments produce an array.\n"
        "Orders must be finite, with magnitude at least double epsilon; an unusable input produces NaN.\n"
        "For negative inputs, only odd integer orders are supported (including negative orders).\n"
        "Usage: |root <number1> <number2> ... -> {number/array}\n";

    static bool abs(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr absName = "abs";
    static auto constexpr absDesc = "Calculates the absolute value of the current JSON value.\n"
        "No arguments are allowed, as it's an operator with a single operand (the current JSON value).\n"
        "An unusable input produces NaN.\n"
        "Usage: |abs -> {number}\n";

    static bool sqrt(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr sqrtName = "sqrt";
    static auto constexpr sqrtDesc = "Calculates the square root of the current JSON value.\n"
        "No arguments are allowed, as it's an operator with a single operand (the current JSON value).\n"
        "Negative or unusable inputs produce NaN.\n"
        "Usage: |sqrt -> {number}\n";

private:
    template<typename F>
    static bool forall(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc, F const& f) {
        if (args.size() < 2) {
            return false;
        }
        double const input = jsonDoc.get<double>(rootKey).value_or(std::numeric_limits<double>::quiet_NaN());
        std::vector<double> results;
        results.reserve(args.size() - 1);
        for (std::size_t index = 1; index < args.size(); ++index) {
            auto const argument = args.tryGetAs<double>(index);
            if (args[index] == "true" || args[index] == "false" || !argument || !std::isfinite(*argument)) {
                return false;
            }
            auto const result = std::invoke(f, input, *argument);
            if (!result) {
                return false;
            }
            results.push_back(*result);
        }
        if (results.size() == 1) {
            jsonDoc.set<double>(rootKey, results.front());
        } else {
            jsonDoc.insertArray(rootKey, results);
        }
        return true;
    }
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_ARITHMETIC_HPP
