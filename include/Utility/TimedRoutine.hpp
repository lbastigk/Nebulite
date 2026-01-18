/**
 * @file TimedRoutine.hpp
 * @brief Definition of TimedRoutine class for scheduling routines at specific intervals.
 */

#ifndef NEBULITE_UTILITY_TIMED_ROUTINE_HPP
#define NEBULITE_UTILITY_TIMED_ROUTINE_HPP

//------------------------------------------
// Includes

// Standard Library
#include <functional>

// Nebulite
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class TimedRoutine
 * @brief A class that schedules a routine to be executed at specified time intervals.
 * @todo Replace existing codebase implementation with manual TimeKeeper usage with this class.
 */
class TimedRoutine {
public:
    enum class ConstructionMode {
        START_IMMEDIATELY,
        WAIT_FOR_START
    };

    TimedRoutine(std::function<void()> const& routine, uint64_t const& intervalMillis, ConstructionMode const& mode = ConstructionMode::WAIT_FOR_START)
        : foo(routine), interval(intervalMillis) {
        if (mode == ConstructionMode::START_IMMEDIATELY) {
            timer.start();
        }
    }

    void start() {
        timer.start();
    }

    void update() {
        if (timer.projected_dt() >= interval) {
            timer.update(); // Update timer to reset dt
            foo(); // Execute the scheduled routine
        }
    }

private:
    std::function<void()> foo;
    TimeKeeper timer;
    uint64_t interval;
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_TIMED_ROUTINE_HPP
