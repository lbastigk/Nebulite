/**
 * @file Boolean.hpp
 * @brief Class for boolean transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_BOOLEAN_HPP
#define NEBULITE_TRANSFORMATION_MODULE_BOOLEAN_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Boolean final : public Data::TransformationModule {
public:
    explicit Boolean(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    // TODO: not, and, or, xor, nand, nor, xnor, etc.
    //       All array-based, make sure input is array before processing
    //       For this to work, we need the ensureArray transformation from Array module

    // TODO: boolean to signed: true -> 1, false -> -1
    //       boolean to unsigned: true -> 1, false -> 0
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_BOOLEAN_HPP
