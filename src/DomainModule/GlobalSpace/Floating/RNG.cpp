#include "DomainModule/GlobalSpace/Floating/RNG.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::GlobalSpace {
Constants::Error RNG::update() {
    // Disabled if renderer skipped update last frame, active otherwise
    bool RNG_update_enabled = domain.getRenderer().isSdlInitialized() && domain.getRenderer().hasSkippedUpdate() == false;
    RNG_update_enabled |= !domain.getRenderer().isSdlInitialized(); // If renderer is not initialized, we always update RNGs
    if (RNG_update_enabled) {
        updateRNGs();
    }

    return Constants::ErrorTable::NONE();
}

void RNG::updateRNGs() {
    // Set Min and Max values for RNGs in document
    // Always set, so overwrites don't stick around
    moduleScope.set<RngVars::rngSize_t>(Key::min, std::numeric_limits<RngVars::rngSize_t>::min());
    moduleScope.set<RngVars::rngSize_t>(Key::max, std::numeric_limits<RngVars::rngSize_t>::max());

    // Generate seeds in a predictable manner
    // Since updateRNG is called at specific times only, we can simply increment RNG with a new seed
    std::string const seedA = "A" + std::to_string(rng.A.get());
    std::string const seedB = "B" + std::to_string(rng.B.get());
    std::string const seedC = "C" + std::to_string(rng.C.get());
    std::string const seedD = "D" + std::to_string(rng.D.get());

    // Hash seeds
    rng.A.update(seedA);
    rng.B.update(seedB);
    rng.C.update(seedC);
    rng.D.update(seedD);

    // Set RNG values in global document
    moduleScope.set<RngVars::rngSize_t>(Key::A, rng.A.get());
    moduleScope.set<RngVars::rngSize_t>(Key::B, rng.B.get());
    moduleScope.set<RngVars::rngSize_t>(Key::C, rng.C.get());
    moduleScope.set<RngVars::rngSize_t>(Key::D, rng.D.get());
}

} // namespace Nebulite::DomainModule::GlobalSpace

