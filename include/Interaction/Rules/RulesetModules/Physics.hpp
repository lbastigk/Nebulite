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
} // namespace Interaction::Rules

//------------------------------------------
namespace Nebulite::Interaction::Rules::RulesetModules {
/**
 * @brief The Physics ruleset module, containing static rulesets related to physics.
 *        All rulesets here should be force-based physics simulations.
 *        Meaning each ruleset modifies the contexts force variables.
 *        After that, the local module ::physics::applyForce needs to be called to apply the accumulated forces.
 *        Make sure to call them each frame, otherwise the forces will accumulate indefinitely!
 */
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

    void elasticCollision(Context const& context);
    static constexpr std::string_view elasticCollisionName = "::physics::elasticCollision";

    void applyForce(Context const& context);
    static constexpr std::string_view applyForceName = "::physics::applyForce";

    //------------------------------------------
    // Constructor
    Physics() {
        // Global rulesets
        bind(RulesetType::Global, gravityName, &Physics::gravity);
        bind(RulesetType::Global, elasticCollisionName, &Physics::elasticCollision);

        // Local rulesets
        bind(RulesetType::Local, applyForceName, &Physics::applyForce);
    }

private:
    static constexpr std::string_view moduleName = "::physics";

    // Unique identifier for caching
    uint64_t const id = Nebulite::global().getUniqueId(std::string(moduleName), Core::GlobalSpace::UniqueIdType::expression);

    //------------------------------------------
    // Base values for physics framework

    // 1.) To retrieve from self and other using the ensureOrderedCacheList function

    /**
     * @brief List of keys for physics-related base values in the ordered cache list.
     */
    const std::vector<std::string> keys = {
        "physics.aX",
        "physics.aY",
        "physics.vX",
        "physics.vY",
        "physics.mass", // TODO: rename to physics.m . Renaming all json files keys is necessary.
        "physics.FX",
        "physics.FY",
        "posX",
        "posY",
        "sprite.sizeX",
        "sprite.sizeY"
    };

    /**
     * @enum Key
     * @brief Enumeration of keys corresponding to physics-related base values.
     *        Used for indexing into the ordered cache list.
     */
    enum class Key : std::size_t {
        physics_aX,
        physics_aY,
        physics_vX,
        physics_vY,
        physics_mass,
        physics_FX,
        physics_FY,
        posX,
        posY,
        spriteSizeX,
        spriteSizeY
    };

    /**
     * @brief Retrieves a base value from the ordered cache list for the given key.
     * @param v The ordered cache list of base values.
     * @param k The key corresponding to the desired base value.
     * @return A reference to the base value associated with the specified key.
     */
    inline static double& baseVal(double** v, Key k) noexcept {
        return *v[static_cast<std::size_t>(k)];
    }

    /**
     * @brief Retrieves the ordered cache list of base values for the given render object context.
     *        Instead of retrieving each value individually, this function fetches all required values in a single call.
     *        This reduces lookup overhead and improves performance when accessing multiple base values.
     * @param ctx The render object context from which to retrieve the base values.
     * @return A pointer to an array of double pointers, each pointing to a base value.
     */
    double** getBaseList(Nebulite::Core::RenderObject& ctx) {
        return ensureOrderedCacheList(*ctx.getDoc(), id, keys)->data();
    }

    // 2.) To retrieve from globalspace
    /**
     * @struct GlobalVal
     * @brief Struct to hold pointers to global variables used in physics calculations.
     */
    struct GlobalVal {
        double* G = Nebulite::global().getDoc()->getStableDoublePointer("physics.G");   // Gravitational constant
        double* dt = Nebulite::global().getDoc()->getStableDoublePointer("time.dt");    // Simulation delta time
        double* t  = Nebulite::global().getDoc()->getStableDoublePointer("time.t");     // Simulation time
        /* Add more global variables here as needed */
    } globalVal;
};
} // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_PHYSICS_HPP