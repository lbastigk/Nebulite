/**
 * @file Domain.hpp
 * @brief Class for domain transformation modules.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_DOMAIN_HPP
#define NEBULITE_TRANSFORMATION_MODULE_DOMAIN_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Domain final : public Data::TransformationModule {
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
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_DOMAIN_HPP
