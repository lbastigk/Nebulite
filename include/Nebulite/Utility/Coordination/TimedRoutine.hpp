#ifndef NEBULITE_UTILITY_COORDINATION_TIMEDROUTINE_HPP
#define NEBULITE_UTILITY_COORDINATION_TIMEDROUTINE_HPP

//------------------------------------------
// Includes

// Standard Library
#include <cstdint>
#include <functional>

// Nebulite
#include "Nebulite/Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Utility::Coordination {
/**
 * @class TimedRoutine
 * @brief A class that schedules a routine to be executed at specified time intervals.
 */
class TimedRoutine {
public:
    enum class ConstructionMode : std::uint8_t {
        START_IMMEDIATELY = 0,
        WAIT_FOR_START,
        START_IMMEDIATELY_AND_EXECUTE_ONCE
    };

    TimedRoutine(std::function<void()> const& routine, std::uint64_t intervalMillis, ConstructionMode mode = ConstructionMode::WAIT_FOR_START);

    /**
     * @brief Start the timer for the scheduled routine.
     *        This will allow the routine to be executed at the specified intervals when update() is called.
     */
    void start();

    /**
     * @brief Check if the scheduled routine should be executed based
     *        on the timer and interval, and execute it if necessary.
     */
    void update();

    /**
     * @brief Force execute the scheduled routine immediately, without checking the timer or updating the timer.
     */
    void forceExecute() const ;

private:
    std::function<void()> foo;
    TimeKeeper timer;
    std::uint64_t interval;
};
} // namespace Nebulite::Utility::Coordination
#endif // NEBULITE_UTILITY_COORDINATION_TIMEDROUTINE_HPP
