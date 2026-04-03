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

    // TODO: Needs rework. The idea of clipping + box Edge sliding doesn't work...
    //       New idea:
    //       1.) ::movement::detectClipping determines 8 different collisions:
    //       - north, east, south, west
    //       - cornerNW, cornerNE, cornerSE, cornerSW
    //       2.) ::movement::processClipping (local) then sets XY forces accordingly:
    //       if only one corner is active and no other normal direction -> edge sliding
    //       if any normal direction is active -> normal clipping
    //       That means:
    //       CNW   N    CNE
    //       W    SLF      E   <- OTR
    //       CSW   S    CSE
    //       Then we set forces accordingly. For safety we may only set edge-sliding forces if there are no forces present in that direction?
    //       N,S -> FY = 0
    //       E,W -> FX = 0
    //       CNW -> FY += dF if FX != 0, FX -= dF if FY != 0
    //       CNE -> FY += dF if FX != 0, FX += dF if FY != 0
    //       CSW -> FY -= dF if FX != 0, FX -= dF if FY != 0
    //       CSE -> FY -= dF if FX != 0, FX += dF if FY != 0
    //       3.) after that, we apply the forces using ::physics::applyForce

    void detectClipping(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr detectClippingName = "::movement::detectClipping";
    static std::string_view constexpr detectClippingDesc = "Global ruleset to detect collision clipping between entities based on current forces. Sets detection directions and corners to true if a collision is detected.";

    void processClipping(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr processClippingName = "::movement::processClipping";
    static std::string_view constexpr processClippingDesc = "Local ruleset to process collision clipping for the self entry based on detected clipping directions and corners. Adjusts forces to achieve clipping and edge sliding behavior.";

    void clip(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr clipName = "::movement::clip";
    static std::string_view constexpr clipDesc = "Global ruleset to handle collision clipping between entities. Affects forces of the entry listening to this ruleset.";

    void boxEdgeSliding(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr boxEdgeSlidingName = "::movement::boxEdgeSliding";
    static std::string_view constexpr boxEdgeSlidingDesc = "Global ruleset to handle edge sliding for box collisions. Affects forces of the entry listening to this ruleset.";

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
        // Clipping
        Data::ScopedKeyView("movement.clip.direction.north"),
        Data::ScopedKeyView("movement.clip.direction.east"),
        Data::ScopedKeyView("movement.clip.direction.south"),
        Data::ScopedKeyView("movement.clip.direction.west"),
        Data::ScopedKeyView("movement.clip.corner.NW"),
        Data::ScopedKeyView("movement.clip.corner.NE"),
        Data::ScopedKeyView("movement.clip.corner.SE"),
        Data::ScopedKeyView("movement.clip.corner.SW")
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
        // Clipping
        clip_north,
        clip_east,
        clip_south,
        clip_west,
        clip_corner_NW,
        clip_corner_NE,
        clip_corner_SE,
        clip_corner_SW
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
