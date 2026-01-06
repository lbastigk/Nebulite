/**
 * @file RNG.hpp
 * @brief Contains the declaration of the RNG DomainModule for the GlobalSpace domain.
 */

#ifndef NEBULITE_GSDM_RNG_HPP
#define NEBULITE_GSDM_RNG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/RNG.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::RNG
 * @brief DomainModule for RNG capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, RNG) {
public:
    Constants::Error update() override;
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
        rng.A.rollback();
        rng.B.rollback();
        rng.C.rollback();
        rng.D.rollback();
    }

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, RNG) {
        updateRNGs();
    }

    //------------------------------------------
    // Scoped Keys

    struct Key {
        static auto constexpr scope = "random.";
        static auto constexpr A = Data::ScopedKeyView::create<scope>("A");
        static auto constexpr B = Data::ScopedKeyView::create<scope>("B");
        static auto constexpr C = Data::ScopedKeyView::create<scope>("C");
        static auto constexpr D = Data::ScopedKeyView::create<scope>("D");
        static auto constexpr min = Data::ScopedKeyView::create<scope>("min");
        static auto constexpr max = Data::ScopedKeyView::create<scope>("max");
    };

private:
    /**
     * @struct RngVars
     * @brief Contains RNG instances used in the global space.
     * @todo Consider a hashmap of RNGs for more versatility in the future.
     *       std::string -> Utility::RNG<rngSize_t>
     *       Simplifies the rng rollback and update functions as well.
     */
    struct RngVars {
        using rngSize_t = uint16_t; // Modify this to change the size of the RNGs
        Utility::RNG<rngSize_t> A; // RNG with key random.A
        Utility::RNG<rngSize_t> B; // RNG with key random.B
        Utility::RNG<rngSize_t> C; // RNG with key random.C
        Utility::RNG<rngSize_t> D; // RNG with key random.D
    } rng;

    /**
     * @brief Updates all RNGs
     */
    void updateRNGs();
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_RNG_HPP
