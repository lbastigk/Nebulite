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
#include "Data/Document/ScopedKey.hpp"
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

    Constants::Error time_haltOnce();
    static auto constexpr time_haltOnce_name = "time halt-once";
    static auto constexpr time_haltOnce_desc = "Halts time for one frame\n"
        "Meaning you can halt time by continuously calling this function.\n"
        "\n"
        "Usage: time halt-once\n";

    Constants::Error time_lock(int argc, char** argv);
    static auto constexpr time_lock_name = "time lock";
    static auto constexpr time_lock_desc = "Locks time with lock provided,\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time lock <lock_name>\n"
        "\n"
        "<lock_name> : Name of the lock to add. Any string without whitespace is valid.\n";

    Constants::Error time_unlock(int argc, char** argv);
    static auto constexpr time_unlock_name = "time unlock";
    static auto constexpr time_unlock_desc = "Removes a time lock.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time unlock <lock_name>\n"
        "\n"
        "<lock_name> : Name of the lock to remove. Must match an existing lock.\n";

    Constants::Error time_masterUnlock();
    static auto constexpr time_masterUnlock_name = "time master-unlock";
    static auto constexpr time_masterUnlock_desc = "Removes all time locks.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time master-unlock\n";

    Constants::Error time_setFixedDeltaTime(int argc, char** argv);
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
    struct Key {
        static auto constexpr scope = ""; // Allow any scope to access these keys
        static auto constexpr time_t = Data::ScopedKey::create<scope>("time.t");
        static auto constexpr time_t_ms = Data::ScopedKey::create<scope>("time.t_ms");
        static auto constexpr time_dt = Data::ScopedKey::create<scope>("time.dt");
        static auto constexpr time_dt_ms = Data::ScopedKey::create<scope>("time.dt_ms");
        static auto constexpr runtime_t = Data::ScopedKey::create<scope>("runtime.t");
        static auto constexpr runtime_t_ms = Data::ScopedKey::create<scope>("runtime.t_ms");
        static auto constexpr runtime_dt = Data::ScopedKey::create<scope>("runtime.dt");
        static auto constexpr runtime_dt_ms = Data::ScopedKey::create<scope>("runtime.dt_ms");
        static auto constexpr frameCount = Data::ScopedKey::create<scope>("frameCount");
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
        (void)bindCategory(time_name, time_desc);
        BINDFUNCTION(&Time::time_haltOnce, time_haltOnce_name, time_haltOnce_desc);
        BINDFUNCTION(&Time::time_lock, time_lock_name, time_lock_desc);
        BINDFUNCTION(&Time::time_unlock, time_unlock_name, time_unlock_desc);
        BINDFUNCTION(&Time::time_masterUnlock, time_masterUnlock_name, time_masterUnlock_desc);
        BINDFUNCTION(&Time::time_setFixedDeltaTime, time_setFixedDeltaTime_name, time_setFixedDeltaTime_desc);
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
