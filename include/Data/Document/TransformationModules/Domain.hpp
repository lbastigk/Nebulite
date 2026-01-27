/**
 * @file Domain.hpp
 * @brief Class for domain transformation modules.
 */

#ifndef NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_DOMAIN_HPP
#define NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_DOMAIN_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Data::TransformationModules {

class Domain final : public TransformationModule {
public:
    explicit Domain(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool nebs(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr nebsName = "nebs";
    static auto constexpr nebsDesc = "Parses a Nebulite Script command on the JSON\n"
        "Usage: |nebs <command> -> {value}\n";
};
} // namespace Nebulite::Data::TransformationModules
#endif // NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_DOMAIN_HPP
