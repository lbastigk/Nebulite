//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <optional>

// Nebulite
#include "Nebulite/Utility/Time.hpp"
#include "Nebulite/Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Utility {

TimeKeeper::TimeKeeper() noexcept : t_start(Time::getTime()) {}

void TimeKeeper::update(std::optional<std::uint64_t> const fixedTimeDelta) {
    onUpdate.lastTimeMilliSeconds = onUpdate.tMilliSeconds;
    onUpdate.tMilliSeconds      = Time::getTime() - t_start;
    if(running){
        // Check if we have a fixed dt
        if(fixedTimeDelta.has_value()){
            deltaTimeMilliSeconds = fixedTimeDelta.value();
        } else {
            deltaTimeMilliSeconds = onUpdate.tMilliSeconds - onUpdate.lastTimeMilliSeconds;
        }
    } else {
        deltaTimeMilliSeconds = 0;
    }
    tMilliSeconds += deltaTimeMilliSeconds;
}

void TimeKeeper::start() noexcept {
    running = true;
}

void TimeKeeper::stop() noexcept {
    running = false;
}

bool TimeKeeper::isRunning() const noexcept {
    return running;
}

std::uint64_t TimeKeeper::dtProjected() noexcept {
    if(running){
        onSimulation.lastTimeMilliSeconds = onUpdate.tMilliSeconds;
        onSimulation.tMilliSeconds = Time::getTime() - t_start;
        onSimulation.dt = onSimulation.tMilliSeconds - onSimulation.lastTimeMilliSeconds;
        return onSimulation.dt;
    }
    return 0;
}

std::uint64_t TimeKeeper::timeMilliseconds() const noexcept {
    return tMilliSeconds;
}

std::uint64_t TimeKeeper::timeDeltaMilliseconds() const noexcept {
    return deltaTimeMilliSeconds;
}

} // namespace Nebulite::Utility
