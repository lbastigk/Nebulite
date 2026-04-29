//------------------------------------------
// Includes

// Nebulite
#include "Module/Domain/GlobalSpace/Time.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

//------------------------------------------
// Update

Constants::Event Time::updateHook() {
    //------------------------------------------
    // Full time (runtime)

    // Update
    RealTime.update();
    uint64_t const dt_ms = RealTime.get_dt_ms();
    uint64_t const t_ms = RealTime.get_t_ms();

    // Set in doc
    moduleScope.set<double>(Key::runtime_dt, static_cast<double>(dt_ms) / 1000.0);
    moduleScope.set<double>(Key::runtime_t, static_cast<double>(t_ms) / 1000.0);
    moduleScope.set<Uint64>(Key::runtime_dt_ms, dt_ms);
    moduleScope.set<Uint64>(Key::runtime_t_ms, t_ms);

    // See if simulation time can progress
    if (!haltThisFrame && timeLocks.empty() && !domain.getRenderer().isSkippingUpdate()) {
        //------------------------------------------
        // Simulation time (can be paused)

        // Update
        if (fixedDeltaTime > 0) {
            // Use fixed delta time
            SimulationTime.update(fixedDeltaTime);
        } else {
            // Use real delta time
            SimulationTime.update(dt_ms);
        }
        uint64_t const sim_dt_ms = SimulationTime.get_dt_ms();
        uint64_t const sim_t_ms = SimulationTime.get_t_ms();

        // Set in doc
        moduleScope.set<double>(Key::time_dt, static_cast<double>(sim_dt_ms) / 1000.0);
        moduleScope.set<double>(Key::time_t, static_cast<double>(sim_t_ms) / 1000.0);
        moduleScope.set<Uint64>(Key::time_dt_ms, sim_dt_ms);
        moduleScope.set<Uint64>(Key::time_t_ms, sim_t_ms);
    } else {
        domain.getRenderer().skipUpdateNextFrame();
    }
    haltThisFrame = false;

    // Ignoring results for now, just return NONE
    return Constants::Event::Success;
}

//------------------------------------------
// Available Functions

Constants::Event Time::time_haltOnce() {
    haltThisFrame = true;
    return Constants::Event::Success;
}

Constants::Event Time::time_lock(int const argc, char** argv) {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const lockName = argv[1];
    timeLocks.insert(lockName);
    return Constants::Event::Success;
}

Constants::Event Time::time_unlock(int const argc, char** argv) {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const lockName = argv[1];
    if (auto const it = timeLocks.find(lockName); it != timeLocks.end()) {
        timeLocks.erase(it);
    } else {
        return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
    }
    return Constants::Event::Success;
}

Constants::Event Time::time_masterUnlock() {
    timeLocks.clear();
    return Constants::Event::Success;
}

Constants::Event Time::time_setFixedDeltaTime(int const argc, char** argv) {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    try {
        uint64_t const dt = std::stoull(argv[1]);
        fixedDeltaTime = dt;
    } catch (...) {
        return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
