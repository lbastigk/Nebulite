/**
 * @file ExpressionPrimitives.hpp
 * @brief This file contains a collection of custom functions for TinyExpr used in Nebulite
 */

#ifndef NEBULITE_INTERACTION_LOGIC_EXPRESSION_PRIMITIVES_HPP
#define NEBULITE_INTERACTION_LOGIC_EXPRESSION_PRIMITIVES_HPP

//------------------------------------------
// Includes

// Standard library
#include <cfloat>
#include <cmath>
#include <vector>

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @brief A collection of custom functions for TinyExpr
 *        Make sure to register all functions with TinyExpr in Nebulite::Interaction::Logic::Expression::reset
 * @note Marking the parameters as `const&` does not work with TinyExpr function pointers,
 *       so they are passed by value instead.
 */
class ExpressionPrimitives {
public:
    //----------------------------------
    // Logical comparison functions

    // NOLINTNEXTLINE
    static double gt(double a, double b) { return a > b; }
    static auto constexpr gtName = "gt";
    static auto constexpr gtDesc = "Returns 1 if a is greater than b.\n"
        "Otherwise returns 0.\n"
        "Usage: gt(a, b)";

    // NOLINTNEXTLINE
    static double lt(double a, double b) { return a < b; }
    static auto constexpr ltName = "lt";
    static auto constexpr ltDesc = "Returns 1 if a is less than b.\n"
        "Otherwise returns 0.\n"
        "Usage: lt(a, b)";

    // NOLINTNEXTLINE
    static double geq(double a, double b) { return a >= b; }
    static auto constexpr geqName = "geq";
    static auto constexpr geqDesc = "Returns 1 if a is greater than or equal to b.\n"
        "Otherwise returns 0.\n"
        "Usage: geq(a, b)";

    // NOLINTNEXTLINE
    static double leq(double a, double b) { return a <= b; }
    static auto constexpr leqName = "leq";
    static auto constexpr leqDesc = "Returns 1 if a is less than or equal to b.\n"
        "Otherwise returns 0.\n"
        "Usage: leq(a, b)";

    // NOLINTNEXTLINE
    static double eq(double a, double b) {
        return std::fabs(a - b) < DBL_EPSILON;
    }
    static auto constexpr eqName = "eq";
    static auto constexpr eqDesc = "Returns 1 if a is equal to b.\n"
        "Equality is tested within a small epsilon (DBL_EPSILON) to handle floating point imprecision.\n"
        "Usage: eq(a, b)";

    // NOLINTNEXTLINE
    static double neq(double a, double b) {
        return std::fabs(a - b) > DBL_EPSILON;
    }
    static auto constexpr neqName = "neq";
    static auto constexpr neqDesc = "Returns 1 if a is not equal to b.\n"
        "Inequality is determined beyond a small epsilon (DBL_EPSILON).\n"
        "Usage: neq(a, b)";

    //----------------------------------
    // Logical gate functions

    // NOLINTNEXTLINE
    static double logical_not(double a) {
        return !(std::fabs(a) > DBL_EPSILON);
    }
    static auto constexpr logicalNotName = "not";
    static auto constexpr logicalNotDesc = "Returns 1 if a is logically false (close to zero), otherwise returns 0.\n"
        "Values with absolute value <= DBL_EPSILON are treated as false.\n"
        "Usage: not(a)";

    // NOLINTNEXTLINE
    static double logical_and(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return aLogical && bLogical;
    }
    static auto constexpr logicalAndName = "and";
    static auto constexpr logicalAndDesc = "Returns 1 if both a and b are logically true, otherwise returns 0.\n"
        "A value is considered true when its absolute value is greater than DBL_EPSILON.\n"
        "Usage: and(a, b)";

    // NOLINTNEXTLINE
    static double logical_or(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return aLogical || bLogical;
    }
    static auto constexpr logicalOrName = "or";
    static auto constexpr logicalOrDesc = "Returns 1 if either a or b is logically true, otherwise returns 0.\n"
        "A value is considered true when its absolute value is greater than DBL_EPSILON.\n"
        "Usage: or(a, b)";

    // NOLINTNEXTLINE
    static double logical_xor(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return aLogical != bLogical;
    }
    static auto constexpr logicalXorName = "xor";
    static auto constexpr logicalXorDesc = "Returns 1 if exactly one of a or b is logically true, otherwise returns 0.\n"
        "Uses DBL_EPSILON threshold to determine logical truthiness.\n"
        "Usage: xor(a, b)";

    // NOLINTNEXTLINE
    static double logical_nand(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return !(aLogical && bLogical);
    }
    static auto constexpr logicalNandName = "nand";
    static auto constexpr logicalNandDesc = "Returns 1 if at least one of a or b is logically false (i.e. not both true), otherwise returns 0.\n"
        "Uses DBL_EPSILON to determine logical truthiness.\n"
        "Usage: nand(a, b)";

    // NOLINTNEXTLINE
    static double logical_nor(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return !(aLogical || bLogical);
    }
    static auto constexpr logicalNorName = "nor";
    static auto constexpr logicalNorDesc = "Returns 1 if both a and b are logically false, otherwise returns 0.\n"
        "Values with absolute value <= DBL_EPSILON are treated as false.\n"
        "Usage: nor(a, b)";

    // NOLINTNEXTLINE
    static double logical_xnor(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return aLogical == bLogical;
    }
    static auto constexpr logicalXnorName = "xnor";
    static auto constexpr logicalXnorDesc = "Returns 1 if a and b are both logically true or both logically false, otherwise returns 0.\n"
        "Uses DBL_EPSILON threshold to determine logical equality.\n"
        "Usage: xnor(a, b)";

    // Other logical functions

    // NOLINTNEXTLINE
    static double to_bipolar(double a) {
        return std::fabs(a) > DBL_EPSILON ? 1 : -1;
    }
    static auto constexpr toBipolarName = "to_bipolar";
    static auto constexpr toBipolarDesc = "Converts a numeric value to bipolar form.\n"
        "output is 1 or -1\n"
        "Returns 1 if a is logically true (absolute value > DBL_EPSILON), otherwise returns -1.\n"
        "Usage: to_bipolar(a)";

    //----------------------------------
    // Mapping functions

    // NOLINTNEXTLINE
    static double map(double value, double in_min, double in_max, double out_min, double out_max) {
        if (std::fabs(in_max - in_min) < DBL_EPSILON) { return out_min; } // Prevent division by zero
        if (value < in_min) { return out_min; }
        if (value > in_max) { return out_max; }
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
    static auto constexpr mapName = "map";
    static auto constexpr mapDesc = "Linearly maps a value from one range to another.\n"
        "Usage: map(value, in_min, in_max, out_min, out_max)";

    // NOLINTNEXTLINE
    static double constrain(double value, double min, double max) {
        if (value < min) { return min; }
        if (value > max) { return max; }
        return value;
    }
    static auto constexpr constrainName = "constrain";
    static auto constexpr constrainDesc = "Constrains a value to lie between a minimum and maximum value.\n"
        "Usage: constrain(value, min, max)";

    //----------------------------------
    // Maximum and Minimum functions

    // NOLINTNEXTLINE
    static double max(double a, double b) { return (a > b) ? a : b; }
    static auto constexpr maxName = "max";
    static auto constexpr maxDesc = "Returns the greater of a and b.\n"
            "Usage: max(a, b)";

    // NOLINTNEXTLINE
    static double min(double a, double b) { return (a < b) ? a : b; }
    static auto constexpr minName = "min";
    static auto constexpr minDesc = "Returns the lesser of a and b.\n"
        "Usage: min(a, b)";

    //----------------------------------
    // More mathematical functions

    // NOLINTNEXTLINE
    static double sgn(double a) { return std::copysign(1.0, a); }
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

    // NOLINTNEXTLINE
    static double rng2arg(double a, double b);
    static auto constexpr rng2argName = "rng2arg";
    static auto constexpr rng2argDesc = "Returns a pseudo-random number between 0 and 1, deterministically seeded from the input values a and b.\n"
        "The same input values will always produce the same output, making it suitable for deterministic procedural generation.\n"
        "Usage: rng2arg(a, b)";

    // NOLINTNEXTLINE
    static double rng3arg(double a, double b, double c);
    static auto constexpr rng3argName = "rng3arg";
    static auto constexpr rng3argDesc = "Returns a pseudo-random number between 0 and 1, deterministically seeded from the input values a, b, and c.\n"
        "The same input values will always produce the same output, making it suitable for deterministic procedural generation.\n"
        "Usage: rng3arg(a, b, c)";

    // NOLINTNEXTLINE
    static double rng2argInt16(double a, double b);
    static auto constexpr rng2argInt16Name = "rng2argInt16";
    static auto constexpr rng2argInt16Desc = "Returns a pseudo-random integer between 0 and 32767, deterministically seeded from the input values a and b.\n"
        "The same input values will always produce the same output, making it suitable for deterministic procedural generation.\n"
        "Usage: rng2argInt16(a, b)";

    // NOLINTNEXTLINE
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
    static void help(std::span<std::string const> const& args);

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
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSION_PRIMITIVES_HPP
