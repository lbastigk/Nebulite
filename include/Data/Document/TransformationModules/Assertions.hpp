/**
 * @file Assertions.hpp
 * @brief Class for assertion transformation modules.
 */

#ifndef NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_ASSERTIONS_HPP
#define NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_ASSERTIONS_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Data::TransformationModules {

class Assertions final : public TransformationModule {
public:
    explicit Assertions(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool assertNonEmpty(Core::JsonScope* jsonDoc);
    static auto constexpr assertNonEmptyName = "assertNonEmpty";
    static auto constexpr assertNonEmptyDesc = "Asserts that the current JSON value is non-empty.\n"
        "If the value is empty, the transformation fails and the program exits\n"
        "Usage: |assertNonEmpty -> {value,<Exception thrown if empty>}\n";
};
} // namespace Nebulite::Data::TransformationModules
#endif // NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_ASSERTIONS_HPP
