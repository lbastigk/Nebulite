#ifndef NEBULITE_MODULE_RULESET_MOVEMENT_HPP
#define NEBULITE_MODULE_RULESET_MOVEMENT_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <string_view>
#include <vector>

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Data/Document/ScopedKey.hpp"
#include "Nebulite/Module/Base/RulesetModule.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Physics.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Ruleset {
/**
 * @brief The Movement ruleset module, containing static rulesets related to movement and collision clipping
 */
class Movement : public Base::RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Global rulesets

    void detectClipping(Interaction::Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr detectClippingName = "::movement::detectClipping";
    static std::string_view constexpr detectClippingDesc = "Global ruleset to detect the closest object in each direction. The listeners distance is set.";

    // Local rulesets

    void processClipping(Interaction::Context const& context, double** slf, double** otr) const ;
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
        Domain::GlobalSpace::Physics::Key::Local::vX,
        Domain::GlobalSpace::Physics::Key::Local::vY,
        Domain::GlobalSpace::Physics::Key::Local::m,
        Domain::GlobalSpace::Physics::Key::Local::FX,
        Domain::GlobalSpace::Physics::Key::Local::FY,
        // Closest X/Y
        Data::ScopedKeyView("movement.clip.closest.N"),
        Data::ScopedKeyView("movement.clip.closest.E"),
        Data::ScopedKeyView("movement.clip.closest.S"),
        Data::ScopedKeyView("movement.clip.closest.W"),
        // X/Y last pos
        Domain::GlobalSpace::Physics::Key::Local::lastPositionX,
        Domain::GlobalSpace::Physics::Key::Local::lastPositionY
    };

    /**
     * @enum Key
     * @brief Enumeration of keys corresponding to movement-related base values.
     *        Used for indexing into the ordered cache list.
     */
    enum class Key : std::uint8_t {
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

    struct Radius {
        double& slf;
        double& otr;

        Radius(double** slfBase, double** otrBase)
        : slf(baseVal(slfBase, Key::sizeR)), otr(baseVal(otrBase, Key::sizeR))
        {}
    };
};
} // namespace Nebulite::Module::Ruleset
#endif // NEBULITE_MODULE_RULESET_MOVEMENT_HPP
