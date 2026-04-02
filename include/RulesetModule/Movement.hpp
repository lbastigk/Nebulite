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
#include "DomainModule/GlobalSpace/Physics.hpp"
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

    void clip(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static constexpr std::string_view clipName = "::movement::clip";
    static constexpr std::string_view clipDesc = "Global ruleset to handle collision clipping between entities. The entry listening to this ruleset will be placed accordingly.";

    //------------------------------------------
    // Constructor
    Movement();

    static constexpr std::string_view moduleName = "::movement";
private:
    //------------------------------------------
    // Base values for movement framework

    // 1.) To retrieve from self and other using the ensureOrderedCacheList function

    /**
     * @brief List of keys for per-object movement-related base values in the ordered cache list.
     */
    const std::vector<Data::ScopedKeyView> baseKeys = {
        Constants::KeyNames::RenderObject::positionX,
        Constants::KeyNames::RenderObject::positionY,
        Constants::KeyNames::RenderObject::sizeX,
        Constants::KeyNames::RenderObject::sizeY,
        Constants::KeyNames::RenderObject::sizeR,
        DomainModule::GlobalSpace::Physics::Key::Local::vX,
        DomainModule::GlobalSpace::Physics::Key::Local::vY,
        DomainModule::GlobalSpace::Physics::Key::Local::m,
        DomainModule::GlobalSpace::Physics::Key::Local::FX,
        DomainModule::GlobalSpace::Physics::Key::Local::FY
    };

    /**
     * @enum Key
     * @brief Enumeration of keys corresponding to movement-related base values.
     *        Used for indexing into the ordered cache list.
     */
    enum class Key : std::size_t {
        posX,
        posY,
        sizeX,
        sizeY,
        sizeR,
        physics_vX,
        physics_vY,
        physics_mass,
        physics_FX,
        physics_FY,
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

    void collisionCircleCircle(Interaction::Context const& context, double**& slf, double**& otr) const ;
    void collisionCircleBox(Interaction::Context const& context, double**& slf, double**& otr) const ;
    void collisionBoxCircle(Interaction::Context const& context, double**& slf, double**& otr) const ;
    void collisionBoxBox(Interaction::Context const& context, double**& slf, double**& otr) const ;
};
} // namespace Nebulite::RulesetModule
#endif // NEBULITE_RULESET_MODULE_MOVEMENT_HPP
