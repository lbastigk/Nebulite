//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <string>

// External
#include <SDL3/SDL_stdinc.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Time.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

//------------------------------------------
// Update

Constants::Event Time::updateHook() {
    //------------------------------------------
    // Update runtime

    // Update
    realTime.update();
    std::uint64_t const deltaTimeMilliSeconds = realTime.timeDeltaMilliseconds();
    std::uint64_t const tMilliSeconds = realTime.timeMilliseconds();

    // Set in doc
    moduleScope.set<double>(Key::runTimeDeltaTime, static_cast<double>(deltaTimeMilliSeconds) / 1000.0);
    moduleScope.set<double>(Key::runTime, static_cast<double>(tMilliSeconds) / 1000.0);
    moduleScope.set<Uint64>(Key::runTimeDeltaTimeMilliSeconds, deltaTimeMilliSeconds);
    moduleScope.set<Uint64>(Key::runtimeMilliSeconds, tMilliSeconds);

    //------------------------------------------
    // Update simulation time
    if (!haltThisFrame && timeLocks.empty() && !domain.getRenderer().isSkippingUpdate()) {
        //------------------------------------------
        // Simulation time (can be paused)

        // Update
        if (fixedDeltaTime > 0) {
            // Use fixed delta time
            simulationTime.update(fixedDeltaTime);
        } else {
            // Use real delta time
            // Perhaps adding a max delta time cap in the future to prevent huge jumps?
            simulationTime.update(deltaTimeMilliSeconds);
        }
        auto const simDeltaTimeMilliSeconds = simulationTime.timeDeltaMilliseconds();
        auto const simTimeMilliSeconds = simulationTime.timeMilliseconds();

        // Set in doc
        moduleScope.set<double>(Key::deltaTime, static_cast<double>(simDeltaTimeMilliSeconds) / 1000.0);
        moduleScope.set<double>(Key::time, static_cast<double>(simTimeMilliSeconds) / 1000.0);
        moduleScope.set<Uint64>(Key::timeDeltaTimeMilliSeconds, simDeltaTimeMilliSeconds);
        moduleScope.set<Uint64>(Key::timeMilliSeconds, simTimeMilliSeconds);
        moduleScope.set<bool>(Key::timeLocked, false);
    } else {
        moduleScope.set<bool>(Key::timeLocked, true);
        domain.getRenderer().skipUpdateNextFrame();
    }
    haltThisFrame = false;

    return Constants::Event::success;
}

//------------------------------------------
// Available Functions

Constants::Event Time::timeHaltOnce() {
    haltThisFrame = true;
    return Constants::Event::success;
}

Constants::Event Time::timeLock(int const argc, char const** argv) {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const lockName = argv[1];
    timeLocks.insert(lockName);
    return Constants::Event::success;
}

Constants::Event Time::timeUnlock(int const argc, char const** argv) {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const lockName = argv[1];
    if (auto const it = timeLocks.find(lockName); it != timeLocks.end()) {
        timeLocks.erase(it);
    } else {
        return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
    }
    return Constants::Event::success;
}

Constants::Event Time::timeMasterUnlock() {
    timeLocks.clear();
    return Constants::Event::success;
}

Constants::Event Time::timeSetFixedDeltaTime(int const argc, char const** argv) {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    try {
        std::uint64_t const dt = std::stoull(argv[1]);
        fixedDeltaTime = dt;
    } catch (...) {
        return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
    }
    return Constants::Event::success;
}

} // namespace Nebulite::Module::Domain::GlobalSpace
