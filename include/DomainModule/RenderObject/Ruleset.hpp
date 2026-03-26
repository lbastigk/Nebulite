/**
 * @file Ruleset.hpp
 * @brief Contains the Ruleset DomainModule for the RenderObject tree.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDEROBJECT_RULESET_HPP
#define NEBULITE_DOMAINMODULE_RENDEROBJECT_RULESET_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
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
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override ;

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event once(std::span<std::string const> const& args) const ;
    static auto constexpr once_name = "ruleset once";
    static auto constexpr once_desc = "Applies all rulesets once on the next update\n"
        "\n"
        "Usage: ruleset once\n"
        "\n"
        "All rulesets are applied once on the next update cycle.\n";

    [[nodiscard]] Constants::Event reload();
    static auto constexpr reload_name = "ruleset reload";
    static auto constexpr reload_desc = "Reloads all rulesets for this RenderObject on the next update.\n"
        "\n"
        "Usage: ruleset reload\n"
        "\n"
        "All rulesets are re-evaluated and reloaded on the next update cycle.\n";

    //------------------------------------------
    // Category names
    static auto constexpr ruleset_name = "ruleset";
    static auto constexpr ruleset_desc = "Ruleset management functions for the RenderObject domain.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Ruleset) {
        // Bind functions
        bindCategory(ruleset_name, ruleset_desc);
        bindFunction(&Ruleset::reload, reload_name, reload_desc);
        bindFunction(&Ruleset::once, once_name, once_desc);
    }

    struct Key : Data::KeyGroup<"ruleset."> {
        // No keys for now
    };

private:
    // Check if the module is initialized and ready to use
    bool initialized = false;

    // Size of subscriptions
    size_t subscription_size = 0;

    // Check if rulesets need to be reloaded
    bool reloadRulesets = true;

    // Global rulesets, intended for self-other-global interaction
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsGlobal;

    // Internal rulesets, intended for self-global interaction
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsLocal;

    // Topic subscriptions
    std::vector<std::shared_ptr<Interaction::Rules::Listener>> listeners;
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_DOMAINMODULE_RENDEROBJECT_RULESET_HPP
