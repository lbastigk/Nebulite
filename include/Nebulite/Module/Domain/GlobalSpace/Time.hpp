#ifndef NEBULITE_MODULE_DOMAIN_GLOBALSPACE_TIME_HPP
#define NEBULITE_MODULE_DOMAIN_GLOBALSPACE_TIME_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <string>
#include <unordered_set>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Data/Document/KeyGroup.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
#include "Nebulite/Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {
/**
 * @class Nebulite::Module::Domain::GlobalSpace::Time
 * @brief DomainModule for time management within the GlobalSpace.
 */
class Time final : public Base::DomainModule<Core::GlobalSpace> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event time_haltOnce();
    static auto constexpr time_haltOnceName = "time halt-once";
    static auto constexpr time_haltOnceDesc = "Halts time for one frame\n"
        "Meaning you can halt time by continuously calling this function.\n"
        "\n"
        "Usage: time halt-once\n";

    [[nodiscard]] Constants::Event time_lock(int argc, char const** argv);
    static auto constexpr time_lockName = "time lock";
    static auto constexpr time_lockDesc = "Locks time with lock provided,\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time lock <lockName>\n"
        "\n"
        "<lockName> : Name of the lock to add. Any string without whitespace is valid.\n";

    [[nodiscard]] Constants::Event time_unlock(int argc, char const** argv);
    static auto constexpr time_unlockName = "time unlock";
    static auto constexpr time_unlockDesc = "Removes a time lock.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time unlock <lockName>\n"
        "\n"
        "<lockName> : Name of the lock to remove. Must match an existing lock.\n";

    [[nodiscard]] Constants::Event time_masterUnlock();
    static auto constexpr time_masterUnlockName = "time master-unlock";
    static auto constexpr time_masterUnlockDesc = "Removes all time locks.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time master-unlock\n";

    [[nodiscard]] Constants::Event time_setFixedDeltaTime(int argc, char const** argv);
    static auto constexpr time_setFixedDeltaTimeName = "time set-fixed-dt";
    static auto constexpr time_setFixedDeltaTimeDesc = "Sets a fixed delta time in milliseconds for the simulation time.\n"
        "\n"
        "Usage: time set-fixed-dt <dt_ms>\n"
        "\n"
        "<dt_ms> : Fixed delta time in milliseconds. Use 0 to reset to real delta time.\n";

    //------------------------------------------
    // Categories

    static auto constexpr timeName = "time";
    static auto constexpr timeDesc = R"(Commands for time management)";

    //------------------------------------------
    // Variables
    struct Key : Data::KeyGroup<"time."> {
        // Keys for simulation time
        // May be unequal to actual time, if a custom dt is set
        static auto constexpr time_t = makeScoped("t");
        static auto constexpr time_t_ms = makeScoped("t_ms");
        static auto constexpr time_dt = makeScoped("dt");
        static auto constexpr time_dt_ms = makeScoped("dt_ms");
        static auto constexpr time_locked = makeScoped("lock");

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
    explicit Time(ConstructorParams const& params) : DomainModule(params) {
        //------------------------------------------
        // Start timers
        SimulationTime.start();
        RealTime.start();

        //------------------------------------------
        // Bind functions
        bindCategory(timeName, timeDesc);
        bindFunction(&Time::time_haltOnce, time_haltOnceName, time_haltOnceDesc);
        bindFunction(&Time::time_lock, time_lockName, time_lockDesc);
        bindFunction(&Time::time_unlock, time_unlockName, time_unlockDesc);
        bindFunction(&Time::time_masterUnlock, time_masterUnlockName, time_masterUnlockDesc);
        bindFunction(&Time::time_setFixedDeltaTime, time_setFixedDeltaTimeName, time_setFixedDeltaTimeDesc);
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
    std::uint64_t fixedDeltaTime = 0;
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_MODULE_DOMAIN_GLOBALSPACE_TIME_HPP
