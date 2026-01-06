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
            static auto constexpr G = Data::ScopedKeyView::create<scope>("G");
        };

        // Per-object physics properties
        struct Local {
            static auto constexpr m = Data::ScopedKeyView::create<scope>("mass"); // TODO: rename to physics.m here and in all json files
            static auto constexpr aX = Data::ScopedKeyView::create<scope>("aX"); // acceleration X
            static auto constexpr aY = Data::ScopedKeyView::create<scope>("aY"); // acceleration Y
            static auto constexpr vX = Data::ScopedKeyView::create<scope>("vX"); // velocity X
            static auto constexpr vY = Data::ScopedKeyView::create<scope>("vY"); // velocity Y
            static auto constexpr FX = Data::ScopedKeyView::create<scope>("FX"); // force X
            static auto constexpr FY = Data::ScopedKeyView::create<scope>("FY"); // force Y

            // More specialized, but still useful keys
            static auto constexpr lastCollisionTimeX = Data::ScopedKeyView::create<scope>("collision.time.lastX");
            static auto constexpr lastCollisionTimeY = Data::ScopedKeyView::create<scope>("collision.time.lastY");
        };
    };
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_PHYSICS_HPP
