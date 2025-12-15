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

    // Global rulesets

    void elasticCollision(ContextBase const& context);
    static constexpr std::string_view elasticCollisionName = "::physics::elasticCollision";
    static constexpr std::string_view elasticCollisionDesc = "Applies elastic collision forces between two render objects based on their masses and velocities.";

    void gravity(ContextBase const& context);
    static constexpr std::string_view gravityName = "::physics::gravity";
    static constexpr std::string_view gravityDesc = "Applies gravitational force between two render objects based on their masses and the gravitational constant.";

    // Local rulesets

    void applyForce(ContextBase const& context);
    static constexpr std::string_view applyForceName = "::physics::applyForce";
    static constexpr std::string_view applyForceDesc = "Applies accumulated forces to the render object's acceleration, velocity, and position based on its mass and the simulation delta time.";

    void drag(ContextBase const& context);
    static constexpr std::string_view dragName = "::physics::drag";
    static constexpr std::string_view dragDesc = "Applies drag force to the render object, simulating air resistance based on its velocity and a drag coefficient.";

    //------------------------------------------
    // Constructor
    Physics() {
        // Global rulesets
        BIND_STATIC_ASSERT(RulesetType::Global, &Physics::elasticCollision, elasticCollisionName, elasticCollisionDesc);
        BIND_STATIC_ASSERT(RulesetType::Global, &Physics::gravity, gravityName, gravityDesc);

        // Local rulesets
        BIND_STATIC_ASSERT(RulesetType::Local, &Physics::applyForce, applyForceName, applyForceDesc);
        BIND_STATIC_ASSERT(RulesetType::Local, &Physics::drag, dragName, dragDesc);
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
        Nebulite::Constants::keyName.renderObject.positionX,
        Nebulite::Constants::keyName.renderObject.positionY,
        Nebulite::Constants::keyName.renderObject.pixelSizeX,
        Nebulite::Constants::keyName.renderObject.pixelSizeY,
        "physics.aX",
        "physics.aY",
        "physics.vX",
        "physics.vY",
        "physics.mass", // TODO: rename to physics.m . Renaming all json files keys is necessary.
        "physics.FX",
        "physics.FY",

    };

    /**
     * @enum Key
     * @brief Enumeration of keys corresponding to physics-related base values.
     *        Used for indexing into the ordered cache list.
     */
    enum class Key : std::size_t {
        posX,
        posY,
        spriteSizeX,
        spriteSizeY,
        physics_aX,
        physics_aY,
        physics_vX,
        physics_vY,
        physics_mass,
        physics_FX,
        physics_FY
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
    double** getBaseList(Interaction::Execution::DomainBase& ctx) {
        return ensureOrderedCacheList(*ctx.getDoc(), id, keys)->data();
    }

    // 2.) To retrieve from globalspace
    /**
     * @struct GlobalVal
     * @brief Struct to hold pointers to global variables used in physics calculations.
     */
    struct GlobalVal {
        double* G = Nebulite::global().getDoc()->getStableDoublePointer("physics.G"); // Gravitational constant
        double* dt = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.time_dt); // Simulation delta time
        double* t = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.time_t); // Simulation time
        /* Add more global variables here as needed */
    } globalVal;
};
} // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_PHYSICS_HPP