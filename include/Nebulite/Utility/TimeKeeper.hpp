#ifndef NEBULITE_UTILITY_TIMEKEEPER_HPP
#define NEBULITE_UTILITY_TIMEKEEPER_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <optional>

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Nebulite::Utility::TimeKeeper
 * @brief Manages time-related operations in the Nebulite engine.
 * @details The TimeKeeper class provides functionality to track elapsed time,
 *          to manage frame rates, handle delays, etc.
 *          The start time is set to construction time.
 *          Timer is initialized to not running.
 */
class TimeKeeper{
public:
    /**
     * @brief Constructs a new TimeKeeper object and initializes the timer.
     * @details The start time is set to construction time. Timer is initialized to not running.
     */
    TimeKeeper() noexcept ;

    /**
     * @brief Updates the timer.
     * @details Calculates the delta time since the last update and updates the timers full runtime.
     * @param fixedTimeDelta If a fixed delta time is provided, it will be used instead of the calculated delta time.
     */
    void update(std::optional<std::uint64_t> fixedTimeDelta = std::nullopt);

    /**
     * @brief Starts the timer, updating the running state.
     * @details Initializes the timer and begins tracking elapsed time.
     *          Make sure to call update() before start() to get an accurate dt,
     *          if you don't start the timer immediately after construction.
     */
    void start() noexcept ;

    /**
     * @brief Stops the timer.
     * @details Pauses tracking elapsed time.
     *          Any accumulated time will be preserved.
     *          Note that in stop, `timeDeltaMilliseconds()` will return the last update's delta time.
     *          Make sure to call `update()` before `stop()` to get an accurate dt.
     */
    void stop() noexcept ;

    /**
     * @brief Checks if the timer is currently running.
     * @return True if the timer is running, false otherwise.
     */
    [[nodiscard]] bool isRunning() const noexcept ;

    /**
     * @brief Calculates the projected dt if `update()` were to be called.
     * @details It does this by simulating the passage of time through a direct `system_clock` call for elapsed time.
     *          If the timer is not running, the projected delta time will be zero.
     * @return The projected delta time in milliseconds.
     */
    std::uint64_t dtProjected() noexcept ;

    /**
     * @brief Gets the current time in milliseconds since the timer started.
     * @details Returns the time elapsed since the timer started.
     *          Note that the returned value is not necessarily equal to elapsed system time,
     *          as the update function allows for a custom dt.
     * @return The time elapsed since the timer started in milliseconds.
     */
    [[nodiscard]] std::uint64_t timeMilliseconds() const noexcept ;

    /**
     * @brief Gets the delta time in milliseconds since the last update.
     * @details Returns the time difference between the last two updates.
     *          If the timer is not running, it will return the last update's delta time.
     *          Note that the returned value is not necessarily equal to system time, as the update function allows for a custom dt.
     * @return The time difference between the last two updates in milliseconds.
     */
    [[nodiscard]] std::uint64_t timeDeltaMilliseconds() const noexcept ;

private:
    // Basic values for current time

    /**
     * @brief The start time in milliseconds when the timer was created.
     * @details For Reference: This value is used to calculate the total elapsed time since the timer was created.
     */
    std::uint64_t tStart;

    /**
     * @brief The current time in milliseconds of the last update.
     */
    std::uint64_t tMilliSeconds = 0;

    /**
     * @brief The delta time in milliseconds between the last two updates.
     */
    std::uint64_t deltaTimeMilliSeconds = 0;

    /**
     * @brief Indicates whether the timer is currently running.
     * @details On construction, the timer is off.
     */
    bool running = false;

    /**
     * @struct OnUpdate
     * @brief Stores the timing information for the update phase.
     */
    struct OnUpdate{
        std::uint64_t lastTimeMilliSeconds = 0;
        std::uint64_t tMilliSeconds = 0;
        OnUpdate() = default;
    } onUpdate;

    /**
     * @struct OnSimulation
     * @brief Stores the timing information for the update simulation phase.
     */
    struct OnSimulation{
        std::uint64_t lastTimeMilliSeconds = 0;
        std::uint64_t tMilliSeconds = 0;
        std::uint64_t dt = 0;
        OnSimulation() = default;
    } onSimulation;
};
}   // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_TIMEKEEPER_HPP
