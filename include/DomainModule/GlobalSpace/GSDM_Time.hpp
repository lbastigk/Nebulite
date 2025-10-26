/**
 * @file GSDM_Time.hpp
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
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Time){
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Halts time for one frame, meaning you can halt time by continuously calling this function
     * 
     * @param argc Argument count.
     * @param argv Argument vector: no arguments available.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_haltOnce(int argc,  char* argv[]);
    static std::string const time_haltOnce_name;
    static std::string const time_haltOnce_desc;

    /**
     * @brief Locks time with lock provided, meaning time will not progress until unlocked.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the locks name.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_lock(int argc,  char* argv[]);
    static std::string const time_lock_name;
    static std::string const time_lock_desc;

    /**
     * @brief Removes a time lock.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the locks name.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_unlock(int argc,  char* argv[]);
    static std::string const time_unlock_name;
    static std::string const time_unlock_desc;

    /**
     * @brief Removes all time locks.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: no arguments available.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_masterUnlock(int argc,  char* argv[]);
    static std::string const time_masterUnlock_name;
    static std::string const time_masterUnlock_desc;

    /**
     * @brief Sets a fixed delta time for the simulation time.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the fixed delta time in milliseconds.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error time_setFixedDeltaTime(int argc,  char* argv[]);
    static std::string const time_setFixedDeltaTime_name;
    static std::string const time_setFixedDeltaTime_desc;

    //------------------------------------------
    // Category names
    static std::string const time_name;
    static std::string const time_desc;

    //------------------------------------------
    // Variables
    static std::string const key_runtime_t;
    static std::string const key_runtime_t_ms;
    static std::string const key_runtime_dt;
    static std::string const key_runtime_dt_ms;

    static std::string const key_time_t;
    static std::string const key_time_t_ms;
    static std::string const key_time_dt;
    static std::string const key_time_dt_ms;

    static std::string const key_framecount;

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
        bindCategory(time_name, &time_desc);
        bindFunction(&Time::time_haltOnce,           time_haltOnce_name,            &time_haltOnce_desc);
        bindFunction(&Time::time_lock,               time_lock_name,                &time_lock_desc);
        bindFunction(&Time::time_unlock,             time_unlock_name,              &time_unlock_desc);
        bindFunction(&Time::time_masterUnlock,       time_masterUnlock_name,        &time_masterUnlock_desc);
        bindFunction(&Time::time_setFixedDeltaTime,  time_setFixedDeltaTime_name,   &time_setFixedDeltaTime_desc);
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