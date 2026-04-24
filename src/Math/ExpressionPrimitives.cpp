//------------------------------------------
// Includes

// External
#include "tinyexpr.h"

// Nebulite
#include "Interaction/Execution/FuncTree.hpp"
#include "Math/ExpressionPrimitives.hpp"

#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Math {

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

namespace {
bool pseudoBind() {
    return true;
}
} // namespace

void ExpressionPrimitives::help(std::span<std::string const> const& args) {
    // Create a temporary funcTree to utilize its printFunctionList method for formatted output

    Interaction::Execution::FuncTree tempFuncTree("Nebulite Expressions", true, true, Global::capture()); // Pass to main capture

    // Use a void lambda as binding function
    for (const auto& funcInfo : availableFunctions()) {
        tempFuncTree.bindFunction(&pseudoBind, funcInfo.name, funcInfo.description);
    }

    // Parse the given command into the temporary funcTree:
    // __FUNCTION__ help <potentially more args for specific function help>
    std::string argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    if (argStr.empty()) {
        argStr = __FUNCTION__ + std::string(" help");
    }
    else {
        argStr = __FUNCTION__ + std::string(" help ") + argStr;
    }
    tempFuncTree.parseStr(argStr);
}

std::vector<ExpressionPrimitives::FunctionInfo> const& ExpressionPrimitives::availableFunctions() {
    static std::vector<FunctionInfo> const functions = {
        // Logical comparison functions
        {gtName, gtDesc, reinterpret_cast<void*>(gt), TE_FUNCTION2, nullptr},
        {ltName, ltDesc, reinterpret_cast<void*>(lt), TE_FUNCTION2, nullptr},
        {geqName, geqDesc, reinterpret_cast<void*>(geq), TE_FUNCTION2, nullptr},
        {leqName, leqDesc, reinterpret_cast<void*>(leq), TE_FUNCTION2, nullptr},
        {eqName, eqDesc, reinterpret_cast<void*>(eq), TE_FUNCTION2, nullptr},
        {neqName, neqDesc, reinterpret_cast<void*>(neq), TE_FUNCTION2, nullptr},

        // Logical gate functions
        {logicalNotName, logicalNotDesc, reinterpret_cast<void*>(logical_not), TE_FUNCTION1, nullptr},
        {logicalAndName, logicalAndDesc, reinterpret_cast<void*>(logical_and), TE_FUNCTION2, nullptr},
        {logicalOrName, logicalOrDesc, reinterpret_cast<void*>(logical_or), TE_FUNCTION2, nullptr},
        {logicalXorName, logicalXorDesc, reinterpret_cast<void*>(logical_xor), TE_FUNCTION2, nullptr},
        {logicalNandName, logicalNandDesc, reinterpret_cast<void*>(logical_nand), TE_FUNCTION2, nullptr},
        {logicalNorName, logicalNorDesc, reinterpret_cast<void*>(logical_nor), TE_FUNCTION2, nullptr},
        {logicalXnorName, logicalXnorDesc, reinterpret_cast<void*>(logical_xnor), TE_FUNCTION2, nullptr},

        // Other logical functions
        {toBipolarName, toBipolarDesc, reinterpret_cast<void*>(to_bipolar), TE_FUNCTION1, nullptr},

        // Mapping functions
        {mapName, mapDesc, reinterpret_cast<void*>(map), TE_FUNCTION5, nullptr},
        {constrainName, constrainDesc, reinterpret_cast<void*>(constrain), TE_FUNCTION3, nullptr},

        // Maximum and Minimum functions
        {maxName, maxDesc, reinterpret_cast<void*>(max), TE_FUNCTION2, nullptr},
        {minName, minDesc, reinterpret_cast<void*>(min), TE_FUNCTION2, nullptr},

        // Rounding
        {roundName, roundDesc, reinterpret_cast<void*>(round), TE_FUNCTION2, nullptr},
        {roundUpName, roundUpDesc, reinterpret_cast<void*>(roundUp), TE_FUNCTION2, nullptr},
        {roundDownName, roundDownDesc, reinterpret_cast<void*>(roundDown), TE_FUNCTION2, nullptr},

        // More mathematical functions
        {sgnName, sgnDesc, reinterpret_cast<void*>(sgn), TE_FUNCTION1, nullptr},

        // RNG functions
        {rng2argName,rng2argDesc,reinterpret_cast<void*>(rng2arg), TE_FUNCTION2, nullptr},
        {rng3argName,rng3argDesc,reinterpret_cast<void*>(rng3arg), TE_FUNCTION3, nullptr},
        {rng2argInt16Name,rng2argInt16Desc,reinterpret_cast<void*>(rng2argInt16), TE_FUNCTION2, nullptr},
        {rng3argInt16Name,rng3argInt16Desc,reinterpret_cast<void*>(rng3argInt16), TE_FUNCTION3, nullptr},
    };
    return functions;
}

void ExpressionPrimitives::registerExpressions(std::vector<te_variable>& te_variables){
    for (const auto& funcInfo : availableFunctions()) {
        te_variables.push_back({funcInfo.name, funcInfo.pointer, funcInfo.type, funcInfo.context});
    }
}

} // namespace Nebulite::Interaction::Logic
