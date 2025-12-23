/**
 * @file TimeKeeper.hpp
 * @brief Contains the Nebulite::Utility::TimeKeeper class for managing time-related operations.
 */

#ifndef NEBULITE_UTILITY_TIMEKEEPER_HPP
#define NEBULITE_UTILITY_TIMEKEEPER_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Utility/Time.hpp"

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::TimeKeeper
 * @brief Manages time-related operations in the Nebulite engine.
 * 
 * The TimeKeeper class provides functionality to track elapsed time,
 * to manage frame rates, handle delays, etc.
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
    TimeKeeper() noexcept : t_start(Time::getTime()) {}

    /**
     * @brief Updates the timer.
     * 
     * This function calculates the delta time since the last update and updates the timers full runtime.
     * 
     * @param fixed_dt_ms If greater than 0, this value will be used as the delta time instead of the calculated value.
     */
    void update(uint64_t fixed_dt_ms = 0) noexcept {
        //------------------------------------------
        // 1.) Gathering timing information, even if the timer is not running
        //     The whole timer works on dt integration, so we always need to know the current dt
        onUpdate.last_t_ms = onUpdate.t_ms;
        onUpdate.t_ms      = Time::getTime() - t_start;
        
        //------------------------------------------
        // 2.) Derive dt from status
        if(running){
            // Check if we have a fixed dt
            if(fixed_dt_ms > 0){
                dt_ms = fixed_dt_ms;
            }
            else{
                dt_ms = onUpdate.t_ms - onUpdate.last_t_ms;
            }
        }
        else{
            // Not running, dt is 0
            dt_ms = 0;
        }

        //------------------------------------------
        // 3.) Integrate dt
        t_ms += dt_ms;
    }

    /**
     * @brief Starts the timer, updating the running state.
     * 
     * This function initializes the timer and begins tracking elapsed time.
     * 
     * Make sure to call update() before start() to get an accurate dt,
     * if you don't start the timer immediately after construction.
     */
    void start() noexcept {
        running = true;
    }

    /**
     * @brief Stops the timer.
     * 
     * This function stops the timer and pauses tracking elapsed time.
     * Any accumulated time will be preserved.
     * Note that in stop, `get_dt_ms()` will return the last update's delta time.
     * 
     * Make sure to call `update()` before `stop()` to get an accurate dt.
     */
    void stop() noexcept {
        running = false;
    }

    /**
     * @brief Checks if the timer is currently running.
     * 
     * @return True if the timer is running, false otherwise.
     */
    [[nodiscard]] bool is_running() const noexcept {
        return running;
    }

    /**
     * @brief Calculates the projected dt if `update()` were to be called.
     * 
     * It does this by simulating the passage of time through a direct `system_clock` call for elapsed time.
     * If the timer is not running, the projected delta time will be zero.
     * 
     * @return The projected delta time in milliseconds.
     */
    uint64_t projected_dt() noexcept {
        if(running){
            onSimulation.last_t_ms = onUpdate.t_ms;
            onSimulation.t_ms = Time::getTime() - t_start;
            onSimulation.dt = onSimulation.t_ms - onSimulation.last_t_ms;
            return onSimulation.dt;
        }
        return 0;
    }

    /**
     * @brief Gets the current time in milliseconds since the timer started.
     * 
     * This function returns the time elapsed since the timer started.
     * Note that the returned value is not necessarily equal to elapsed system time, 
     * as the update function allows for a custom dt.
     * 
     * @return The time elapsed since the timer started in milliseconds.
     */
    [[nodiscard]] uint64_t get_t_ms() const noexcept {
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
    [[nodiscard]] uint64_t get_dt_ms() const noexcept {
        return dt_ms;
    }

private:
    // Basic values for current time

    /**
     * @brief The start time in milliseconds when the timer was created.
     * 
     * For Reference: This value is used to calculate the total elapsed time since the timer was created.
     */
    uint64_t t_start;

    /**
     * @brief The current time in milliseconds of the last update.
     */
    uint64_t t_ms = 0;

    /**
     * @brief The delta time in milliseconds between the last two updates.
     */
    uint64_t dt_ms = 0;

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
    struct alignas(2 * sizeof(uint64_t)) OnUpdate{
        uint64_t last_t_ms = 0;
        uint64_t t_ms = 0;
        OnUpdate() = default;
    } onUpdate;

    /**
     * @brief Ensures proper alignment for OnUpdate struct.
     */
    static_assert(alignof(OnUpdate) >= 2 * sizeof(uint64_t), "OnUpdate alignment");

    /**
     * @struct OnSimulation
     * @brief Stores the timing information for the update simulation phase.
     */
    struct OnSimulation{
        uint64_t last_t_ms = 0;
        uint64_t t_ms = 0;
        uint64_t dt = 0;
        OnSimulation() = default;
    } onSimulation;
};
}   // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_TIMEKEEPER_HPP
