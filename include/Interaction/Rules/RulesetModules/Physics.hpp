/**
 * @file Physics.hpp
 * @brief This file defines the Physics ruleset module, containing static rulesets related to physics.
 */

#ifndef NEBULITE_INTERACTION_RULES_RULESET_MODULES_PHYSICS_HPP
#define NEBULITE_INTERACTION_RULES_RULESET_MODULES_PHYSICS_HPP

//------------------------------------------
// Includes

// Standard library

// External

// Nebulite
#include "Interaction/Rules/RulesetModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Interaction::Rules {
struct Context;
}  // namespace Interaction::Rules

//------------------------------------------
namespace Nebulite::Interaction::Rules::RulesetModules {
class Physics : public RulesetModule {
public:
    //------------------------------------------
    // Functions

    void gravity(Context const& context);

    //------------------------------------------
    // Constructor
    Physics() {
        // Bind physics-related static rulesets here
        bind(RulesetType::Global, "::gravity", &Physics::gravity);
    }
};
}  // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_PHYSICS_HPP
