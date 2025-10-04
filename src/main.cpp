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

// Include the Global Space class
// Initializes callable functions from both user CLI and runtime environment.
// Also sets up the global Renderer used across Tree-based function calls.
#include "Core/GlobalSpace.hpp" 
#include "DomainModule/GlobalSpace/GDM_Debug.hpp"   // For turning error logging off/on

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
 * @todo:   settings.jsonc: Renderer size, fps setting (Input Mapping is already a work in progress. See GDM_InputMapping.h)
 */
int main(int argc, char* argv[]){
    //------------------------------------------
    // Initialize the global space, parse command line arguments
    std::string binaryName = argv[0];
    Nebulite::Core::GlobalSpace globalSpace(binaryName);
    globalSpace.parseCommandLineArguments(argc, argv);
    
    //------------------------------------------
    // Render loop
    bool queueParsed = false;   // Indicates if the task queue has been parsed on this frame render
    bool criticalStop = false;  // Indicates if a critical stop has occurred
    bool continueLoop = true;   // Determines if we continue the loop
    Nebulite::Constants::Error lastCriticalResult = Nebulite::Constants::ErrorTable::NONE(); // Last critical error result
    do {
        // At least one loop, to handle taskQueues

        //------------------------------------------
        /**
         * Parse queue in GlobalSpace.
         * Result determines if a critical stop is initiated.
         * 
         * We do this once before rendering
         * 
         * @note For now, all tasks are parsed even if the program is in console mode.
         *       This is useful as tasks like "spawn" or "echo" are directly executed.
         *       But might break for more complex tasks, so this should be taken into account later on,
         *       e.G. inside the GlobalSpace, checking state of Renderer might be useful
         */
        if(!queueParsed){
            lastCriticalResult = globalSpace.parseQueue();
            criticalStop = (lastCriticalResult != Nebulite::Constants::ErrorTable::NONE());
            queueParsed = true;
        }

        //------------------------------------------
        // Update and render, only if initialized
        // If renderer wasnt initialized, it is still a nullptr
        if (!criticalStop && globalSpace.RendererExists() && globalSpace.getRenderer()->timeToRender()) {
            globalSpace.update();
            bool didUpdate = globalSpace.getRenderer()->tick();

            // Reduce script wait counter if not in console mode or other halting states
            if(didUpdate){
                if(globalSpace.scriptWaitCounter > 0) globalSpace.scriptWaitCounter--; 
                if(globalSpace.scriptWaitCounter < 0) globalSpace.scriptWaitCounter = 0;
            }  

            // Frame was rendered, meaning we potentially have new tasks to process
            queueParsed = false;
        }

        //------------------------------------------
        // Check if we need to continue the loop
        continueLoop = !criticalStop && globalSpace.RendererExists() && !globalSpace.getRenderer()->isQuit();

        // Overwrite: If there is a wait operation and no renderer exists, 
        // we need to continue the loop and decrease scriptWaitCounter
        if(globalSpace.scriptWaitCounter > 0 && !globalSpace.RendererExists()){
            continueLoop = true;
            globalSpace.scriptWaitCounter--;

            // Parse new tasks on next loop
            queueParsed = false;
        }
    /**
     * @note It might be tempting to add the condition that all tasks are done,
     *       but this could cause issues if the user wishes to quit while a task is still running.
     */
    } while (continueLoop);

    //------------------------------------------
    // Exit

    // Destroy renderer
    if(globalSpace.RendererExists()) globalSpace.getRenderer()->destroy();

    // Inform user about any errors and return error code
    if(criticalStop){
        std::cerr << "Critical Error: " << lastCriticalResult.getDescription() << std::endl;
    }

    // Parser handles if error files need to be closed
    globalSpace.parseStr(binaryName + " " + Nebulite::DomainModule::GlobalSpace::Debug::errorlog_name + " off");

    // Return 1 on critical stop, 0 otherwise
    return (int)criticalStop;
}