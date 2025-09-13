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
 *
 */
int main(int argc, char* argv[]){
    //------------------------------------------
    // Initialize the global space
    std::string binaryName = argv[0];
    Nebulite::Core::GlobalSpace globalSpace(binaryName);

    //------------------------------------------
    // Add main args to taskList, split by ';'
    if (argc > 1) {
        std::ostringstream oss;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) oss << ' ';
            oss << argv[i];
        }

        // Split oss.str() on ';' and push each trimmed command
        std::string argStr = oss.str();
        std::stringstream ss(argStr);
        std::string command;

        while (std::getline(ss, command, ';')) {
            // Trim whitespace from each command
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);
            if (!command.empty()) {
                globalSpace.tasks.script.taskList.push_back(command);
            }
        }
    }
    else{
        /**
         * If no addition arguments were provided:
         *
         * @note For now, an empty Renderer is initiated via set-fps 60
         * 
         * @todo Later on it might be helpful to insert a task like:
         *       `env-load ./Resources/Levels/main.jsonc`
         *       Which represents the menue screen of the game.
         *       Or, for a more scripted task:
         *       `task TaskFiles/main.nebs`.
         *       Making sure that any state currently loaded is cleared.
         *       Having main be a state itself is also an idea, 
         *       but this might become challenging as the user could accidentally overwrite the main state.
         *       Best solution is therefore an env-load, with the environment architecture yet to be defined
         *       best solution is probably:
         * 
         *       - a field with the container 
         * 
         *       - a vector which contains tasks to be executed on environment load
         * 
         *       - potentially an extra task vector for tasks that are executed BEFORE the env is loaded
         * 
         *       - potentially an extra task vector for tasks that are executed BEFORE the env is de-loaded
         * 
         *       Keys like: after-load, after-deload, before-load, before-deload
         *       For easier usage, hardcoding the env-load task is not a good idea, 
         *       instead call some function like "entrypoint" or "main" which is defined in a GlobalSpace DomainModule
         *       This is important, as it's now clear what the entrypoint is, without knowing exactly what main file is loaded
         *       If a user ever defines addition arguments via, e.g. Steam when launching the game, this might become a problem
         *       as any additional argument would make the entrypoint not be called.
         *       So later on, we might consider always calling entrypoint as first task AFTER the command line arguments are parsed
         *       This is necessary, as the user might define important configurations like --headless, which would not be set if the renderer is initialized before them.
         *    
         */
        globalSpace.tasks.script.taskList.push_back(std::string("set-fps 60"));
    }
    
    //------------------------------------------
    // Render loop
    bool queueParsed = false;   // Indicates if the task queue has been parsed on this frame render
    bool criticalStop = false;  // Indicates if a critical stop has occurred
    bool continueLoop = true;   // Determines if we continue the loop
    Nebulite::Constants::ERROR_TYPE lastCriticalResult = Nebulite::Constants::ERROR_TYPE::NONE;
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
         *       e.G. inside the FuncTree, checking state of Renderer might be useful
         */
        if(!queueParsed){
            lastCriticalResult = globalSpace.parseQueue();
            criticalStop = (lastCriticalResult != Nebulite::Constants::ERROR_TYPE::NONE);
            queueParsed = true;
        }

        //------------------------------------------
        // Update and render, only if initialized
        // If renderer wasnt initialized, it is still a nullptr
        if (!criticalStop && globalSpace.RendererExists() && globalSpace.getRenderer()->timeToRender()) {
            globalSpace.update();
            globalSpace.getRenderer()->tick();

            // Reduce script wait counter if not in console mode
            if(!globalSpace.getRenderer()->isConsoleMode()){
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
        std::cerr << "Critical Error: " << globalSpace.errorTable.getErrorDescription(lastCriticalResult) << std::endl;
    }

    // Parser handles if error files need to be closed
    globalSpace.parseStr(binaryName + " errorlog off");

    // Return 1 on critical stop, 0 otherwise
    return (int)criticalStop;
}