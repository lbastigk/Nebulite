/**
 * @file Movement.hpp
 * @brief This file defines the Movement ruleset module, containing static rulesets related to movement.
 */

#ifndef NEBULITE_RULESET_MODULE_MOVEMENT_HPP
#define NEBULITE_RULESET_MODULE_MOVEMENT_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Module/Domain/GlobalSpace/Physics.hpp"
#include "Interaction/Rules/RulesetModule.hpp"

//------------------------------------------
namespace Nebulite::RulesetModule {
/**
 * @brief The Movement ruleset module, containing static rulesets related to movement and collision clipping
 */
class Movement : public Interaction::Rules::RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Global rulesets

    void detectClipping(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr detectClippingName = "::movement::detectClipping";
    static std::string_view constexpr detectClippingDesc = "Global ruleset to detect the closest object in each direction. The listeners distance is set.";

    void processClipping(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr processClippingName = "::movement::processClipping";
    static std::string_view constexpr processClippingDesc = "Local ruleset to process collision clipping for the self entry based on position delta and closest objects.\n"
        "Call this ruleset after ::physics::applyForce. Requires ::physics::storeLastPosition to be called before ::physics::applyForce.";

    //------------------------------------------
    // Constructor
    Movement();

    static std::string_view constexpr moduleName = "::movement";
private:
    //------------------------------------------
    // Base values for movement framework

    // 1.) To retrieve from self and other using the ensureOrderedCacheList function

    /**
     * @brief List of keys for per-object movement-related base values in the ordered cache list.
     */
    const std::vector<Data::ScopedKeyView> baseKeys = {
        // Position and size
        Constants::KeyNames::RenderObject::positionX,
        Constants::KeyNames::RenderObject::positionY,
        Constants::KeyNames::RenderObject::sizeX,
        Constants::KeyNames::RenderObject::sizeY,
        Constants::KeyNames::RenderObject::sizeR,
        // Physics
        DomainModule::GlobalSpace::Physics::Key::Local::vX,
        DomainModule::GlobalSpace::Physics::Key::Local::vY,
        DomainModule::GlobalSpace::Physics::Key::Local::m,
        DomainModule::GlobalSpace::Physics::Key::Local::FX,
        DomainModule::GlobalSpace::Physics::Key::Local::FY,
        // Closest X/Y
        Data::ScopedKeyView("movement.clip.closest.N"),
        Data::ScopedKeyView("movement.clip.closest.E"),
        Data::ScopedKeyView("movement.clip.closest.S"),
        Data::ScopedKeyView("movement.clip.closest.W"),
        // X/Y last pos
        DomainModule::GlobalSpace::Physics::Key::Local::lastPositionX,
        DomainModule::GlobalSpace::Physics::Key::Local::lastPositionY
    };

    /**
     * @enum Key
     * @brief Enumeration of keys corresponding to movement-related base values.
     *        Used for indexing into the ordered cache list.
     */
    enum class Key : std::size_t {
        // Position and size
        posX,
        posY,
        sizeX,
        sizeY,
        sizeR,
        // Physics
        physics_vX,
        physics_vY,
        physics_mass,
        physics_FX,
        physics_FY,
        // Closest X/Y
        clip_closest_N,
        clip_closest_E,
        clip_closest_S,
        clip_closest_W,
        // X/Y last
        position_last_X,
        position_last_Y
    };

    // 2.) To retrieve from globalspace
    /**
     * @struct GlobalVal
     * @brief Struct to hold pointers to global variables used in movement calculations.
     */
    struct GlobalVal {
        /* Add more global variables here as needed */
        double* dt = nullptr; // Time step for movement calculations
    } globalVal = {};

    struct Radius {
        double& slf;
        double& otr;

        Radius(double**& slfBase, double**& otrBase)
        : slf(baseVal(slfBase, Key::sizeR)), otr(baseVal(otrBase, Key::sizeR))
        {}
    };
};
} // namespace Nebulite::RulesetModule
#endif // NEBULITE_RULESET_MODULE_MOVEMENT_HPP
