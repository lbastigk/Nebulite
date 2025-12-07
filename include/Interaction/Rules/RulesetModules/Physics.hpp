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
    static constexpr std::string_view gravityName = "::physics::gravity";

    /**
     * @todo Both Gravity Rulesets, static and nonstatic, should add forces
     *       Just like any other ruleset, we should always work backwards and apply a force addition
     *       Then, we call ::physics::update to apply the accumulated forces to acceleration, velocity, and position
     *       This way, we can have multiple rulesets adding forces without interfering with each other
     *       Do the same for elastic collision, may be harder as we have to calculate backwards:
     *       we know the new v, and need to use dt to calculate the force that caused it
     */

    //void elasticCollision(Context const& context);
    //static constexpr std::string_view elasticCollisionName;

    //void applyForce(Context const& context);
    //static constexpr std::string_view updateName;

    //------------------------------------------
    // Constructor
    Physics() {
        // Bind physics-related static rulesets here
        bind(RulesetType::Global, gravityName, &Physics::gravity);
    }
private:
    static constexpr std::string_view moduleName = "::physics";

    // Unique identifier for caching
    uint64_t const id = Nebulite::global().getUniqueId(std::string(moduleName), Core::GlobalSpace::UniqueIdType::expression);

    //------------------------------------------
    // Base values for physics framework

    // 1.) To retrieve from self and other using the ensureOrderedCacheList function

    // Variable context Keys being used for both entities
    const std::vector<std::string> keys = {
        "physics.aX",
        "physics.aY",
        "physics.mass",
        "posX",
        "posY"
    };
    enum class Key : std::size_t {
        physics_aX,
        physics_aY,
        physics_mass,
        posX,
        posY
    };
    double& baseVal(double** v, Key k) noexcept {
        return *v[static_cast<std::size_t>(k)];
    };

    double** getBaseList(Nebulite::Core::RenderObject& ctx) {
        return ensureOrderedCacheList(*ctx.getDoc(), id, keys)->data();
    }

    // 2.) To retrieve from globalspace
    struct GlobalVal {
        double* G = Nebulite::global().getDoc()->getStableDoublePointer("physics.G");
    } globalVal;

};
}  // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_PHYSICS_HPP
