/**
 * @file Ruleset.hpp
 * @brief Contains the declaration of the Ruleset DomainModule for the GlobalSpace domain.
 */

#ifndef MODULE_DOMAIN_COMMON_RULESET_HPP
#define MODULE_DOMAIN_COMMON_RULESET_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
/**
 * @class Nebulite::Module::Domain::Common::Ruleset
 * @brief DomainModule for managing rulesets.
 */
class Ruleset final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event invokeOnce(std::span<std::string_view const> const& args) const ;
    static auto constexpr invokeOnceName = "invoke";
    static auto constexpr invokeOnceDesc = "Applies all given rulesets once on the next update\n"
        "\n"
        "Usage: invoke <list>\n"
        "\n"
        "Use invoke ::help to list all available static rulesets."
        "All rulesets are applied once on the next update cycle.\n";

    //------------------------------------------
    // Categories

    static auto constexpr ruleset_name = "ruleset";
    static auto constexpr ruleset_desc = "Functions for managing rulesets in the GlobalSpace.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit Ruleset(ConstructorParams const& params) : DomainModule(params) {
        //------------------------------------------
        // Binding functions to the FuncTree
        bindFunction(&Ruleset::invokeOnce, invokeOnceName, invokeOnceDesc);
    }
};
} // namespace Nebulite::Module::Domain::Common
#endif // MODULE_DOMAIN_COMMON_RULESET_HPP
