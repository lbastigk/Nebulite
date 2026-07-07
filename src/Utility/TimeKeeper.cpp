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

void TimeKeeper::update(std::optional<std::uint64_t> const fixed_dt_ms) {
    onUpdate.last_t_ms = onUpdate.t_ms;
    onUpdate.t_ms      = Time::getTime() - t_start;
    if(running){
        // Check if we have a fixed dt
        if(fixed_dt_ms.has_value()){
            dt_ms = fixed_dt_ms.value();
        } else {
            dt_ms = onUpdate.t_ms - onUpdate.last_t_ms;
        }
    } else {
        dt_ms = 0;
    }
    t_ms += dt_ms;
}

void TimeKeeper::start() noexcept {
    running = true;
}

void TimeKeeper::stop() noexcept {
    running = false;
}

bool TimeKeeper::is_running() const noexcept {
    return running;
}

std::uint64_t TimeKeeper::projected_dt() noexcept {
    if(running){
        onSimulation.last_t_ms = onUpdate.t_ms;
        onSimulation.t_ms = Time::getTime() - t_start;
        onSimulation.dt = onSimulation.t_ms - onSimulation.last_t_ms;
        return onSimulation.dt;
    }
    return 0;
}

std::uint64_t TimeKeeper::get_t_ms() const noexcept {
    return t_ms;
}

std::uint64_t TimeKeeper::get_dt_ms() const noexcept {
    return dt_ms;
}

} // namespace Nebulite::Utility
