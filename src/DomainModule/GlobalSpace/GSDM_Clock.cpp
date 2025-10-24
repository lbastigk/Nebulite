#include "DomainModule/GlobalSpace/GSDM_Clock.hpp"

#include "Core/GlobalSpace.hpp"
#include "DomainModule/GlobalSpace/GSDM_Time.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

Nebulite::Constants::Error Clock::update() {
    // Update current time from document
    current_time_ms = domain->getDoc()->get<uint64_t>(Time::key_time_t_ms);

    // Check all Timers against their desired time
    for(auto& clockEntry : clockEntries) {
        clockEntry.second.update(current_time_ms);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}

void Clock::readClocksFromDocument() {
    // Remove all existing entries
    clockEntries.clear();

    // Read all clocks from the document
    auto type = domain->getDoc()->memberCheck(key_arr_active_clocks);
    if(type != Nebulite::Utility::JSON::KeyType::array) {
        // No clocks found, nothing to do
        return;
    }

    uint64_t size = domain->getDoc()->memberSize(key_arr_active_clocks);

    for(uint64_t i = 0; i < size; i++) {
        std::string key = key_arr_active_clocks + "[" + std::to_string(i) + "]";
        auto interval_type = domain->getDoc()->memberCheck(key);
        if(interval_type != Nebulite::Utility::JSON::KeyType::value) {
            // Invalid entry, skip
            continue;
        }

        uint64_t interval_ms = domain->getDoc()->get<uint64_t>(key);
        if(interval_ms < 1) {
            // Invalid interval, skip
            continue;
        }

        // Create new ClockEntry
        clockEntries.emplace(interval_ms, ClockEntry(interval_ms, domain->getDoc(), current_time_ms));
    }
}

//------------------------------------------
// Functions

Nebulite::Constants::Error Clock::addClock(int argc,  char* argv[]){
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if(argc > 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    // Parse interval
    uint64_t interval_ms = 0;
    try{
        interval_ms = std::stoull(argv[1]);
    }
    catch(const std::exception& e){
        return Nebulite::Constants::ErrorTable::addError(__FUNCTION__ + std::string(": Invalid interval argument, must be a positive integer"), Nebulite::Constants::Error::NON_CRITICAL);
    }

    if(interval_ms < 1){
        return Nebulite::Constants::ErrorTable::addError(__FUNCTION__ + std::string(": Interval must be greater than 0"), Nebulite::Constants::Error::NON_CRITICAL);
    }

    // Check if clock already exists
    if(clockEntries.find(interval_ms) != clockEntries.end()){
        return Nebulite::Constants::ErrorTable::addError(__FUNCTION__ + std::string(": Clock with this interval already exists"), Nebulite::Constants::Error::NON_CRITICAL);
    }

    // Add to document
    std::string key = key_arr_active_clocks + "[" + std::to_string(domain->getDoc()->memberSize(key_arr_active_clocks)) + "]";
    domain->getDoc()->set(key, interval_ms);

    // Create new ClockEntry
    clockEntries.emplace(interval_ms, ClockEntry(interval_ms, domain->getDoc(), current_time_ms));

    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Clock::addClock_name = "add-clock";
std::string const Clock::addClock_desc = "Adds a clock with specified interval (ms) to the global clock system";

//------------------------------------------
// Keys
std::string const Clock::key_arr_active_clocks = "clocks.active";
std::string const Clock::key_doc_status_clocks = "clocks.status";

//------------------------------------------
// ClockEntry

Clock::ClockEntry::ClockEntry(uint64_t interval, Nebulite::Utility::JSON* doc, uint64_t current_time) : 
    last_trigger_ms(current_time),
    interval_ms(interval)
{
    // Extract reference to global document entry
    std::string key = key_doc_status_clocks + "." + intervalToKey(interval_ms);
    doc->set(key, 0.0); // Initialize to 0.0
    this->globalReference = doc->getStableDoublePointer(key);
}

void Clock::ClockEntry::update(uint64_t const& current_time){
    // Check projected dt of timer
    if(current_time - last_trigger_ms >= interval_ms){
        // Instead of setting last_trigger_ms to current_time, 
        // we set it forward by as much interval_ms as possible to avoid drift
        // in case of delays
        uint64_t dt = current_time - last_trigger_ms;
        uint64_t intervals_passed = dt / interval_ms;
        last_trigger_ms += intervals_passed * interval_ms;
        *globalReference = 1.0;
    }
    else{
        // Projected dt not reached, set back to 0
        *globalReference = 0.0;
    }
}

}   // namespace Nebulite::DomainModule::GlobalSpace