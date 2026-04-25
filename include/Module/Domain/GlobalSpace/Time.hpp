/**
 * @file Time.hpp
 * @brief Contains the declaration of the Time DomainModule for the GlobalSpace domain.
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBALSPACE_TIME_HPP
#define NEBULITE_DOMAINMODULE_GLOBALSPACE_TIME_HPP

//------------------------------------------
// Includes

// Standard library
#include <unordered_set>

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "Constants/StandardCapture.hpp"
#include "Constants/KeyNames.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
} // Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::Time
 * @brief DomainModule for time management within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Time) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event time_haltOnce();
    static auto constexpr time_haltOnce_name = "time halt-once";
    static auto constexpr time_haltOnce_desc = "Halts time for one frame\n"
        "Meaning you can halt time by continuously calling this function.\n"
        "\n"
        "Usage: time halt-once\n";

    [[nodiscard]] Constants::Event time_lock(int argc, char** argv);
    static auto constexpr time_lock_name = "time lock";
    static auto constexpr time_lock_desc = "Locks time with lock provided,\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time lock <lock_name>\n"
        "\n"
        "<lock_name> : Name of the lock to add. Any string without whitespace is valid.\n";

    [[nodiscard]] Constants::Event time_unlock(int argc, char** argv);
    static auto constexpr time_unlock_name = "time unlock";
    static auto constexpr time_unlock_desc = "Removes a time lock.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time unlock <lock_name>\n"
        "\n"
        "<lock_name> : Name of the lock to remove. Must match an existing lock.\n";

    [[nodiscard]] Constants::Event time_masterUnlock();
    static auto constexpr time_masterUnlock_name = "time master-unlock";
    static auto constexpr time_masterUnlock_desc = "Removes all time locks.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time master-unlock\n";

    [[nodiscard]] Constants::Event time_setFixedDeltaTime(int argc, char** argv);
    static auto constexpr time_setFixedDeltaTime_name = "time set-fixed-dt";
    static auto constexpr time_setFixedDeltaTime_desc = "Sets a fixed delta time in milliseconds for the simulation time.\n"
        "\n"
        "Usage: time set-fixed-dt <dt_ms>\n"
        "\n"
        "<dt_ms> : Fixed delta time in milliseconds. Use 0 to reset to real delta time.\n";

    //------------------------------------------
    // Category names
    static auto constexpr time_name = "time";
    static auto constexpr time_desc = R"(Commands for time management)";

    //------------------------------------------
    // Variables
    struct Key : Data::KeyGroup<"time."> {
        // Keys for simulation time
        // May be unequal to actual time, if a custom dt is set
        static auto constexpr time_t = makeScoped("t");
        static auto constexpr time_t_ms = makeScoped("t_ms");
        static auto constexpr time_dt = makeScoped("dt");
        static auto constexpr time_dt_ms = makeScoped("dt_ms");

        // More specific keys for actual runtime
        static auto constexpr runtime_t = makeScoped("runtime.t");
        static auto constexpr runtime_t_ms = makeScoped("runtime.t_ms");
        static auto constexpr runtime_dt = makeScoped("runtime.dt");
        static auto constexpr runtime_dt_ms = makeScoped("runtime.dt_ms");
    };

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
        bindCategory(time_name, time_desc);
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
     * @details If this set is empty, time can progress.
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
     * @brief Fixed delta time for the simulation time. If 0, uses real delta time.
     */
    uint64_t fixedDeltaTime = 0;
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_TIME_HPP
