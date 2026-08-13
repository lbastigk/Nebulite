//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <ranges>
#include <string>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Clock.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Time.hpp"

//------------------------------------------
namespace {
/**
 * @brief Converts a clock interval in milliseconds to a key string.
 * @details Takes a clock interval in milliseconds and converts it into a key string with zero-padding
 *          that can be used to access the corresponding clock entry in the global document.
 *          While up to std::uint64_t is supported, practical clock intervals should be much lower, so we don't pad for the full length.
 *          This makes the keys more manageable while still being properly sorted for typical use cases.
 *          Example: An interval of 100ms becomes "ms000100".
 * @param intervalMilliSeconds The clock interval in milliseconds.
 * @return The key string for the clock entry.
 */
std::string intervalToKey(std::uint64_t const intervalMilliSeconds) {
    static std::uint16_t constexpr padding = 6; // Not enough for std::uint64_t max value, but reasonable for practical clock intervals
    return "ms" + std::to_string(intervalMilliSeconds).insert(0, padding - std::to_string(intervalMilliSeconds).length(), '0');
}
} // namespace

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

Constants::Event Clock::updateHook() {
    // Update current time from document
    currentTimeMilliSeconds = moduleScope.get<uint64_t>(Time::Key::timeMilliSeconds).value_or(0);

    // Check all Timers against their desired time
    for (auto& clockEntry : std::views::values(clockEntries)) {
        clockEntry.update(currentTimeMilliSeconds);
    }
    return Constants::Event::success;
}

//------------------------------------------
// Functions

Constants::Event Clock::addClock(int const argc, char const** argv) {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (argc > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    // Parse interval
    std::uint64_t intervalMilliSeconds = 0;
    try {
        intervalMilliSeconds = std::stoull(argv[1]);
    } catch (...) {
        domain.capture.warning.println("Invalid interval argument, must be a positive integer.");
        return Constants::Event::warning;
    }

    if (intervalMilliSeconds < 1) {
        domain.capture.warning.println("Interval must be greater than 0");
        return Constants::Event::warning;
    }

    // Check if clock already exists
    if (clockEntries.find(intervalMilliSeconds) != clockEntries.end()) {
        domain.capture.warning.println("Clock with this interval already exists.");
        return Constants::Event::warning;
    }

    // Add to document
    auto const key = Key::activeClocks.addIndex(moduleScope.memberSize(Key::activeClocks));
    moduleScope.set(key, intervalMilliSeconds);

    // Create new ClockEntry
    clockEntries.emplace(intervalMilliSeconds, ClockEntry(intervalMilliSeconds, moduleScope, currentTimeMilliSeconds));

    return Constants::Event::success;
}

//------------------------------------------
// ClockEntry

Clock::ClockEntry::ClockEntry(std::uint64_t const interval, Data::JsonScope& doc, std::uint64_t const currentTime) :
    lastTriggerMilliSeconds(currentTime),
    intervalMilliSeconds(interval) {
    // Extract reference to global document entry
    auto const key = Key::clockStatus.addMember(intervalToKey(intervalMilliSeconds));
    doc.set(key, 0.0); // Initialize to 0.0
    this->globalReference = doc.getStableDoublePointer(key);
}

void Clock::ClockEntry::update(std::uint64_t const currentTime) {
    // Check projected dt of timer
    if (currentTime - lastTriggerMilliSeconds >= intervalMilliSeconds) {
        // Instead of setting lastTriggerMilliSeconds to currentTime,
        // we set it forward by as much intervalMilliSeconds as possible to avoid drift
        // in case of delays
        std::uint64_t const dt = currentTime - lastTriggerMilliSeconds;
        std::uint64_t const intervalsPassed = dt / intervalMilliSeconds;
        lastTriggerMilliSeconds += intervalsPassed * intervalMilliSeconds;
        *globalReference = 1.0;
    } else {
        // Projected dt not reached, set back to 0
        *globalReference = 0.0;
    }
}

//------------------------------------------
// Private Functions

void Clock::readClocksFromDocument() {
    // Remove all existing entries
    clockEntries.clear();

    // Read all clocks from the document
    if (moduleScope.memberType(Key::activeClocks) != Data::KeyType::array) {
        // No clocks found, nothing to do
        return;
    }
    for (auto const size = moduleScope.memberSize(Key::activeClocks); auto const key : Key::activeClocks.getArrayKeys(size)) {
        if (auto const intervalType = moduleScope.memberType(key); intervalType != Data::KeyType::value) {
            // Invalid entry, skip
            continue;
        }

        auto const intervalMilliSeconds = moduleScope.get<uint64_t>(key).value_or(0);
        if (intervalMilliSeconds < 1) {
            // Invalid interval, skip
            continue;
        }

        // Create new ClockEntry
        clockEntries.emplace(intervalMilliSeconds, ClockEntry(intervalMilliSeconds, moduleScope, currentTimeMilliSeconds));
    }
}

} // namespace Nebulite::Module::Domain::GlobalSpace
