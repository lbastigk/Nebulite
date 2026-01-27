//------------------------------------------
// Includes

// Standard library
#include <cstdint>

// External
#include "tinyexpr.h"

// Nebulite
#include "Interaction/Logic/ExpressionPrimitives.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Logic {

// NOLINTNEXTLINE
double ExpressionPrimitives::rng2arg(double a, double b) {
    uint64_t seed = static_cast<uint64_t>(a * 73856093) ^ static_cast<uint64_t>(b * 19349663);
    seed = (seed ^ seed >> 30) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ seed >> 27) * 0x94d049bb133111eb;
    seed = seed ^ seed >> 31;
    return static_cast<double>(seed % 10000) / 10000.0; // Return a value between 0 and 1
}

// NOLINTNEXTLINE
double ExpressionPrimitives::rng3arg(double a, double b, double c) {
    uint64_t seed = static_cast<uint64_t>(a * 73856093) ^ static_cast<uint64_t>(b * 19349663) ^ static_cast<uint64_t>(c * 83492791);
    seed = (seed ^ seed >> 30) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ seed >> 27) * 0x94d049bb133111eb;
    seed = seed ^ seed >> 31;
    return static_cast<double>(seed % 10000) / 10000.0; // Return a value between 0 and 1
}

// NOLINTNEXTLINE
double ExpressionPrimitives::rng2argInt16(double a, double b) {
    uint64_t seed = static_cast<uint64_t>(a * 73856093) ^ static_cast<uint64_t>(b * 19349663);
    seed = (seed ^ seed >> 30) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ seed >> 27) * 0x94d049bb133111eb;
    seed = seed ^ seed >> 31;
    return static_cast<double>(seed % 32768); // Return a value between 0 and 32767
}

// NOLINTNEXTLINE
double ExpressionPrimitives::rng3argInt16(double a, double b, double c) {
    uint64_t seed = static_cast<uint64_t>(a * 73856093) ^ static_cast<uint64_t>(b * 19349663) ^ static_cast<uint64_t>(c * 83492791);
    seed = (seed ^ seed >> 30) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ seed >> 27) * 0x94d049bb133111eb;
    seed = seed ^ seed >> 31;
    return static_cast<double>(seed % 32768); // Return a value between 0 and 32767
}

void ExpressionPrimitives::registerExpressions(std::vector<te_variable>& te_variables){
//====================================================================================================================
    // Category             Name           Pointer                                               Type           Context
    //====================================================================================================================

    // Logical comparison functions
    te_variables.push_back({"gt", reinterpret_cast<void*>(gt), TE_FUNCTION2, nullptr});
    te_variables.push_back({"lt", reinterpret_cast<void*>(lt), TE_FUNCTION2, nullptr});
    te_variables.push_back({"geq", reinterpret_cast<void*>(geq), TE_FUNCTION2, nullptr});
    te_variables.push_back({"leq", reinterpret_cast<void*>(leq), TE_FUNCTION2, nullptr});
    te_variables.push_back({"eq", reinterpret_cast<void*>(eq), TE_FUNCTION2, nullptr});
    te_variables.push_back({"neq", reinterpret_cast<void*>(neq), TE_FUNCTION2, nullptr});

    // Logical gate functions
    te_variables.push_back({"not", reinterpret_cast<void*>(logical_not), TE_FUNCTION1, nullptr});
    te_variables.push_back({"and", reinterpret_cast<void*>(logical_and), TE_FUNCTION2, nullptr});
    te_variables.push_back({"or", reinterpret_cast<void*>(logical_or), TE_FUNCTION2, nullptr});
    te_variables.push_back({"xor", reinterpret_cast<void*>(logical_xor), TE_FUNCTION2, nullptr});
    te_variables.push_back({"nand", reinterpret_cast<void*>(logical_nand), TE_FUNCTION2, nullptr});
    te_variables.push_back({"nor", reinterpret_cast<void*>(logical_nor), TE_FUNCTION2, nullptr});
    te_variables.push_back({"xnor", reinterpret_cast<void*>(logical_xnor), TE_FUNCTION2, nullptr});

    // Other logical functions
    te_variables.push_back({"to_bipolar", reinterpret_cast<void*>(to_bipolar), TE_FUNCTION1, nullptr});

    // Mapping functions
    te_variables.push_back({"map", reinterpret_cast<void*>(map), TE_FUNCTION5, nullptr});
    te_variables.push_back({"constrain", reinterpret_cast<void*>(constrain), TE_FUNCTION3, nullptr});

    // Maximum and Minimum functions
    te_variables.push_back({"max", reinterpret_cast<void*>(max), TE_FUNCTION2, nullptr});
    te_variables.push_back({"min", reinterpret_cast<void*>(min), TE_FUNCTION2, nullptr});

    // More mathematical functions
    te_variables.push_back({"sgn", reinterpret_cast<void*>(sgn), TE_FUNCTION1, nullptr});

    // RNG functions

    // Range 0 to 1
    te_variables.push_back({"rng2arg", reinterpret_cast<void*>(rng2arg), TE_FUNCTION2, nullptr});
    te_variables.push_back({"rng3arg", reinterpret_cast<void*>(rng3arg), TE_FUNCTION3, nullptr});

    // Range 0 to 32767
    te_variables.push_back({"rng2argInt16", reinterpret_cast<void*>(rng2argInt16), TE_FUNCTION2, nullptr});
    te_variables.push_back({"rng3argInt16", reinterpret_cast<void*>(rng3argInt16), TE_FUNCTION3, nullptr});
}

} // namespace Nebulite::Interaction::Logic
