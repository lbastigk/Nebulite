/**
 * @file Boolean.hpp
 * @brief Class for boolean transformation functions.
 */

#ifndef MODULE_TRANSFORMATION_BOOLEAN_HPP
#define MODULE_TRANSFORMATION_BOOLEAN_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Boolean
 * @brief Boolean transformation utilities
 * @details **WORK IN PROGRESS**
 */
class Boolean final : public Base::TransformationModule {
public:
    explicit Boolean(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool booleanNot(Data::JsonScope* jsonDoc);
    static auto constexpr booleanNotName = "not";
    static auto constexpr booleanNotDesc = "Logical NOT operation on the current JSON value.\n"
        "Usage: |not -> {value:bool}\n"
        "If the given value is not convertible to bool, the value is assumed to be false and is set to true";

    // TODO: and, or, xor, nand, nor, xnor, etc.
    //       All array-based, make sure input is array before processing
    //       For this to work, we need the ensureArray transformation from Array module

    // TODO: boolean to signed: true -> 1, false -> -1
    //       boolean to unsigned: true -> 1, false -> 0
};
} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_BOOLEAN_HPP
