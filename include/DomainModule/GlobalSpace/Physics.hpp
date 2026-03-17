/**
 * @file Physics.hpp
 * @brief This file defines the Physics DomainModule in the GlobalSpace,
 *        containing keys for global physics constants and settings.
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBALSPACE_PHYSICS_HPP
#define NEBULITE_DOMAINMODULE_GLOBALSPACE_PHYSICS_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace;
} // namespace Core

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Physics
 * @brief The Physics DomainModule in the GlobalSpace,
 *        containing keys for global physics constants and settings.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Physics) {
public:
    [[nodiscard]] Constants::Error update() override;
    void reinit() override {}

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Physics) {
        // TODO: Set Global physics constants
    }

    struct Key : Data::KeyGroup<"physics."> {
        // Global physics constants
        struct Global {
            static auto constexpr G = makeScoped("G");
        };

        // Per-object physics properties
        struct Local {
            static auto constexpr m = makeScoped("mass"); // TODO: rename to physics.m here and in all json files
            static auto constexpr aX = makeScoped("aX"); // acceleration X
            static auto constexpr aY = makeScoped("aY"); // acceleration Y
            static auto constexpr vX = makeScoped("vX"); // velocity X
            static auto constexpr vY = makeScoped("vY"); // velocity Y
            static auto constexpr FX = makeScoped("FX"); // force X
            static auto constexpr FY = makeScoped("FY"); // force Y

            // Correction values
            struct Correction : KeyGroup<"physics.correction."> {
                static auto constexpr X = makeScoped("X"); // position correction X to resolve overlaps
                static auto constexpr Y = makeScoped("Y"); // position correction Y to resolve overlaps
                static auto constexpr vX = makeScoped("vX"); // velocity correction X to resolve collisions
                static auto constexpr vY = makeScoped("vY"); // velocity correction Y to resolve collisions
            };

            // More specialized, but still useful keys
            static auto constexpr lastCollisionTimeX = makeScoped("collision.time.lastX");
            static auto constexpr lastCollisionTimeY = makeScoped("collision.time.lastY");
        };
    };
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_PHYSICS_HPP
