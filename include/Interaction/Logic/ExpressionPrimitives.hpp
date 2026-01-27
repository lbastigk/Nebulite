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

    // NOLINTNEXTLINE
    static double lt(double a, double b) { return a < b; }

    // NOLINTNEXTLINE
    static double geq(double a, double b) { return a >= b; }

    // NOLINTNEXTLINE
    static double leq(double a, double b) { return a <= b; }

    // NOLINTNEXTLINE
    static double eq(double a, double b) {
        return std::fabs(a - b) < DBL_EPSILON;
    }

    // NOLINTNEXTLINE
    static double neq(double a, double b) {
        return std::fabs(a - b) > DBL_EPSILON;
    }

    //----------------------------------
    // Logical gate functions

    // NOLINTNEXTLINE
    static double logical_not(double a) {
        return !(std::fabs(a) > DBL_EPSILON);
    }

    // NOLINTNEXTLINE
    static double logical_and(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return aLogical && bLogical;
    }

    // NOLINTNEXTLINE
    static double logical_or(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return aLogical || bLogical;
    }

    // NOLINTNEXTLINE
    static double logical_xor(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return aLogical != bLogical;
    }

    // NOLINTNEXTLINE
    static double logical_nand(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return !(aLogical && bLogical);
    }

    // NOLINTNEXTLINE
    static double logical_nor(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return !(aLogical || bLogical);
    }

    // NOLINTNEXTLINE
    static double logical_xnor(double a, double b) {
        bool const aLogical = std::fabs(a) > DBL_EPSILON;
        bool const bLogical = std::fabs(b) > DBL_EPSILON;
        return aLogical == bLogical;
    }

    // Other logical functions

    // NOLINTNEXTLINE
    static double to_bipolar(double a) {
        return std::fabs(a) > DBL_EPSILON ? 1 : -1;
    }

    //----------------------------------
    // Mapping functions

    // NOLINTNEXTLINE
    static double map(double value, double in_min, double in_max, double out_min, double out_max) {
        if (std::fabs(in_max - in_min) < DBL_EPSILON) { return out_min; } // Prevent division by zero
        if (value < in_min) { return out_min; }
        if (value > in_max) { return out_max; }
        return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    // NOLINTNEXTLINE
    static double constrain(double value, double min, double max) {
        if (value < min) { return min; }
        if (value > max) { return max; }
        return value;
    }

    //----------------------------------
    // Maximum and Minimum functions

    // NOLINTNEXTLINE
    static double max(double a, double b) { return (a > b) ? a : b; }

    // NOLINTNEXTLINE
    static double min(double a, double b) { return (a < b) ? a : b; }

    //----------------------------------
    // More mathematical functions

    // NOLINTNEXTLINE
    static double sgn(double a) { return std::copysign(1.0, a); }

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

    // NOLINTNEXTLINE
    static double rng3arg(double a, double b, double c);

    // NOLINTNEXTLINE
    static double rng2argInt16(double a, double b);

    // NOLINTNEXTLINE
    static double rng3argInt16(double a, double b, double c);

    //------------------------------------------
    // Register

    static void registerExpressions(std::vector<te_variable>& te_variables);
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_EXPRESSION_PRIMITIVES_HPP
