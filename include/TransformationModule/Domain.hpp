/**
 * @file Domain.hpp
 * @brief Class for domain transformation functions.
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
    
    static bool parse(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr parseName = "parse";
    static auto constexpr parseDesc = "Parses a Nebulite Script command on the JSON\n"
        "Supports argument wrapped in {! ... } for safety with Expression evaluation\n"
        "Usage: |parse <command> -> {value}\n";
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_DOMAIN_HPP
