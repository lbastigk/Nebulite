/**
 * @file GDM_Time.hpp
 * 
 * @brief Contains the declaration of the Time DomainModule for the GlobalSpace domain.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include <functional>
#include <unordered_set>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Time
 * @brief DomainModule for time management within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Time) {
public:
    /**
     * @brief Overwridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Halts time for one frame, meaning you can halt time by continuously calling this function
     * 
     * @param argc Argument count.
     * @param argv Argument vector: no arguments available.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_haltOnce(int argc, char** argv);
    std::string time_haltOnce_desc = R"(Halts time for one frame
    Meaning you can halt time by continuously calling this function.

    Usage: time halt-once
    )";

    /**
     * @brief Locks time with lock provided, meaning time will not progress until unlocked.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the locks name.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_lock(int argc, char** argv);
    std::string time_lock_desc = R"(Locks time with lock provided, 
    Time can only progress if no locks are present.

    Usage: time lock <lock_name>

    <lock_name> : Name of the lock to add. Any string without whitespace is valid.
    )";

    /**
     * @brief Removes a time lock.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the locks name.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_unlock(int argc, char** argv);
    std::string time_unlock_desc = R"(Removes a time lock.
    Time can only progress if no locks are present.

    Usage: time unlock <lock_name>

    <lock_name> : Name of the lock to remove. Must match an existing lock.
    )";

    /**
     * @brief Removes all time locks.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: no arguments available.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_masterUnlock(int argc, char** argv);
    std::string time_masterUnlock_desc = R"(Removes all time locks.
    Time can only progress if no locks are present.

    Usage: time master-unlock
    )";

    /**
     * @brief Sets a fixed delta time for the simulation time.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the fixed delta time in milliseconds.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_setFixedDeltaTime(int argc, char** argv);
    std::string time_setFixedDeltaTime_desc = R"(Sets a fixed delta time in milliseconds for the simulation time.
    Use 0 to disable fixed dt.

    Usage: time set-fixed-dt <dt_ms>
    )";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Time){
        //------------------------------------------
        // Start timers
        SimulationTime.start();
        RealTime.start();

        //------------------------------------------
        // Bind functions
        bindSubtree("time", "Commands for time management");
        bindFunction(&Time::time_haltOnce,           "time halt-once",       time_haltOnce_desc);
        bindFunction(&Time::time_lock,               "time lock",            time_lock_desc);
        bindFunction(&Time::time_unlock,             "time unlock",          time_unlock_desc);
        bindFunction(&Time::time_masterUnlock,       "time master-unlock",   time_masterUnlock_desc);
        bindFunction(&Time::time_setFixedDeltaTime,  "time set-fixed-dt",    time_setFixedDeltaTime_desc);
    }

private:
    /**
     * @brief Indicates if time should be halted this frame.
     */
    bool haltThisFrame = false;

    /**
     * @brief Set of active time locks.
     * 
     * If this set is empty, time can progress.
     */
    std::unordered_set<std::string> timeLocks;

    //------------------------------------------
    // Timekeeper

    /**
     * @brief Simulation time, which can be paused/modified.
     */
	Nebulite::Utility::TimeKeeper SimulationTime;

    /**
     * @brief Full application runtime
     */
	Nebulite::Utility::TimeKeeper RealTime;

    /**
     * @brief Amount of frames rendererd
     */
    uint64_t frameCount = 0;

    /**
     * @brief Fixed delta time for the simulation time. If 0, uses real delta time.
     */
    uint64_t fixedDeltaTime = 0;
};
} // namespace GlobalSpace
} // namespace DomainModule
} // namespace Nebulite