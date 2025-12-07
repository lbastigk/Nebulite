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
    static std::string gravityName;

    /**
     * @todo Both Gravity Rulesets, static and nonstatic, should add forces
     *       Just like any other ruleset, we should always work backwards and apply a force addition
     *       Then, we call ::physics::update to apply the accumulated forces to acceleration, velocity, and position
     *       This way, we can have multiple rulesets adding forces without interfering with each other
     *       Do the same for elastic collision, may be harder as we have to calculate backwards:
     *       we know the new v, and need to use dt to calculate the force that caused it
     */

    //void elasticCollision(Context const& context);
    //static std::string elasticCollisionName;

    //void update(Context const& context);
    //static std::string updateName;



    //------------------------------------------
    // Constructor
    Physics() {
        // Bind physics-related static rulesets here
        bind(RulesetType::Global, gravityName, &Physics::gravity);
    }
};
}  // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_PHYSICS_HPP
