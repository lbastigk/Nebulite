/**
 * @file General.hpp
 * @brief Class for basic transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_CORE_HPP
#define NEBULITE_TRANSFORMATION_MODULE_CORE_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class General final : public Data::TransformationModule {
public:
    explicit General(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    // TODO: setString: setString <key> <value>
    // TODO: setInt: setInt <key> <value>
    // TODO: setDouble: setDouble <key> <value>
    // TODO: setBool: setBool <key> <value>
    // TODO: setNull: setNull <key>

    // TODO: removeKey: removeKey <key>

    // TODO: setFromResult: setFromResult <key> <transformation>
    //       allows us to do transformations while keeping the original document structure:
    //       e.g.: {global.obj|setFromResult __tmp__ complexFilteringTransformation|someTransformationBasedOnTmpValue}
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_CORE_HPP
