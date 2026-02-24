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
    Constants::Error update() override;
    void reinit() override {}

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Physics) {
        // TODO: Set Global physics constants
    }

    struct Key {
        static auto constexpr scope = "physics.";

        // Global physics constants
        struct Global {
            static auto constexpr G = MAKE_SCOPED("G");
        };

        // Per-object physics properties
        struct Local {
            static auto constexpr m = MAKE_SCOPED("mass"); // TODO: rename to physics.m here and in all json files
            static auto constexpr aX = MAKE_SCOPED("aX"); // acceleration X
            static auto constexpr aY = MAKE_SCOPED("aY"); // acceleration Y
            static auto constexpr vX = MAKE_SCOPED("vX"); // velocity X
            static auto constexpr vY = MAKE_SCOPED("vY"); // velocity Y
            static auto constexpr FX = MAKE_SCOPED("FX"); // force X
            static auto constexpr FY = MAKE_SCOPED("FY"); // force Y

            // Correction values
            struct Correction {
                DECLARE_SCOPE("physics.correction.")
                static auto constexpr X = MAKE_SCOPED("X"); // position correction X to resolve overlaps
                static auto constexpr Y = MAKE_SCOPED("Y"); // position correction Y to resolve overlaps
                static auto constexpr vX = MAKE_SCOPED("vX"); // velocity correction X to resolve collisions
                static auto constexpr vY = MAKE_SCOPED("vY"); // velocity correction Y to resolve collisions
            };

            // More specialized, but still useful keys
            static auto constexpr lastCollisionTimeX = MAKE_SCOPED("collision.time.lastX");
            static auto constexpr lastCollisionTimeY = MAKE_SCOPED("collision.time.lastY");
        };
    };
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_PHYSICS_HPP
