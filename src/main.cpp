/**
 * @file main.cpp
 * @brief Main entry point for the NEBULITE engine.
 * @details This file initializes the core systems, parses command-line arguments,
 *          and coordinates execution scenarios such as
 *          loading levels or running performance benchmarks.
 *          Usage:
 *            Nebulite <command> <options>
 *
 *          Example Commands:
 *          ```bash
 *          ./bin/Nebulite spawn <RenderObject.jsonc>      # Load a single RenderObject
 *          ./bin/Nebulite env-load <level.jsonc>          # Load and render a level
 *          ./bin/Nebulite task <task.nebs>                # Run a task file
 *          ```
 */

//------------------------------------------
//     _   ____________  __  ____    ________________
//    / | / / ____/ __ )/ / / / /   /  _/_  __/ ____/
//   /  |/ / __/ / __  / / / / /    / /  / / / __/
//  / /|  / /___/ /_/ / /_/ / /____/ /  / / / /___
// /_/ |_/_____/_____/\____/_____/___/ /_/ /_____/
//

//------------------------------------------
// Includes

// Standard library
#include <exception>
#include <string>

// Nebulite
#include "Nebulite.hpp"
#include "Constants/StandardCapture.hpp"
#include "Module/Domain/GlobalSpace/Debug.hpp"

//------------------------------------------
// Constants

namespace {
struct MainReturnValues {
    static constexpr int success = 0; ///< Return value for successful execution
    static constexpr int criticalError = 1; ///< Return value for execution halted by critical error
    static constexpr int logCloseError = 2; ///< Return value for failure to close error log
    static constexpr int logCloseException = 3; ///< Return value for exception during error log closure
};
} // namespace

//------------------------------------------
// NEBULITE main

/**
 * @brief Main function for the NEBULITE engine.
 * @details Initializes the engine, sets up the global space, and processes command-line arguments.
 */
int main(int const argc, char* argv[]) {
    auto const binaryName = std::string(argv[0]);
    auto& global = Nebulite::Global::instance();
    auto& capture = Nebulite::Global::capture();

    //------------------------------------------
    // Initialize the global space, parse command line arguments
    global.initialize();
    global.parseCommandLineArguments(argc, const_cast<char const**>(argv));

    //------------------------------------------
    // Render loop
    do {
        // At least one loop, to handle command line arguments
        global.notifyEvent(global.update());
    } while (global.shouldContinueLoop());

    //------------------------------------------
    // Exit

    // Destroy renderer
    global.getRenderer().destroy();

    // Parser handles if error files need to be closed
    try {
        // Dummy scope for closing
        Nebulite::Interaction::Context ctx{global, global, global};
        Nebulite::Data::JsonScope dummy;
        Nebulite::Interaction::ContextScope dummyCtxScope{dummy, dummy, dummy};
        if (auto const event = global.parseStr(binaryName + " " + std::string(Nebulite::DomainModule::GlobalSpace::Debug::errorLog_name) + " off", ctx, dummyCtxScope); event != Nebulite::Constants::Event::Success) {
            capture.error.println("Could not close log properly!");
            return MainReturnValues::logCloseError; // Closing log failed without exceptions
        }
    } catch (std::exception const& e) {
        capture.error.println("Exception during error log closure: ", e.what());
        return MainReturnValues::logCloseException; // Return a different error code for log closing failure with exceptions
    }

    // Return 1 on critical stop, 0 otherwise
    if (global.criticalErrorOccurred()) {
        return MainReturnValues::criticalError;
    }
    return MainReturnValues::success;
}
