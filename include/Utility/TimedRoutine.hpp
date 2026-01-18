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
    TimedRoutine(std::function<void()> const& routine, uint64_t const& intervalMillis)
        : routine(routine), interval(intervalMillis) {}

    void start() {
        timer.start();
    }

    void update() {
        if (timer.projected_dt() >= interval) {
            timer.update(); // Update timer to reset dt
            routine(); // Execute the scheduled routine
        }
    }

private:
    std::function<void()> routine;
    TimeKeeper timer;
    uint64_t interval;
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_TIMED_ROUTINE_HPP
