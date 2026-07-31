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

    [[nodiscard]] Constants::Event timeHaltOnce();
    static auto constexpr timeHaltOnceName = "time halt-once";
    static auto constexpr timeHaltOnceDesc = "Halts time for one frame\n"
        "Meaning you can halt time by continuously calling this function.\n"
        "\n"
        "Usage: time halt-once\n";

    [[nodiscard]] Constants::Event timeLock(int argc, char const** argv);
    static auto constexpr timeLockName = "time lock";
    static auto constexpr timeLockDesc = "Locks time with lock provided,\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time lock <lockName>\n"
        "\n"
        "<lockName> : Name of the lock to add. Any string without whitespace is valid.\n";

    [[nodiscard]] Constants::Event timeUnlock(int argc, char const** argv);
    static auto constexpr timeUnlockName = "time unlock";
    static auto constexpr timeUnlockDesc = "Removes a time lock.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time unlock <lockName>\n"
        "\n"
        "<lockName> : Name of the lock to remove. Must match an existing lock.\n";

    [[nodiscard]] Constants::Event timeMasterUnlock();
    static auto constexpr timeMasterUnlockName = "time master-unlock";
    static auto constexpr timeMasterUnlockDesc = "Removes all time locks.\n"
        "Time can only progress if no locks are present.\n"
        "\n"
        "Usage: time master-unlock\n";

    [[nodiscard]] Constants::Event timeSetFixedDeltaTime(int argc, char const** argv);
    static auto constexpr timeSetFixedDeltaTimeName = "time set-fixed-dt";
    static auto constexpr timeSetFixedDeltaTimeDesc = "Sets a fixed delta time in milliseconds for the simulation time.\n"
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
        static auto constexpr time = makeScoped("t");
        static auto constexpr timeMilliSeconds = makeScoped("t_ms");
        static auto constexpr deltaTime = makeScoped("dt");
        static auto constexpr timeDeltaTimeMilliSeconds = makeScoped("dt_ms");
        static auto constexpr timeLocked = makeScoped("lock");

        // More specific keys for actual runtime
        static auto constexpr runTime = makeScoped("runtime.t");
        static auto constexpr runtimeMilliSeconds = makeScoped("runtime.t_ms");
        static auto constexpr runTimeDeltaTime = makeScoped("runtime.dt");
        static auto constexpr runTimeDeltaTimeMilliSeconds = makeScoped("runtime.dt_ms");
    };

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit Time(ConstructorParams const& params) : DomainModule(params) {
        //------------------------------------------
        // Start timers
        simulationTime.start();
        realTime.start();

        //------------------------------------------
        // Bind functions
        bindCategory(timeName, timeDesc);
        bindFunction(&Time::timeHaltOnce, timeHaltOnceName, timeHaltOnceDesc);
        bindFunction(&Time::timeLock, timeLockName, timeLockDesc);
        bindFunction(&Time::timeUnlock, timeUnlockName, timeUnlockDesc);
        bindFunction(&Time::timeMasterUnlock, timeMasterUnlockName, timeMasterUnlockDesc);
        bindFunction(&Time::timeSetFixedDeltaTime, timeSetFixedDeltaTimeName, timeSetFixedDeltaTimeDesc);
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
    Utility::TimeKeeper simulationTime;

    /**
     * @brief Full application runtime
     */
    Utility::TimeKeeper realTime;

    /**
     * @brief Fixed delta time for the simulation time. If 0, uses real delta time.
     */
    std::uint64_t fixedDeltaTime = 0;
};
} // namespace Nebulite::Module::Domain::GlobalSpace
#endif // NEBULITE_MODULE_DOMAIN_GLOBALSPACE_TIME_HPP
