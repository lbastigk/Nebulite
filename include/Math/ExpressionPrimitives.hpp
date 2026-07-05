#ifndef MATH_EXPRESSIONPRIMITIVES_HPP
#define MATH_EXPRESSIONPRIMITIVES_HPP

//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cmath>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

// External
#include <tinyexpr.h>

//------------------------------------------
namespace Nebulite::Math {
/**
 * @brief A collection of custom functions for TinyExpr
 *        Make sure to register all functions with TinyExpr in Nebulite::Math::Expression::registerExpressions
 */
class ExpressionPrimitives {
public:
    //----------------------------------
    // Logical comparison functions

    static double gt(double const a, double const b) { return a > b; }
    static auto constexpr gtName = "gt";
    static auto constexpr gtDesc = "Returns 1 if a is greater than b.\n"
        "Otherwise returns 0.\n"
        "Usage: gt(a, b)";

    static double lt(double const a, double const b) { return a < b; }
    static auto constexpr ltName = "lt";
    static auto constexpr ltDesc = "Returns 1 if a is less than b.\n"
        "Otherwise returns 0.\n"
        "Usage: lt(a, b)";

    static double geq(double const a, double const b) { return a >= b; }
    static auto constexpr geqName = "geq";
    static auto constexpr geqDesc = "Returns 1 if a is greater than or equal to b.\n"
        "Otherwise returns 0.\n"
        "Usage: geq(a, b)";

    static double leq(double const a, double const b) { return a <= b; }
    static auto constexpr leqName = "leq";
    static auto constexpr leqDesc = "Returns 1 if a is less than or equal to b.\n"
        "Otherwise returns 0.\n"
        "Usage: leq(a, b)";

    static double eq(double const a, double const b) {
        return std::fabs(a - b) < std::numeric_limits<double>::epsilon();
    }
    static auto constexpr eqName = "eq";
    static auto constexpr eqDesc = "Returns 1 if a is equal to b.\n"
        "Equality is tested within a small epsilon to handle floating point imprecision.\n"
        "Usage: eq(a, b)";

    static double neq(double const a, double const b) {
        return std::fabs(a - b) > std::numeric_limits<double>::epsilon();
    }
    static auto constexpr neqName = "neq";
    static auto constexpr neqDesc = "Returns 1 if a is not equal to b.\n"
        "Inequality is determined beyond a small epsilon.\n"
        "Usage: neq(a, b)";

    //----------------------------------
    // Logical gate functions

    static double logical_not(double const a) {
        return !(std::fabs(a) > std::numeric_limits<double>::epsilon());
    }
    static auto constexpr logicalNotName = "not";
    static auto constexpr logicalNotDesc = "Returns 1 if a is logically false (close to zero), otherwise returns 0.\n"
        "Values with absolute value <= epsilon are treated as false.\n"
        "Usage: not(a)";

    static double logical_and(double const a, double const b) {
        bool const aLogical = std::fabs(a) > std::numeric_limits<double>::epsilon();
        bool const bLogical = std::fabs(b) > std::numeric_limits<double>::epsilon();
        return aLogical && bLogical;
    }
    static auto constexpr logicalAndName = "and";
    static auto constexpr logicalAndDesc = "Returns 1 if both a and b are logically true, otherwise returns 0.\n"
        "A value is considered true when its absolute value is greater than epsilon.\n"
        "Usage: and(a, b)";

    static double logical_or(double const a, double const b) {
        bool const aLogical = std::fabs(a) > std::numeric_limits<double>::epsilon();
        bool const bLogical = std::fabs(b) > std::numeric_limits<double>::epsilon();
        return aLogical || bLogical;
    }
    static auto constexpr logicalOrName = "or";
    static auto constexpr logicalOrDesc = "Returns 1 if either a or b is logically true, otherwise returns 0.\n"
        "A value is considered true when its absolute value is greater than epsilon.\n"
        "Usage: or(a, b)";

    static double logical_xor(double const a, double const b) {
        bool const aLogical = std::fabs(a) > std::numeric_limits<double>::epsilon();
        bool const bLogical = std::fabs(b) > std::numeric_limits<double>::epsilon();
        return aLogical != bLogical;
    }
    static auto constexpr logicalXorName = "xor";
    static auto constexpr logicalXorDesc = "Returns 1 if exactly one of a or b is logically true, otherwise returns 0.\n"
        "Uses epsilon threshold to determine logical truthiness.\n"
        "Usage: xor(a, b)";

    static double logical_nand(double const a, double const b) {
        bool const aLogical = std::fabs(a) > std::numeric_limits<double>::epsilon();
        bool const bLogical = std::fabs(b) > std::numeric_limits<double>::epsilon();
        return !(aLogical && bLogical);
    }
    static auto constexpr logicalNandName = "nand";
    static auto constexpr logicalNandDesc = "Returns 1 if at least one of a or b is logically false (i.e. not both true), otherwise returns 0.\n"
        "Uses epsilon to determine logical truthiness.\n"
        "Usage: nand(a, b)";

    static double logical_nor(double const a, double const b) {
        bool const aLogical = std::fabs(a) > std::numeric_limits<double>::epsilon();
        bool const bLogical = std::fabs(b) > std::numeric_limits<double>::epsilon();
        return !(aLogical || bLogical);
    }
    static auto constexpr logicalNorName = "nor";
    static auto constexpr logicalNorDesc = "Returns 1 if both a and b are logically false, otherwise returns 0.\n"
        "Values with absolute value <= epsilon are treated as false.\n"
        "Usage: nor(a, b)";

    static double logical_xnor(double const a, double const b) {
        bool const aLogical = std::fabs(a) > std::numeric_limits<double>::epsilon();
        bool const bLogical = std::fabs(b) > std::numeric_limits<double>::epsilon();
        return aLogical == bLogical;
    }
    static auto constexpr logicalXnorName = "xnor";
    static auto constexpr logicalXnorDesc = "Returns 1 if a and b are both logically true or both logically false, otherwise returns 0.\n"
        "Uses epsilon threshold to determine logical equality.\n"
        "Usage: xnor(a, b)";

    // Other logical functions

    static double toBipolar(double const a) {
        return std::fabs(a) > std::numeric_limits<double>::epsilon() ? 1.0 : -1.0;
    }
    static auto constexpr toBipolarName = "toBipolar";
    static auto constexpr toBipolarDesc = "Converts a numeric value to bipolar form.\n"
        "output is 1 or -1\n"
        "Returns 1 if a is logically true (absolute value > epsilon), otherwise returns -1.\n"
        "Usage: toBipolar(a)";

    //----------------------------------
    // Mapping functions

    static double map(double const value, double const in_min, double const in_max, double const out_min, double const out_max) {
        if (std::fabs(in_max - in_min) < std::numeric_limits<double>::epsilon()) { return out_min; } // Prevent division by zero
        if (value < in_min) { return out_min; }
        if (value > in_max) { return out_max; }
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
    static auto constexpr mapName = "map";
    static auto constexpr mapDesc = "Linearly maps a value from one range to another.\n"
        "Usage: map(value, in_min, in_max, out_min, out_max)";

    static double constrain(double const value, double const min, double const max) {
        if (value < min) { return min; }
        if (value > max) { return max; }
        return value;
    }
    static auto constexpr constrainName = "constrain";
    static auto constexpr constrainDesc = "Constrains a value to lie between a minimum and maximum value.\n"
        "Usage: constrain(value, min, max)";

    //----------------------------------
    // Maximum and Minimum functions

    static double max(double const a, double const b) { return std::max(a,b); }
    static auto constexpr maxName = "max";
    static auto constexpr maxDesc = "Returns the greater of a and b.\n"
            "Usage: max(a, b)";

    static double min(double const a, double const b) { return std::min(a,b); }
    static auto constexpr minName = "min";
    static auto constexpr minDesc = "Returns the lesser of a and b.\n"
        "Usage: min(a, b)";

    //----------------------------------
    // Rounding

    static double round(double const a, double const b) {
        auto const digits = std::floor(b);
        return std::round(a * std::pow(10, digits)) / std::pow(10, digits);
    }
    static auto constexpr roundName = "round";
    static auto constexpr roundDesc = "Rounds the first argument to the amount of decimal places specified by the second argument.\n"
        "Usage: round(a, b)";

    static double roundUp(double const a, double const b) {
        auto const digits = std::floor(b);
        return std::floor(a * std::pow(10, digits)) / std::pow(10, digits);
    }
    static auto constexpr roundUpName = "roundUp";
    static auto constexpr roundUpDesc = "Rounds the first argument up to the amount of decimal places specified by the second argument.\n"
        "Usage: roundUp(a, b)";

    static auto constexpr roundDown(double const a, double const b) {
        auto const digits = std::floor(b);
        return std::ceil(a * std::pow(10, digits)) / std::pow(10, digits);
    }
    static auto constexpr roundDownName = "roundDown";
    static auto constexpr roundDownDesc = "Rounds the first argument down to the amount of decimal places specified by the second argument.\n"
        "Usage: roundDown(a, b)";

    //----------------------------------
    // More mathematical functions

    static double sgn(double const a) { return std::copysign(1.0, a); }
    static auto constexpr sgnName = "sgn";
    static auto constexpr sgnDesc = "Returns the sign of a.\n"
        "Returns 1 if a is positive, -1 if a is negative, and 0 if a is zero.\n"
        "Usage: sgn(a)";

    //----------------------------------
    // Pseudo-random functions

    // Idea: offer various RNG functions here, useful for pseudo-random logic:
    // RNG2ARG(a,b) -> returns a random number, seeded from a and b
    // RNG3ARG(a,b,c) -> returns a random number, seeded from a, b and c
    // etc.
    // We could use this to:
    // - determine tileset usage based on position
    // - offer more variety of predefined RNGs in GlobalSpace
    // This is still deterministic, so we don't have to worry about issues with variation between runs.
    // However, perhaps this can still occur if we seed from a non-deterministic source (e.g. time) or derived non-deterministic source.
    // IMPORTANT: Consider non-deterministic propagation!

    static double rng2arg(double a, double b);
    static auto constexpr rng2argName = "rng2arg";
    static auto constexpr rng2argDesc = "Returns a pseudo-random number between 0 and 1, deterministically seeded from the input values a and b.\n"
        "The same input values will always produce the same output, making it suitable for deterministic procedural generation.\n"
        "Usage: rng2arg(a, b)";

    static double rng3arg(double a, double b, double c);
    static auto constexpr rng3argName = "rng3arg";
    static auto constexpr rng3argDesc = "Returns a pseudo-random number between 0 and 1, deterministically seeded from the input values a, b, and c.\n"
        "The same input values will always produce the same output, making it suitable for deterministic procedural generation.\n"
        "Usage: rng3arg(a, b, c)";

    static double rng2argInt16(double a, double b);
    static auto constexpr rng2argInt16Name = "rng2argInt16";
    static auto constexpr rng2argInt16Desc = "Returns a pseudo-random integer between 0 and 32767, deterministically seeded from the input values a and b.\n"
        "The same input values will always produce the same output, making it suitable for deterministic procedural generation.\n"
        "Usage: rng2argInt16(a, b)";

    static double rng3argInt16(double a, double b, double c);
    static auto constexpr rng3argInt16Name = "rng3argInt16";
    static auto constexpr rng3argInt16Desc = "Returns a pseudo-random integer between 0 and 32767, deterministically seeded from the input values a, b, and c.\n"
        "The same input values will always produce the same output, making it suitable for deterministic procedural generation.\n"
        "Usage: rng3argInt16(a, b, c)";


    //------------------------------------------
    // Register

    /**
     * @brief Registers all custom functions with TinyExpr by adding them to the provided vector of te_variable.
     * @param te_variables A vector of te_variable to which the custom functions will be added for registration with TinyExpr.
     */
    static void registerExpressions(std::vector<te_variable>& te_variables);

    //------------------------------------------
    // List

    /**
     * @brief Prints a list of all available functions with their descriptions to the console.
     * @details Utilizes a temporary funcTree for the sole purpose of printing the list of functions with their descriptions in a formatted manner.
     */
    static void help(std::span<std::string_view const> const& args);

private:
    //------------------------------------------
    // Struct for a function with metadata

    /**
     * @brief Struct to hold information about a function for registration and help listing
     * @details This struct is used to store the function pointer along with its name, description, and other metadata.
     *          It allows us to maintain a list of available functions in a structured way.
     */
    struct FunctionInfo {
        const char* name;
        const char* description;
        void* pointer;
        int type;
        void* context;
    };

    //------------------------------------------
    // List of all functions with metadata for registration

    /**
     * @brief Returns a reference to a vector containing information about all available functions for registration and help listing.
     * @return A reference to a vector of FunctionInfo structs, each representing a function available in ExpressionPrimitives.
     */
    static std::vector<FunctionInfo> const& availableFunctions();
};
} // namespace Nebulite::Math
#endif // MATH_EXPRESSIONPRIMITIVES_HPP
