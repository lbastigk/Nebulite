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
    /**
     * @brief Override of update.
     */
    Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    Constants::Error once(std::span<std::string const> const& args);
    static std::string const once_name;
    static std::string const once_desc;

    // TODO: Additional ruleset management for push/pop, enable/disable, list, etc.

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
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_RULESET_HPP