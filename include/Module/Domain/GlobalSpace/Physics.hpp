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
#include "ScopeAccessor.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace;
} // namespace Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::Physics
 * @brief The Physics DomainModule in the GlobalSpace,
 *        containing keys for global physics constants and settings.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Physics) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Physics) {
        setupConstants();
    }

    struct Key : Data::KeyGroup<"physics."> {
        // Global physics constants
        struct Global {
            // Fundamental constants
            static constexpr auto G        = makeScoped("G");        // Gravitational constant (m^3 kg^-1 s^-2)
            static constexpr auto c        = makeScoped("c");        // Speed of light in vacuum (m/s)
            static constexpr auto h        = makeScoped("h");        // Planck constant (J·s)
            static constexpr auto hbar     = makeScoped("hbar");     // Reduced Planck constant (J·s)
            static constexpr auto kB       = makeScoped("kB");       // Boltzmann constant (J/K)

            // Electromagnetic constants
            static constexpr auto e        = makeScoped("e");        // Elementary charge (C)
            static constexpr auto epsilon0 = makeScoped("epsilon0"); // Vacuum permittivity (F/m)
            static constexpr auto mu0      = makeScoped("mu0");      // Vacuum permeability (N/A^2)
            static constexpr auto ke       = makeScoped("ke");       // Coulomb constant (N·m^2/C^2)

            // Particle masses
            static constexpr auto me       = makeScoped("me");       // Electron mass (kg)
            static constexpr auto mp       = makeScoped("mp");       // Proton mass (kg)
            static constexpr auto mn       = makeScoped("mn");       // Neutron mass (kg)

            // Thermodynamics / chemistry
            static constexpr auto NA       = makeScoped("NA");       // Avogadro constant (1/mol)
            static constexpr auto R        = makeScoped("R");        // Gas constant (J/mol·K)

            // Earth-related
            static constexpr auto g        = makeScoped("g");        // Standard gravity (m/s^2)
        };

        // Per-object physics properties
        struct Local {
            static auto constexpr m = makeScoped("mass"); // TODO: rename to physics.m here and in all json and nebs files
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

            // Last X/Y Pos
            static auto constexpr lastPositionX = makeScoped("last.position.X");
            static auto constexpr lastPositionY = makeScoped("last.position.Y");
        };
    };

private:
    /**
     * @brief Sets physics constants in the GlobalSpace
     */
    void setupConstants() const ;
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_PHYSICS_HPP
