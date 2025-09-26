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
    Nebulite::Constants::Error haltOnce(int argc, char** argv);

    /**
     * @brief Locks time with lock provided, meaning time will not progress until unlocked.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the locks name.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error lock(int argc, char** argv);

    /**
     * @brief Removes a time lock.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the locks name.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error unlock(int argc, char** argv);

    /**
     * @brief Removes all time locks.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: no arguments available.
     * @return Error code indicating success or failure.
     */
    Nebulite::Constants::Error masterUnlock(int argc, char** argv);

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
        bindFunction(&Time::haltOnce,       "time halt-once",       "Halt time for one frame: time-halt-once");
        bindFunction(&Time::lock,           "time lock",            "Lock time with a name: time-lock <name>");
        bindFunction(&Time::unlock,         "time unlock",          "Unlock time with a name: time-unlock <name>");
        bindFunction(&Time::masterUnlock,   "time master-unlock",   "Unlock all time locks: time-master-unlock");
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
};
} // namespace GlobalSpace
} // namespace DomainModule
} // namespace Nebulite