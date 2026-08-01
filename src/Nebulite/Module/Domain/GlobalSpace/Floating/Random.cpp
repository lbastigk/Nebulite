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
#include "Nebulite/Module/Domain/GlobalSpace/Floating/Random.hpp"
#include "Nebulite/Utility/Random.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
Constants::Event Random::updateHook() {
    // Disabled if renderer skipped update last frame, active otherwise
    bool rngUpdateEnabled = domain.getRenderer().isSdlInitialized() && !domain.getRenderer().hasSkippedUpdate();
    rngUpdateEnabled |= !domain.getRenderer().isSdlInitialized(); // If renderer is not initialized, we always update RNGs
    if (rngUpdateEnabled) {
        updateRng();
    }

    return Constants::Event::success;
}

void Random::rngRollback() {
    for (auto& rng : rngMap | std::views::values) {
        rng.rollback();
    }
}

Random::Random(ConstructorParams const& params) : DomainModule(params) {
    initRng();
    updateRng();
}

//------------------------------------------
// Private functions

void Random::initRng(){
    rngMap.emplace("A", Utility::Random<RngSize>());
    rngMap.emplace("B", Utility::Random<RngSize>());
    rngMap.emplace("C", Utility::Random<RngSize>());
    rngMap.emplace("D", Utility::Random<RngSize>());
}

void Random::updateRng() {
    // Set Min and Max values for RNGs in document
    // Always set, so overwrites don't stick around
    moduleScope.set<RngSize>(Key::min, std::numeric_limits<RngSize>::min());
    moduleScope.set<RngSize>(Key::max, std::numeric_limits<RngSize>::max());

    for (auto& [key, rng] : rngMap) {
        auto const seed = key + std::to_string(rng.get());
        rng.update(seed);
        auto scopedKey = Key::root().addMember(key);
        moduleScope.set<RngSize>(scopedKey, rng.get());
    }
}

} // namespace Nebulite::Module::Domain::GlobalSpace

