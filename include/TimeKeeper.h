/**
 * @file TimeKeeper.h
 * @brief Provides time management utilities for the Nebulite engine.
 * 
 * This file contains classes and functions to manage and track time,
 * including timers, delays, and frame rate calculations.
 * 
 * @author Leo Bastigkeit
 * @date August 15, 2025
 */

#include "Time.h"

/**
 * @class TimeKeeper
 * @brief Manages time-related operations in the Nebulite engine.
 * 
 * The TimeKeeper class provides functionality to track elapsed time,
 * manage frame rates, and handle delays.
 */
class TimeKeeper{
public:

    TimeKeeper(){
        t_ms = Time::gettime() - t_start;
        dt_ms = 0;
        loop_t_ms = t_ms;
        loop_last_t_ms = t_ms;
    }

    /**
     * @brief Updates the timer.
     * 
     * This function calculates the delta time since the last update and updates the timer.
     * 
     * @param fixed_dt_ms If greater than 0, this value will be used as the delta time instead of the calculated value.
     */
    void update(uint64_t fixed_dt_ms = 0){
        // Calculate dt from last update call
        // Regardless of whether the timer is running or not
        loop_last_t_ms = loop_t_ms;
        loop_t_ms = Time::gettime() - t_start;
        loop_dt = loop_t_ms - loop_last_t_ms;

        if(fixed_dt_ms > 0){
            // If fixed dt is set, use that
            loop_dt = fixed_dt_ms;
        }

        // If running, add dt to values
        if(running){
            dt_ms = loop_dt;
        }
        else{
            dt_ms = 0;
        }
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
            uint64_t sim_last_t_ms = loop_t_ms;
            uint64_t sim_t_ms = Time::gettime() - t_start;
            uint64_t sim_dt = sim_t_ms - sim_last_t_ms;
            return sim_dt;
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
    uint64_t t_ms;
    uint64_t dt_ms;

    // Status
    bool running = false; // On construction, timer is off

    // Needed values for dt calculation on update
    uint64_t loop_last_t_ms;
    uint64_t loop_t_ms;
    uint64_t loop_dt;

    // Start value for reference
    const uint64_t t_start = Time::gettime();
};