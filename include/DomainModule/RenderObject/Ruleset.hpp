/**
 * @file Ruleset.hpp
 * @brief Ruleset management functions for the domain RenderObject.
 */

#ifndef NEBULITE_RODM_RULESET_HPP
#define NEBULITE_RODM_RULESET_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
}

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
        subscription_size = domain->getDoc()->memberSize(Constants::keyName.renderObject.invokeSubscriptions);
        id = domain->getDoc()->get<uint32_t>(Constants::keyName.renderObject.id,0);
    }

    //------------------------------------------
    // Available Functions

    Constants::Error once(std::span<std::string const> const& args);
    static std::string const once_name;
    static std::string const once_desc;

    // TODO: Additional ruleset management for push/pop, enable/disable, list, etc.
    //       perhaps just an option to reload is enough?
    //       any push/pop could be done via json manipulation directly
    Constants::Error reload(std::span<std::string const> const& args);
    static std::string const reload_name;
    static std::string const reload_desc;

    //------------------------------------------
    // Category names
    static std::string const Ruleset_name;
    static std::string const Ruleset_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Ruleset) {
        // Bind functions
        bindFunction(&Ruleset::once, once_name,&once_desc);
    }

    // TODO: For some reason, only obj id2 is broadcasting??

private:
    // Size of subscriptions
    size_t subscription_size = 0;

    // Check if rulesets need to be reloaded
    bool reloadRulesets = true;

    // RenderObject id
    uint32_t id = 0;

    // Rulesets
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsGlobal; // Global rulesets, intended for self-other-global interaction
    std::vector<std::shared_ptr<Interaction::Rules::Ruleset>> rulesetsLocal; // Internal rulesets, intended for self-global interaction
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_RULESET_HPP