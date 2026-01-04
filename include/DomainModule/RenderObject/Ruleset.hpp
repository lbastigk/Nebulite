/**
 * @file Ruleset.hpp
 * @brief Contains the Ruleset DomainModule for the RenderObject tree.
 */

#ifndef NEBULITE_RODM_RULESET_HPP
#define NEBULITE_RODM_RULESET_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Constants/KeyNames.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::Ruleset
 * @brief Ruleset management for the RenderObject tree DomainModule.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Ruleset) {
public:
    Constants::Error update() override;
    void reinit() override {
        reloadRulesets = true;
        subscription_size = getDoc().memberSize(Constants::KeyNames::RenderObject::Ruleset::listen);
        id = getDoc().get<uint32_t>(Constants::KeyNames::RenderObject::id,0);
    }

    //------------------------------------------
    // Available Functions

    Constants::Error once(std::span<std::string const> const& args);
    static std::string_view constexpr once_name = "ruleset once";
    static std::string_view constexpr once_desc = "Applies all rulesets once on the next update\n"
        "\n"
        "Usage: ruleset once\n"
        "\n"
        "All rulesets are applied once on the next update cycle.\n";

    // TODO: Additional ruleset management for push/pop, enable/disable, list, etc.
    //       perhaps just an option to reload is enough?
    //       any push/pop could be done via json manipulation directly

    Constants::Error reload();
    static std::string_view constexpr reload_name = "ruleset reload";
    static std::string_view constexpr reload_desc = "Reloads all rulesets for this RenderObject on the next update.\n"
        "\n"
        "Usage: ruleset reload\n"
        "\n"
        "All rulesets are re-evaluated and reloaded on the next update cycle.\n";

    //------------------------------------------
    // Category names
    static std::string_view constexpr ruleset_name = "ruleset";
    static std::string_view constexpr ruleset_desc = "Ruleset management functions for the RenderObject domain.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Ruleset) {
        // Bind functions
        (void)bindCategory(ruleset_name, ruleset_desc);
        bindFunction(&Ruleset::reload, reload_name, reload_desc);
        bindFunction(&Ruleset::once, once_name, once_desc);
    }

private:
    // Size of subscriptions
    size_t subscription_size = 0;

    // Check if rulesets need to be reloaded
    bool reloadRulesets = true;

    // RenderObject id
    uint32_t id = 0;

    // Global rulesets, intended for self-other-global interaction
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsGlobal;

    // Internal rulesets, intended for self-global interaction
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsLocal;
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_RULESET_HPP
