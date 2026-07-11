//------------------------------------------
// Includes

// Standard library
#include <limits>
#include <string>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Floating/RNG.hpp"
#include "Nebulite/Utility/RNG.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
Constants::Event RNG::updateHook() {
    // Disabled if renderer skipped update last frame, active otherwise
    bool RNG_update_enabled = domain.getRenderer().isSdlInitialized() && !domain.getRenderer().hasSkippedUpdate();
    RNG_update_enabled |= !domain.getRenderer().isSdlInitialized(); // If renderer is not initialized, we always update RNGs
    if (RNG_update_enabled) {
        updateRNGs();
    }

    return Constants::Event::Success;
}

void RNG::initRNGs(){
    rngMap.emplace("A", Utility::RNG<rngSize_t>());
    rngMap.emplace("B", Utility::RNG<rngSize_t>());
    rngMap.emplace("C", Utility::RNG<rngSize_t>());
    rngMap.emplace("D", Utility::RNG<rngSize_t>());
}

void RNG::updateRNGs() {
    // Set Min and Max values for RNGs in document
    // Always set, so overwrites don't stick around
    moduleScope.set<rngSize_t>(Key::min, std::numeric_limits<rngSize_t>::min());
    moduleScope.set<rngSize_t>(Key::max, std::numeric_limits<rngSize_t>::max());

    for (auto& [key, rng] : rngMap) {
        auto const seed = key + std::to_string(rng.get());
        rng.update(seed);
        auto scopedKey = Key::root().addMember(key);
        moduleScope.set<rngSize_t>(scopedKey, rng.get());
    }
}

} // namespace Nebulite::Module::Domain::GlobalSpace

