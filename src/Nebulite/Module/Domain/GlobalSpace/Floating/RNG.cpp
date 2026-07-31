//------------------------------------------
// Includes

// Standard library
#include <limits>
#include <ranges>
#include <string>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Floating/RNG.hpp"
#include "Nebulite/Utility/RNG.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
Constants::Event RNG::updateHook() {
    // Disabled if renderer skipped update last frame, active otherwise
    bool rngUpdateEnabled = domain.getRenderer().isSdlInitialized() && !domain.getRenderer().hasSkippedUpdate();
    rngUpdateEnabled |= !domain.getRenderer().isSdlInitialized(); // If renderer is not initialized, we always update RNGs
    if (rngUpdateEnabled) {
        updateRng();
    }

    return Constants::Event::success;
}

void RNG::rngRollback() {
    for (auto& rng : rngMap | std::views::values) {
        rng.rollback();
    }
}

RNG::RNG(ConstructorParams const& params) : DomainModule(params) {
    initRng();
    updateRng();
}

//------------------------------------------
// Private functions

void RNG::initRng(){
    rngMap.emplace("A", Utility::RNG<rngSize>());
    rngMap.emplace("B", Utility::RNG<rngSize>());
    rngMap.emplace("C", Utility::RNG<rngSize>());
    rngMap.emplace("D", Utility::RNG<rngSize>());
}

void RNG::updateRng() {
    // Set Min and Max values for RNGs in document
    // Always set, so overwrites don't stick around
    moduleScope.set<rngSize>(Key::min, std::numeric_limits<rngSize>::min());
    moduleScope.set<rngSize>(Key::max, std::numeric_limits<rngSize>::max());

    for (auto& [key, rng] : rngMap) {
        auto const seed = key + std::to_string(rng.get());
        rng.update(seed);
        auto scopedKey = Key::root().addMember(key);
        moduleScope.set<rngSize>(scopedKey, rng.get());
    }
}

} // namespace Nebulite::Module::Domain::GlobalSpace

