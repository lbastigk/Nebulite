/**
 * @file GSDM_Clock.hpp
 * 
 * This file contains the DomainModule of the GlobalSpace for clock management capabilities.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Clock
 * @brief DomainModule for clock management capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Clock) {
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
    Nebulite::Constants::Error addClock(int argc, char** argv);
    static const std::string addClock_name;
    static const std::string addClock_desc;

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
    absl::flat_hash_map<uint64_t, Nebulite::Utility::TimeKeeper> clocks; // Pair of time in ms and TimeKeeper 

    /**
     * @brief Reads the clock list from the global document.
     * 
     * This function initializes the clocks hashmap based on available entries in the global document.
     * This ensures that any pre-configured clocks are loaded and ready for use.
     */
    void readClocksFromDocument();
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite