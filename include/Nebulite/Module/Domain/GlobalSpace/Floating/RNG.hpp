#ifndef NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FLOATING_RNG_HPP
#define NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FLOATING_RNG_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <ranges>
#include <string>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/Document/KeyGroup.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Utility/RNG.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::RNG
 * @brief DomainModule for RNG capabilities within the GlobalSpace.
 */
class RNG final : public Base::DomainModule<Core::GlobalSpace> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // None for now

    //------------------------------------------
    // Other public functions

    /**
     * @brief Rolls back all RNGs to their previous state.
     *        Can be called by any domainModule function
     *        if you don't want this functioncall to modify RNG state.
     *        Example: calling a script should not modify RNG, so that we can
     *                 always load scripts for TAS without RNG state changes.
     */
    void rngRollback() {
        for (auto& rng : rngMap | std::views::values) {
            rng.rollback();
        }
    }

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    explicit RNG(ConstructorParams const& params) : DomainModule(params) {
        initRNGs();
        updateRNGs();
    }

    //------------------------------------------
    // Scoped Keys

    struct Key : Data::KeyGroup<"random."> {
        static auto constexpr min = makeScoped("min");
        static auto constexpr max = makeScoped("max");
    };

private:
    using rngSize_t = std::uint16_t; // Modify this to change the size of the RNGs

    absl::flat_hash_map<std::string, Utility::RNG<rngSize_t>> rngMap; // Future-proofing for more RNGs

    /**
     * @brief Initializes standard RNGs
     */
    void initRNGs();

    /**
     * @brief Updates all RNGs
     */
    void updateRNGs();
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_MODULE_DOMAIN_GLOBALSPACE_FLOATING_RNG_HPP
