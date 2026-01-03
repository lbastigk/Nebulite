#include "DomainModule/GlobalSpace/Time.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

//------------------------------------------
// Update

Constants::Error Time::update() {
    //------------------------------------------
    // Full time (runtime)

    // Update
    RealTime.update();
    uint64_t const dt_ms = RealTime.get_dt_ms();
    uint64_t const t_ms = RealTime.get_t_ms();

    // Set in doc
    getDoc().set<double>(Key::runtime_dt, static_cast<double>(dt_ms) / 1000.0);
    getDoc().set<double>(Key::runtime_t, static_cast<double>(t_ms) / 1000.0);
    getDoc().set<Uint64>(Key::runtime_dt_ms, dt_ms);
    getDoc().set<Uint64>(Key::runtime_t_ms, t_ms);

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
        getDoc().set<double>(Key::time_dt, static_cast<double>(sim_dt_ms) / 1000.0);
        getDoc().set<double>(Key::time_t, static_cast<double>(sim_t_ms) / 1000.0);
        getDoc().set<Uint64>(Key::time_dt_ms, sim_dt_ms);
        getDoc().set<Uint64>(Key::time_t_ms, sim_t_ms);

        //------------------------------------------
        // Increase Frame count
        /**
         * @todo Consider a secondary frame count that only increases when simulation time progresses
         *       Perhaps even use that as standard frame count, and have a separate "real frame count" for total frames rendered
         *       Best idea would be to check if the renderer is skipping the update, and only increase frame count if not
         */
        getDoc().set<uint64_t>(Key::frameCount, frameCount); // Starts at 0
        frameCount++;
    } else {
        domain.getRenderer().skipUpdateNextFrame();
    }
    haltThisFrame = false;

    // Ignoring results for now, just return NONE
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Constants::Error Time::time_haltOnce() {
    haltThisFrame = true;
    return Constants::ErrorTable::NONE();
}

Constants::Error Time::time_lock(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const lockName = argv[1];
    timeLocks.insert(lockName);
    return Constants::ErrorTable::NONE();
}

Constants::Error Time::time_unlock(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string const lockName = argv[1];
    if (auto const it = timeLocks.find(lockName); it != timeLocks.end()) {
        timeLocks.erase(it);
    } else {
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error Time::time_masterUnlock() {
    timeLocks.clear();
    return Constants::ErrorTable::NONE();
}

Constants::Error Time::time_setFixedDeltaTime(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    try {
        uint64_t const dt = std::stoull(argv[1]);
        fixedDeltaTime = dt;
    } catch (...) {
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::GlobalSpace
