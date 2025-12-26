/**
 * @file Time.hpp
 * @brief Contains the declaration of the Time DomainModule for the GlobalSpace domain.
 */

#ifndef NEBULITE_GSDM_TIME_HPP
#define NEBULITE_GSDM_TIME_HPP

//------------------------------------------
// Includes

// Standard library
#include <unordered_set>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
}

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Time
 * @brief DomainModule for time management within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Time) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @brief Halts time for one frame, meaning you can halt time by continuously calling this function
     * 
     * @param argc Argument count.
     * @param argv Argument vector: no arguments available.
     * @return Error code indicating success or failure.
     */
    Constants::Error time_haltOnce(int argc, char** argv);
    static std::string_view constexpr time_haltOnce_name = "time halt-once";
    static std::string_view constexpr time_haltOnce_desc = "Halts time for one frame\n"
        "Meaning you can halt time by continuously calling this function.\n"
        "\n"
        "Usage: time halt-once\n";

    /**
     * @brief Locks time with lock provided, meaning time will not progress until unlocked.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the locks name.
     * @return Error code indicating success or failure.
     */
    Constants::Error time_lock(int argc, char** argv);
    static std::string_view constexpr time_lock_name = "time lock";
    static std::string_view constexpr time_lock_desc = "Locks time with lock provided,\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time lock <lock_name>\n"
        "\n"
        "<lock_name> : Name of the lock to add. Any string without whitespace is valid.\n";

    /**
     * @brief Removes a time lock.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the locks name.
     * @return Error code indicating success or failure.
     */
    Constants::Error time_unlock(int argc, char** argv);
    static std::string_view constexpr time_unlock_name = "time unlock";
    static std::string_view constexpr time_unlock_desc = "Removes a time lock.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time unlock <lock_name>\n"
        "\n"
        "<lock_name> : Name of the lock to remove. Must match an existing lock.\n";

    /**
     * @brief Removes all time locks.
     * 
     * Time can only progress if no locks are present.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: no arguments available.
     * @return Error code indicating success or failure.
     */
    Constants::Error time_masterUnlock(int argc, char** argv);
    static std::string_view constexpr time_masterUnlock_name = "time master-unlock";
    static std::string_view constexpr time_masterUnlock_desc = "Removes all time locks.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time master-unlock\n";

    /**
     * @brief Sets a fixed delta time for the simulation time.
     * 
     * @param argc Argument count.
     * @param argv Argument vector: the fixed delta time in milliseconds.
     * @return Error code indicating success or failure.
     */
    Constants::Error time_setFixedDeltaTime(int argc, char** argv);
    static std::string_view constexpr time_setFixedDeltaTime_name = "time set-fixed-dt";
    static std::string_view constexpr time_setFixedDeltaTime_desc = "Sets a fixed delta time in milliseconds for the simulation time.\n"
        "\n"
        "Usage: time set-fixed-dt <dt_ms>\n"
        "\n"
        "<dt_ms> : Fixed delta time in milliseconds. Use 0 to reset to real delta time.\n";

    //------------------------------------------
    // Category names
    static std::string_view constexpr time_name = "time";
    static std::string_view constexpr time_desc = R"(Commands for time management)";

    //------------------------------------------
    // Variables
    static std::string_view constexpr key_runtime_t = "runtime.t";
    static std::string_view constexpr key_runtime_t_ms = "runtime.t_ms";
    static std::string_view constexpr key_runtime_dt = "runtime.dt";
    static std::string_view constexpr key_runtime_dt_ms = "runtime.dt_ms";

    static std::string_view constexpr key_time_t = "time.t";
    static std::string_view constexpr key_time_t_ms = "time.t_ms";
    static std::string_view constexpr key_time_dt = "time.dt";
    static std::string_view constexpr key_time_dt_ms = "time.dt_ms";

    static std::string_view constexpr key_framecount = "frameCount";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Time) {
        //------------------------------------------
        // Start timers
        SimulationTime.start();
        RealTime.start();

        //------------------------------------------
        // Bind functions
        (void)bindCategory(time_name, time_desc);
        bindFunction(&Time::time_haltOnce, time_haltOnce_name, time_haltOnce_desc);
        bindFunction(&Time::time_lock, time_lock_name, time_lock_desc);
        bindFunction(&Time::time_unlock, time_unlock_name, time_unlock_desc);
        bindFunction(&Time::time_masterUnlock, time_masterUnlock_name, time_masterUnlock_desc);
        bindFunction(&Time::time_setFixedDeltaTime, time_setFixedDeltaTime_name, time_setFixedDeltaTime_desc);
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
    Utility::TimeKeeper SimulationTime;

    /**
     * @brief Full application runtime
     */
    Utility::TimeKeeper RealTime;

    /**
     * @brief Amount of frames rendered
     */
    uint64_t frameCount = 0;

    /**
     * @brief Fixed delta time for the simulation time. If 0, uses real delta time.
     */
    uint64_t fixedDeltaTime = 0;
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_TIME_HPP
