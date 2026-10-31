#ifndef NEBULITE_MODULE_TRANSFORMATION_BOOLEAN_HPP
#define NEBULITE_MODULE_TRANSFORMATION_BOOLEAN_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>

// Nebulite
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Boolean
 * @brief Boolean transformation utilities
 * @details **WORK IN PROGRESS**
 */
class Boolean final : public Base::TransformationModule {
public:
    explicit Boolean(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool booleanNot(Data::JsonScope& jsonDoc);
    static auto constexpr booleanNotName = "not";
    static auto constexpr booleanNotDesc = "Logical NOT operation on the current JSON value.\n"
        "Usage: |not -> {value:bool}\n"
        "If the given value is not convertible to bool, the value is assumed to be false and is set to true";

    static bool booleanToSigned(Data::JsonScope& jsonDoc);
    static auto constexpr booleanToSignedName = "booleanToSigned";
    static auto constexpr booleanToSignedDesc = "Converts the root boolean value to 1 (true) or -1 (false)"
        "Usage: |booleanToSigned -> {value:int}\n";

    static bool booleanAnd(Data::JsonScope& jsonDoc);
    static auto constexpr booleanAndName = "and";
    static auto constexpr booleanAndDesc = "Logical and operation on the current JSON array values.\n"
        "Input must be an array.\n"
        "Usage: |and -> {value:bool}\n";

    static bool booleanOr(Data::JsonScope& jsonDoc);
    static auto constexpr booleanOrName = "or";
    static auto constexpr booleanOrDesc = "Logical or operation on the current JSON array values.\n"
        "Input must be an array.\n"
        "Usage: |or -> {value:bool}\n";

    static bool booleanXor(Data::JsonScope& jsonDoc);
    static auto constexpr booleanXorName = "xor";
    static auto constexpr booleanXorDesc = "Logical xor operation on the current JSON array values.\n"
        "Input must be an array.\n"
        "Usage: |xor -> {value:bool}\n";

    static bool booleanNand(Data::JsonScope& jsonDoc);
    static auto constexpr booleanNandName = "nand";
    static auto constexpr booleanNandDesc = "Logical nand operation on the current JSON array values.\n"
        "Input must be an array.\n"
        "Usage: |nand -> {value:bool}\n";

    static bool booleanNor(Data::JsonScope& jsonDoc);
    static auto constexpr booleanNorName = "nor";
    static auto constexpr booleanNorDesc = "Logical nor operation on the current JSON array values.\n"
        "Input must be an array.\n"
        "Usage: |nor -> {value:bool}\n";

    static bool booleanXnor(Data::JsonScope& jsonDoc);
    static auto constexpr booleanXnorName = "xnor";
    static auto constexpr booleanXnorDesc = "Logical xnor operation on the current JSON array values.\n"
        "Input must be an array.\n"
        "Usage: |xnor -> {value:bool}\n";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_BOOLEAN_HPP
