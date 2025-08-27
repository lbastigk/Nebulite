/**
 * @file TimeKeeper.h
 * @brief Contains the Nebulite::Utility::TimeKeeper class for managing time-related operations.
 */

#include "Utility/Time.h"

namespace Nebulite {
namespace Utility {
/**
 * @class Nebulite::Utility::TimeKeeper
 * @brief Manages time-related operations in the Nebulite engine.
 * 
 * The TimeKeeper class provides functionality to track elapsed time,
 * manage frame rates, and handle delays.
 * 
 * The start time is set to construction time. Timer is initialized to not running.
 */
class TimeKeeper{
public:

    /**
     * @brief Constructs a new TimeKeeper object and initializes the timer.
     * 
     * The start time is set to construction time. Timer is initialized to not running.
     */
    TimeKeeper(){
        t_ms = Time::gettime() - t_start;
        dt_ms = 0;
        onUpdate.t_ms = t_ms;
        onUpdate.last_t_ms = t_ms;
    }

    /**
     * @brief Updates the timer.
     * 
     * This function calculates the delta time since the last update and updates the timer.
     * 
     * @param fixed_dt_ms If greater than 0, this value will be used as the delta time instead of the calculated value.
     * 
     * @todo The implemented logic is has too many sets, then resets and all that nonsense. Create a proper branching path
     */
    void update(uint64_t fixed_dt_ms = 0){
        // 1.) Gather timing information: last t, current t, dt
        onUpdate.last_t_ms = onUpdate.t_ms;
        onUpdate.t_ms = Time::gettime() - t_start;
        onUpdate.dt = onUpdate.t_ms - onUpdate.last_t_ms;

        // 2.) If a fixed dt was specified, we use that value
        if(fixed_dt_ms > 0){
            onUpdate.dt = fixed_dt_ms;
        }

        // 3.) Set dt
        if(running){
            dt_ms = onUpdate.dt;
        }
        else{
            dt_ms = 0;
        }

        // 4.) Integrate dt
        t_ms += dt_ms;
    }

    /**
     * @brief Starts the timer.
     * 
     * This function initializes the timer and begins tracking elapsed time.
     */
    void start(){
        running = true;
    }

    /**
     * @brief Stops the timer.
     * 
     * This function stops the timer and pauses tracking elapsed time.
     * Any accumulated time will be preserved.
     * Note that in stop, get_dt_ms() will return the last update's delta time.
     */
    void stop(){
        running = false;
    }

    /**
     * @brief Calculates the projected dt if update() was to be called.
     * 
     * This function estimates the delta time that would be reported if update() were called.
     * It does this by simulating the passage of time through a direct system_clock call for elapsed time.
     * If the timer is not running, the projected delta time will be zero.
     * 
     * @return The projected delta time in milliseconds.
     */
    uint64_t projected_dt(){
        if(running){
            onSimulation.last_t_ms = onUpdate.t_ms;
            onSimulation.t_ms = Time::gettime() - t_start;
            onSimulation.dt = onSimulation.t_ms - onSimulation.last_t_ms;
            return onSimulation.dt;
        }
        else{
            return 0;
        }
    }

    /**
     * @brief Gets the current time in milliseconds since the timer started.
     * 
     * This function returns the time elapsed since the timer started.
     * Note that the returned value is not necessarily equal to system time, as the update function allows for a custom dt.
     * 
     * @return The time elapsed since the timer started in milliseconds.
     */
    uint64_t get_t_ms() const {
        return t_ms;
    }

    /**
     * @brief Gets the delta time in milliseconds since the last update.
     * 
     * This function returns the time difference between the last two updates.
     * If the timer is not running, it will return the last update's delta time.
     * Note that the returned value is not necessarily equal to system time, as the update function allows for a custom dt.
     * 
     * @return The time difference between the last two updates in milliseconds.
     */
    uint64_t get_dt_ms() const {
        return dt_ms;
    }

private:
    // Basic values for current time

    /**
     * @brief The current time in milliseconds of the last update.
     */
    uint64_t t_ms;

    /**
     * @brief The delta time in milliseconds between the last two updates.
     */
    uint64_t dt_ms;

    /**
     * @brief The start time in milliseconds when the timer was created.
     * 
     * For Reference: This value is used to calculate the total elapsed time since the timer was created.
     */
    const uint64_t t_start = Time::gettime();

    /**
     * @brief Indicates whether the timer is currently running.
     * 
     * On construction, the timer is off.
     */
    bool running = false;

    /**
     * @struct OnUpdate
     * @brief Stores the timing information for the update phase.
     */
    struct OnUpdate{
        uint64_t last_t_ms;
        uint64_t t_ms;
        uint64_t dt;
    } onUpdate;

    /**
     * @struct OnSimulation
     * @brief Stores the timing information for the update simulation phase.
     */
    struct OnSimulation{
        uint64_t last_t_ms;
        uint64_t t_ms;
        uint64_t dt;
    } onSimulation;
};
}   // namespace Utility
}   // namespace Nebulite