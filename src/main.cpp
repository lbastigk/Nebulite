// ----------------------------------------------------------------------
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
 * ./bin/Nebulite task <task.nebs>                # Run a tasktile
 * ```
 */

// ----------------------------------------------------------------------
// Includes

// Standard library
#include <exception>
#include <string>

// Nebulite
#include "Constants/ErrorTypes.hpp"                 // For error handling
#include "Core/GlobalSpace.hpp"                     // Global Workspace of Nebulite
#include "DomainModule/GlobalSpace/GSDM_Debug.hpp"  // For turning error logging off/on
#include "Utility/Capture.hpp"                      // For error output capture

// ----------------------------------------------------------------------
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
 * - Create taskfiles for scripted tests and call with `./bin/Nebulite task <taskfile.nebs>`
 * 
 * Use functions for debugging of specific features and taskFiles for complex, scripted scenarios and tests.
 *
 * @todo:   settings.jsonc: Renderer size, fps setting (Input Mapping is already a work in progress. See GSDM_InputMapping.h)
 */
int main(int argc, char* argv[]){
    //------------------------------------------
    // Initialize the global space, parse command line arguments
    std::string const binaryName = std::string(argv[0]);
    Nebulite::Core::GlobalSpace globalSpace(binaryName);
    globalSpace.parseCommandLineArguments(argc, const_cast<char const**>(argv));
    
    //------------------------------------------
    // Render loop
    Nebulite::Constants::Error lastCriticalResult = Nebulite::Constants::ErrorTable::NONE(); // Last critical error result
    do{
        // At least one loop, to handle command line arguments
        lastCriticalResult = globalSpace.update();
    } while(globalSpace.shouldContinueLoop());

    //------------------------------------------
    // Exit

    // Check if we had a critical error
    const bool criticalStop = lastCriticalResult.isCritical();

    // Destroy renderer
    globalSpace.getRenderer()->destroy();

    // Inform user about any errors and return error code
    if(criticalStop){
        Nebulite::Utility::Capture::cerr() << "Critical Error: " << lastCriticalResult.getDescription() << "\n";
    }

    // Parser handles if error files need to be closed
    try{
        globalSpace.parseStr(binaryName + " " + Nebulite::DomainModule::GlobalSpace::Debug::errorlog_name + " off");
    } catch(std::exception const& e){
        Nebulite::Utility::Capture::cerr() << "Error closing error log: " << e.what() << "\n";
        return 2;   // Return a different error code for log closing failure
    }
    

    // Return 1 on critical stop, 0 otherwise
    return static_cast<int>(criticalStop);
}