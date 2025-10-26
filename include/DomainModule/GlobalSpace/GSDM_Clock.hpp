/**
 * @file GSDM_Clock.hpp
 * 
 * This file contains the DomainModule of the GlobalSpace for clock management capabilities.
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBALSPACE_GSDM_CLOCK_HPP
#define NEBULITE_DOMAINMODULE_GLOBALSPACE_GSDM_CLOCK_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/JSON.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Clock
 * @brief DomainModule for clock management capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Clock){
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Adds a clock to the global clock list.
     */
    Nebulite::Constants::Error addClock(int argc,  char* argv[]);
    static std::string const addClock_name;
    static std::string const addClock_desc;

    //------------------------------------------
    // Keys in the global document

    /**
     * @brief Key for accessing the list of active clocks.
     * 
     * access with key_arr_active_clocks + ".ms" + <interval_padded>
     */
    static std::string const key_arr_active_clocks;

    /**
     * @brief Key for accessing the status of each clock.
     * 
     * Current status of each clock (0 or 1), access with key_doc_status_clocks + ".ms" + <interval_padded>
     * 
     * Example: key_doc_status_clocks + ".ms000100" for the clock with 100ms interval
     */
    static std::string const key_doc_status_clocks; 

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Clock){
        //------------------------------------------
        // Binding functions to the FuncTree
        bindFunction(&Clock::addClock,          addClock_name,                 &addClock_desc);

        // Read clock list from document
        readClocksFromDocument();
    }

private:
    /**
     * @brief Current time in milliseconds since the program started.
     * 
     * Extracted from the global document, calculated via GSDM_Time.
     */
    uint64_t current_time_ms = 0;

    /**
     * @struct ClockEntry
     * @brief Structure representing a clock entry in the global clock list.
     */
    struct ClockEntry {
        uint64_t last_trigger_ms;   // Last time the clock was triggered
        uint64_t interval_ms;       // Trigger interval in milliseconds
        double* globalReference;    // Pointer to the global document entry

        ClockEntry(uint64_t interval, Nebulite::Utility::JSON* doc, uint64_t current_time);

        /**
         * @brief Updates the clock entry, setting the global reference based on the timer.
         * 
         * If dt is greater than or equal to the interval, sets the global reference to 1.0.
         * Otherwise, sets it to 0.0.
         */
        void update(uint64_t const& current_time);
    };

    /**
     * @brief Map of clock interval to ClockEntry.
     * 
     * We use a hashmap so we can easily create new entries and check existing ones.
     */
    absl::flat_hash_map<uint64_t, ClockEntry> clockEntries;

    /**
     * @brief Reads the clock list from the global document.
     * 
     * This function initializes the clocks hashmap based on available entries in the global document.
     * This ensures that any pre-configured clocks are loaded and ready for use.
     */
    void readClocksFromDocument();

    /**
     * @brief Converts a clock interval in milliseconds to a key string.
     * 
     * This function takes a clock interval in milliseconds and converts it into a key string with zero-padding
     * that can be used to access the corresponding clock entry in the global document.
     * 
     * While up to uint64_t is supported, practical clock intervals should be much lower, so we don't pad for the full length.
     * This makes the keys more manageable while still being properly sorted for typical use cases.
     * 
     * Example: An interval of 100ms becomes "ms000100".
     * 
     * @param interval_ms The clock interval in milliseconds.
     * @return The key string for the clock entry.
     */
    static std::string intervalToKey(uint64_t const& interval_ms){
        static uint16_t padding = 6; // Not enough for uint64_t max value, but reasonable for practical clock intervals
        return "ms" + std::to_string(interval_ms).insert(0, padding - std::to_string(interval_ms).length(), '0');
    }
};
}   // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_GSDM_CLOCK_HPP