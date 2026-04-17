/**
 * @file Physics.hpp
 * @brief This file defines the Physics ruleset module, containing static rulesets related to physics.
 */

#ifndef NEBULITE_RULESET_MODULE_PHYSICS_HPP
#define NEBULITE_RULESET_MODULE_PHYSICS_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Module/Domain/GlobalSpace/Physics.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

//------------------------------------------
namespace Nebulite::RulesetModule {
/**
 * @brief The Physics ruleset module, containing static rulesets related to physics.
 * @details All rulesets here should be force-based physics simulations.
 *          Meaning each ruleset modifies the contexts force variables.
 *          After that, the local module ::physics::applyForce needs to be called to apply the accumulated forces.
 *          Make sure to call them each frame, otherwise the forces will accumulate indefinitely!
 */
class Physics : public Interaction::Rules::RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Global rulesets

    void elasticCollision(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr elasticCollisionName = "::physics::elasticCollision";
    static std::string_view constexpr elasticCollisionDesc = "Applies elastic collision velocity corrections between two RenderObjects based on their masses and velocities.";

    void gravity(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr gravityName = "::physics::gravity";
    static std::string_view constexpr gravityDesc = "Applies gravitational force between two render objects based on their masses and the gravitational constant.";

    // Local rulesets

    void storeLastPosition(Interaction::Context const& context, double**& slf, double**& otr) const;
    static std::string_view constexpr storeLastPositionName = "::physics::storeLastPosition";
    static std::string_view constexpr storeLastPositionDesc = "Stores the current position. Should be called before ::physics::applyForce.";

    void applyForce(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr applyForceName = "::physics::applyForce";
    static std::string_view constexpr applyForceDesc = "Applies accumulated forces to the render object's acceleration, velocity, and position based on its mass and the simulation delta time.";

    void applyCorrection(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr applyCorrectionName = "::physics::applyCorrection";
    static std::string_view constexpr applyCorrectionDesc = "Applies position and velocity corrections to resolve overlaps and prevent tunneling.";

    void drag(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr dragName = "::physics::drag";
    static std::string_view constexpr dragDesc = "Applies drag force to the render object, simulating air resistance based on its velocity and a drag coefficient.";

    //------------------------------------------
    // Constructor
    Physics();

    static std::string_view constexpr moduleName = "::physics";

private:
    //------------------------------------------
    // Base values for physics framework

    // 1.) To retrieve from self and other using the ensureOrderedCacheList function

    /**
     * @brief List of keys for per-object physics-related base values in the ordered cache list.
     */
    const std::vector<Data::ScopedKeyView> baseKeys = {
        // Base values for size
        Constants::KeyNames::RenderObject::positionX,
        Constants::KeyNames::RenderObject::positionY,
        Constants::KeyNames::RenderObject::sizeX,
        Constants::KeyNames::RenderObject::sizeY,
        Constants::KeyNames::RenderObject::sizeR,
        // Base Physics values
        DomainModule::GlobalSpace::Physics::Key::Local::aX,
        DomainModule::GlobalSpace::Physics::Key::Local::aY,
        DomainModule::GlobalSpace::Physics::Key::Local::vX,
        DomainModule::GlobalSpace::Physics::Key::Local::vY,
        DomainModule::GlobalSpace::Physics::Key::Local::m,
        DomainModule::GlobalSpace::Physics::Key::Local::FX,
        DomainModule::GlobalSpace::Physics::Key::Local::FY,
        // Correction values
        DomainModule::GlobalSpace::Physics::Key::Local::Correction::X,
        DomainModule::GlobalSpace::Physics::Key::Local::Correction::Y,
        DomainModule::GlobalSpace::Physics::Key::Local::Correction::vX,
        DomainModule::GlobalSpace::Physics::Key::Local::Correction::vY,
        // More specialized keys
        DomainModule::GlobalSpace::Physics::Key::Local::lastCollisionTimeX,
        DomainModule::GlobalSpace::Physics::Key::Local::lastCollisionTimeY,
        DomainModule::GlobalSpace::Physics::Key::Local::lastPositionX,
        DomainModule::GlobalSpace::Physics::Key::Local::lastPositionY
    };

    /**
     * @enum Key
     * @brief Enumeration of keys corresponding to physics-related base values.
     *        Used for indexing into the ordered cache list.
     */
    enum class Key : std::size_t {
        // Base values for size
        posX,
        posY,
        sizeX,
        sizeY,
        sizeR,
        // Base Physics values
        physics_aX,
        physics_aY,
        physics_vX,
        physics_vY,
        physics_mass,
        physics_FX,
        physics_FY,
        // Correction values
        physics_correction_X,
        physics_correction_Y,
        physics_correction_vX,
        physics_correction_vY,
        // More specialized keys
        physics_lastCollisionX,
        physics_lastCollisionY,
        physics_lastPositionX,
        physics_lastPositionY,
    };

    // 2.) To retrieve from GlobalSpace

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
} // namespace Nebulite::RulesetModule
#endif // NEBULITE_RULESET_MODULE_PHYSICS_HPP
