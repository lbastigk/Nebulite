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
// Forward declarations
namespace Nebulite::Interaction::Rules {
struct Context;
} // namespace Interaction::Rules

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

    void clip(Interaction::Context const& context) const ;
    static constexpr std::string_view clipName = "::movement::clip";
    static constexpr std::string_view clipDesc = "Global ruleset to handle collision clipping between entities. The entry listening to this ruleset will be placed accordingly.";

    //------------------------------------------
    // Constructor
    Movement();

private:
    static constexpr std::string_view moduleName = "::movement";

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
        DomainModule::GlobalSpace::Physics::Key::Local::vY
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
        physics_vY
    };

    // 2.) To retrieve from globalspace
    /**
     * @struct GlobalVal
     * @brief Struct to hold pointers to global variables used in movement calculations.
     */
    struct GlobalVal {
        /* Add more global variables here as needed */
    } globalVal = {};
};
} // namespace Nebulite::RulesetModule
#endif // NEBULITE_RULESET_MODULE_MOVEMENT_HPP
