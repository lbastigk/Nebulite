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
    Physics();

private:
    static constexpr std::string_view moduleName = "::physics";

    //------------------------------------------
    // Base values for physics framework

    // 1.) To retrieve from self and other using the ensureOrderedCacheList function

    /**
     * @brief List of keys for physics-related base values in the ordered cache list.
     */
    const std::vector<std::string_view> keys = {
        Nebulite::Constants::KeyNames::RenderObject::positionX,
        Nebulite::Constants::KeyNames::RenderObject::positionY,
        Nebulite::Constants::KeyNames::RenderObject::pixelSizeX,
        Nebulite::Constants::KeyNames::RenderObject::pixelSizeY,
        "physics.aX",
        "physics.aY",
        "physics.vX",
        "physics.vY",
        "physics.mass", // TODO: rename to physics.m . Renaming all json files keys is necessary.
        "physics.FX",
        "physics.FY"
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

    // 2.) To retrieve from globalspace
    /**
     * @struct GlobalVal
     * @brief Struct to hold pointers to global variables used in physics calculations.
     */
    struct GlobalVal {
        double* G; // Gravitational constant
        double* dt; // Simulation delta time
        double* t; // Simulation time
        /* Add more global variables here as needed */
    } globalVal = {};
};
} // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_PHYSICS_HPP
