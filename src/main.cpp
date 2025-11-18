//------------------------------------------
//     _   ____________  __  ____    ________________
//    / | / / ____/ __ )/ / / / /   /  _/_  __/ ____/
//   /  |/ / __/ / __  / / / / /    / /  / / / __/   
//  / /|  / /___/ /_/ / /_/ / /____/ /  / / / /___   
// /_/ |_/_____/_____/\____/_____/___/ /_/ /_____/   
//                                                     

/**
 * @file main.cpp
 * 
 * @brief Main entry point for the NEBULITE engine.
 * 
 * This file initializes the core systems, parses command-line arguments, 
 * and coordinates execution scenarios such as
 * loading levels or running performance benchmarks.
 *
 * Usage:
 *   Nebulite <command> <options>
 *
 * Example Commands:
 * ```bash
 * ./bin/Nebulite spawn <RenderObject.jsonc>      # Load a single renderobject
 * ./bin/Nebulite env-load <level.jsonc>          # Load and render a level
 * ./bin/Nebulite task <task.nebs>                # Run a task file
 * ```
 */

//------------------------------------------
// Includes

// Standard library
#include <exception>
#include <string>

// Nebulite
#include "Nebulite.hpp"                             // Main Nebulite include
#include "Constants/ErrorTypes.hpp"                 // For error handling
#include "DomainModule/GlobalSpace/Debug.hpp"  // For turning error logging off/on

//------------------------------------------
// Constants

namespace Nebulite::Constants {
    struct MainReturnValues {
        static constexpr int success = 0;               ///< Return value for successful execution
        static constexpr int criticalError = 1;         ///< Return value for execution halted by critical error
        static constexpr int logCloseError = 2;         ///< Return value for failure to close error log
        static constexpr int logCloseException = 3;     ///< Return value for exception during error log closure
    };
}   // namespace Nebulite::Constants

//------------------------------------------
// NEBULITE main

/**
 * @brief Main function for the NEBULITE engine.
 * 
 * This function initializes the engine, sets up the global space, and processes command-line arguments.
 * 
 * No secondary binary is required for tests or special scenarios: 
 * 
 * - Expand the GlobalSpace for specials scenarios and call with: `./bin/Nebulite my-scenario`
 * 
 * - Create task files for scripted tests and call with `./bin/Nebulite task </path/to/task/file.nebs>`
 * 
 * Use functions for debugging of specific features and taskFiles for complex, scripted scenarios and tests.
 *
 * @todo:   settings.jsonc: Renderer size, fps setting (Input Mapping is already a work in progress. See GSDM_InputMapping.h)
 */
int main(int const argc, char* argv[]){
    //------------------------------------------
    // Initialize the global space, parse command line arguments
    auto const binaryName = std::string(argv[0]);
    Nebulite::global().parseCommandLineArguments(argc, const_cast<char const**>(argv));
    
    //------------------------------------------
    // Render loop
    Nebulite::Constants::Error lastCriticalResult; // Last critical error result
    do{
        // At least one loop, to handle command line arguments
        lastCriticalResult = Nebulite::global().update();
    } while(Nebulite::global().shouldContinueLoop());

    //------------------------------------------
    // Exit

    // Check if we had a critical error
    const bool criticalStop = lastCriticalResult.isCritical();

    // Destroy renderer
    Nebulite::global().getRenderer()->destroy();

    // Inform user about any errors and return error code
    if(criticalStop){
        Nebulite::cerr() << "Critical Error: " << lastCriticalResult.getDescription() << "\n";
    }

    // Parser handles if error files need to be closed
    try{
        if (Nebulite::Constants::Error const result = Nebulite::global().parseStr(binaryName + " " + Nebulite::DomainModule::GlobalSpace::Debug::errorlog_name + " off"); result .isCritical()){
            Nebulite::cerr() << "Error disabling error log: " << result.getDescription() << "\n";
            return Nebulite::Constants::MainReturnValues::logCloseError;   // Closing log failed without exceptions
        }
    } catch(std::exception const& e){
        Nebulite::cerr() << "Error closing error log: " << e.what() << "\n";
        return Nebulite::Constants::MainReturnValues::logCloseException;   // Return a different error code for log closing failure with exceptions
    }

    // Return 1 on critical stop, 0 otherwise
    if (criticalStop) {
        return Nebulite::Constants::MainReturnValues::criticalError;
    }
    return Nebulite::Constants::MainReturnValues::success;
}