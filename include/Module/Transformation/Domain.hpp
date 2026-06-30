#ifndef MODULE_TRANSFORMATION_DOMAIN_HPP
#define MODULE_TRANSFORMATION_DOMAIN_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>

// Nebulite
#include "Interaction/Execution/FuncTree.hpp"
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Domain
 * @brief Various transformations that use short-lived domains to perform complex operations that would be difficult to achieve with pure transformations.
 */
class Domain final : public Base::TransformationModule {
public:
    explicit Domain(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool injectScript(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr injectScriptName = "injectScript";
    static auto constexpr injectScriptDesc = "Injects a nebulite script to modify the json doc.\n"
        "Usage: |injectScript <path/to/script.nebs> -> {modified-json}\n";
};
} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_DOMAIN_HPP
