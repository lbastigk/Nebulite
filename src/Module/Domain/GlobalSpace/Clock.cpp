//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <ranges>
#include <string>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Core/GlobalSpace.hpp"
#include "Data/Document/KeyType.hpp"
#include "Module/Domain/GlobalSpace/Clock.hpp"
#include "Module/Domain/GlobalSpace/Time.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

Constants::Event Clock::updateHook() {
    // Update current time from document
    current_time_ms = moduleScope.get<uint64_t>(Time::Key::time_t_ms).value_or(0);

    // Check all Timers against their desired time
    for (auto& clockEntry : std::views::values(clockEntries)) {
        clockEntry.update(current_time_ms);
    }
    return Constants::Event::Success;
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
    std::uint64_t interval_ms = 0;
    try {
        interval_ms = std::stoull(argv[1]);
    } catch (...) {
        domain.capture.warning.println("Invalid interval argument, must be a positive integer.");
        return Constants::Event::Warning;
    }

    if (interval_ms < 1) {
        domain.capture.warning.println("Interval must be greater than 0");
        return Constants::Event::Warning;
    }

    // Check if clock already exists
    if (clockEntries.find(interval_ms) != clockEntries.end()) {
        domain.capture.warning.println("Clock with this interval already exists.");
        return Constants::Event::Warning;
    }

    // Add to document
    auto const key = Key::arr_active_clocks.addIndex(moduleScope.memberSize(Key::arr_active_clocks));
    moduleScope.set(key, interval_ms);

    // Create new ClockEntry
    clockEntries.emplace(interval_ms, ClockEntry(interval_ms, moduleScope, current_time_ms));

    return Constants::Event::Success;
}

//------------------------------------------
// ClockEntry

Clock::ClockEntry::ClockEntry(std::uint64_t const interval, Data::JsonScope& doc, std::uint64_t const current_time) :
    last_trigger_ms(current_time),
    interval_ms(interval) {
    // Extract reference to global document entry
    auto const key = Key::doc_status_clocks.addMember(intervalToKey(interval_ms));
    doc.set(key, 0.0); // Initialize to 0.0
    this->globalReference = doc.getStableDoublePointer(key);
}

void Clock::ClockEntry::update(std::uint64_t const current_time) {
    // Check projected dt of timer
    if (current_time - last_trigger_ms >= interval_ms) {
        // Instead of setting last_trigger_ms to current_time,
        // we set it forward by as much interval_ms as possible to avoid drift
        // in case of delays
        std::uint64_t const dt = current_time - last_trigger_ms;
        std::uint64_t const intervals_passed = dt / interval_ms;
        last_trigger_ms += intervals_passed * interval_ms;
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
    if (moduleScope.memberType(Key::arr_active_clocks) != Data::KeyType::array) {
        // No clocks found, nothing to do
        return;
    }

    std::uint64_t const size = moduleScope.memberSize(Key::arr_active_clocks);

    for (std::uint64_t i = 0; i < size; i++) {
        auto key = Key::arr_active_clocks.addIndex(i);
        if (auto const interval_type = moduleScope.memberType(key); interval_type != Data::KeyType::value) {
            // Invalid entry, skip
            continue;
        }

        auto interval_ms = moduleScope.get<uint64_t>(key).value_or(0);
        if (interval_ms < 1) {
            // Invalid interval, skip
            continue;
        }

        // Create new ClockEntry
        clockEntries.emplace(interval_ms, ClockEntry(interval_ms, moduleScope, current_time_ms));
    }
}

std::string Clock::intervalToKey(std::uint64_t const interval_ms) {
    static std::uint16_t constexpr padding = 6; // Not enough for std::uint64_t max value, but reasonable for practical clock intervals
    return "ms" + std::to_string(interval_ms).insert(0, padding - std::to_string(interval_ms).length(), '0');
}

} // namespace Nebulite::Module::Domain::GlobalSpace
