//------------------------------------------
// Includes

// Standard library
// NOLINTNEXTLINE
#include <cstdlib>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// External
#include <tinyexpr.h>

// Nebulite
#include "Nebulite/Utility/Args/FuncTree.hpp"
#include "Nebulite/Math/ExpressionPrimitives.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Math {

// NOLINTNEXTLINE
double ExpressionPrimitives::rng2arg(double a, double b) {
    if (a<0) a = abs(a) * 125.5;
    if (b<0) b = abs(b) * 125.5;

    std::uint64_t seed = static_cast<uint64_t>(a * 73856093) ^ static_cast<uint64_t>(b * 19349663);
    seed = (seed ^ seed >> 30) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ seed >> 27) * 0x94d049bb133111eb;
    seed = seed ^ seed >> 31;
    return static_cast<double>(seed % 10000) / 10000.0; // Return a value between 0 and 1
}

// NOLINTNEXTLINE
double ExpressionPrimitives::rng3arg(double a, double b, double c) {
    if (a<0) a = abs(a) * 125.5;
    if (b<0) b = abs(b) * 125.5;
    if (c<0) c = abs(c) * 125.5;

    std::uint64_t seed = static_cast<uint64_t>(a * 73856093) ^ static_cast<uint64_t>(b * 19349663) ^ static_cast<uint64_t>(c * 83492791);
    seed = (seed ^ seed >> 30) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ seed >> 27) * 0x94d049bb133111eb;
    seed = seed ^ seed >> 31;
    return static_cast<double>(seed % 10000) / 10000.0; // Return a value between 0 and 1
}

// NOLINTNEXTLINE
double ExpressionPrimitives::rng2argInt16(double a, double b) {
    if (a<0) a = abs(a) * 125.5;
    if (b<0) b = abs(b) * 125.5;

    std::uint64_t seed = static_cast<uint64_t>(a * 73856093) ^ static_cast<uint64_t>(b * 19349663);
    seed = (seed ^ seed >> 30) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ seed >> 27) * 0x94d049bb133111eb;
    seed = seed ^ seed >> 31;
    return static_cast<double>(seed % 32768); // Return a value between 0 and 32767
}

// NOLINTNEXTLINE
double ExpressionPrimitives::rng3argInt16(double a, double b, double c) {
    if (a<0) a = abs(a) * 125.5;
    if (b<0) b = abs(b) * 125.5;
    if (c<0) c = abs(c) * 125.5;

    std::uint64_t seed = static_cast<uint64_t>(a * 73856093) ^ static_cast<uint64_t>(b * 19349663) ^ static_cast<uint64_t>(c * 83492791);
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

void ExpressionPrimitives::help(std::span<std::string_view const> const& args) {
    // Create a temporary funcTree to utilize its printFunctionList method for formatted output

    Utility::Args::FuncTree tempFuncTree("Nebulite Expressions", true, true, Global::capture()); // Pass to main capture

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
        {.name=gtName, .description=gtDesc, .pointer=reinterpret_cast<void*>(gt), .type=TE_FUNCTION2, .context=nullptr},
        {.name=ltName, .description=ltDesc, .pointer=reinterpret_cast<void*>(lt), .type=TE_FUNCTION2, .context=nullptr},
        {.name=geqName, .description=geqDesc, .pointer=reinterpret_cast<void*>(geq), .type=TE_FUNCTION2, .context=nullptr},
        {.name=leqName, .description=leqDesc, .pointer=reinterpret_cast<void*>(leq), .type=TE_FUNCTION2, .context=nullptr},
        {.name=eqName, .description=eqDesc, .pointer=reinterpret_cast<void*>(eq), .type=TE_FUNCTION2, .context=nullptr},
        {.name=neqName, .description=neqDesc, .pointer=reinterpret_cast<void*>(neq), .type=TE_FUNCTION2, .context=nullptr},

        // Logical gate functions
        {.name=logicalNotName, .description=logicalNotDesc, .pointer=reinterpret_cast<void*>(logical_not), .type=TE_FUNCTION1, .context=nullptr},
        {.name=logicalAndName, .description=logicalAndDesc, .pointer=reinterpret_cast<void*>(logical_and), .type=TE_FUNCTION2, .context=nullptr},
        {.name=logicalOrName, .description=logicalOrDesc, .pointer=reinterpret_cast<void*>(logical_or), .type=TE_FUNCTION2, .context=nullptr},
        {.name=logicalXorName, .description=logicalXorDesc, .pointer=reinterpret_cast<void*>(logical_xor), .type=TE_FUNCTION2, .context=nullptr},
        {.name=logicalNandName, .description=logicalNandDesc, .pointer=reinterpret_cast<void*>(logical_nand), .type=TE_FUNCTION2, .context=nullptr},
        {.name=logicalNorName, .description=logicalNorDesc, .pointer=reinterpret_cast<void*>(logical_nor), .type=TE_FUNCTION2, .context=nullptr},
        {.name=logicalXnorName, .description=logicalXnorDesc, .pointer=reinterpret_cast<void*>(logical_xnor), .type=TE_FUNCTION2, .context=nullptr},

        // Other logical functions
        {.name=toBipolarName, .description=toBipolarDesc, .pointer=reinterpret_cast<void*>(toBipolar), .type=TE_FUNCTION1, .context=nullptr},

        // Mapping functions
        {.name=mapName, .description=mapDesc, .pointer=reinterpret_cast<void*>(map), .type=TE_FUNCTION5, .context=nullptr},
        {.name=constrainName, .description=constrainDesc, .pointer=reinterpret_cast<void*>(constrain), .type=TE_FUNCTION3, .context=nullptr},

        // Maximum and Minimum functions
        {.name=maxName, .description=maxDesc, .pointer=reinterpret_cast<void*>(max), .type=TE_FUNCTION2, .context=nullptr},
        {.name=minName, .description=minDesc, .pointer=reinterpret_cast<void*>(min), .type=TE_FUNCTION2, .context=nullptr},

        // Rounding
        {.name=roundName, .description=roundDesc, .pointer=reinterpret_cast<void*>(round), .type=TE_FUNCTION2, .context=nullptr},
        {.name=roundUpName, .description=roundUpDesc, .pointer=reinterpret_cast<void*>(roundUp), .type=TE_FUNCTION2, .context=nullptr},
        {.name=roundDownName, .description=roundDownDesc, .pointer=reinterpret_cast<void*>(roundDown), .type=TE_FUNCTION2, .context=nullptr},

        // More mathematical functions
        {.name=sgnName, .description=sgnDesc, .pointer=reinterpret_cast<void*>(sgn), .type=TE_FUNCTION1, .context=nullptr},

        // RNG functions
        {.name=rng2argName,.description=rng2argDesc,.pointer=reinterpret_cast<void*>(rng2arg), .type=TE_FUNCTION2, .context=nullptr},
        {.name=rng3argName,.description=rng3argDesc,.pointer=reinterpret_cast<void*>(rng3arg), .type=TE_FUNCTION3, .context=nullptr},
        {.name=rng2argInt16Name,.description=rng2argInt16Desc,.pointer=reinterpret_cast<void*>(rng2argInt16), .type=TE_FUNCTION2, .context=nullptr},
        {.name=rng3argInt16Name,.description=rng3argInt16Desc,.pointer=reinterpret_cast<void*>(rng3argInt16), .type=TE_FUNCTION3, .context=nullptr},
    };
    return functions;
}

void ExpressionPrimitives::registerExpressions(std::vector<te_variable>& te_variables){
    for (const auto& funcInfo : availableFunctions()) {
        te_variables.push_back({.name=funcInfo.name, .address=funcInfo.pointer, .type=funcInfo.type, .context=funcInfo.context});
    }
}

} // namespace Nebulite::Math
