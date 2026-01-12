/**
 * @file Debug.hpp
 * @brief Contains the declaration of the Debug DomainModule for the GlobalSpace domain.
 */

#ifndef NEBULITE_GSDM_DEBUG_HPP
#define NEBULITE_GSDM_DEBUG_HPP

//------------------------------------------
// Includes

// Standard library
#include <fstream>
#include <memory>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"


//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
} // namespace Nebulite::Core


//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Debug
 * @brief DomainModule for debugging capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Debug) {
public:
    Constants::Error update() override;
    void reinit() override {}


    //------------------------------------------
    // Available Functions

    /**
     * @todo: errorlog on causes crash with wine
     *        wine: Unhandled page fault on write access to 0000000000000000 at address 0000000140167A65 (thread 0110), starting debugger...
     */
    Constants::Error errorlog(int argc, char** argv);
    static auto constexpr errorlog_name = "errorlog";
    static auto constexpr errorlog_desc = "Activates or deactivates error logging to a file.\n"
        "Usage: errorlog <on/off>\n"
        "\n"
        "- on:  Activates error logging to 'error.log' in the working directory.\n"
        "- off: Deactivates error logging, reverting to standard error output.\n"
        "Note: Ensure you have write permissions in the working directory when activating error logging.\n";

    Constants::Error clearConsole();
    static auto constexpr clearConsole_name = "clear";
    static auto constexpr clearConsole_desc = "Clears the console screen.\n"
        "Usage: clear\n"
        "\n"
        "Note: This function attempts to clear the console screen using system-specific commands.\n"
        "      It may not work in all environments or IDEs.\n";

    Constants::Error log_global(int argc, char** argv);
    static auto constexpr log_global_name = "log global";
    static auto constexpr log_global_desc = "Logs the global document to a file.\n"
        "Usage: log global [<filenames>...]\n"
        "\n"
        "- <filenames>: Optional. One or more filenames to log the global document to.\n"
        "               If no filenames are provided, defaults to 'global.log.jsonc'.\n";

    Constants::Error log_state(int argc, char** argv);
    static auto constexpr log_state_name = "log state";
    static auto constexpr log_state_desc = "Logs the current state of the renderer to a file.\n"
        "Usage: log state [<filenames>...]\n"
        "\n"
        "- <filenames>: Optional. One or more filenames to log the renderer state to.\n"
        "               If no filenames are provided, defaults to 'state.log.jsonc'.\n";

    Constants::Error crash(int argc, char** argv);
    static auto constexpr crash_name = "crash";
    static auto constexpr crash_desc = "Crashes the program, useful for checking if the testing suite can catch crashes.\n"
        "Usage: crash [<type>]\n"
        "\n"
        "- <type>: Optional. The type of crash to induce. Options are:\n"
        "    - segfault   : Causes a segmentation fault (default)\n"
        "    - abort      : Calls std::abort()\n"
        "    - terminate  : Calls std::terminate()\n"
        "    - throw      : Throws an uncaught exception\n";

    Constants::Error error(int argc, char** argv);
    static auto constexpr error_name = "error";
    static auto constexpr error_desc = "Echoes all arguments as string to the standard error.\n"
        "Usage: error <string...>\n"
        "\n"
        "- <string...>: One or more strings to echo to the standard error.\n";

    Constants::Error warn(int argc, char** argv);
    static auto constexpr warn_name = "warn";
    static auto constexpr warn_desc = "Returns a warning: a custom, noncritical error.\n"
        "Usage: warn <string>\n"
        "\n"
        "- <string>: The warning message.\n";

    Constants::Error critical(int argc, char** argv);
    static auto constexpr critical_name = "critical";
    static auto constexpr critical_desc = "Returns a critical error.\n"
        "Usage: critical <string>\n"
        "\n"
        "- <string>: The critical error message.\n";

    Constants::Error waitForInput(int argc, char** argv);
    static auto constexpr waitForInput_name = "input-wait";
    static auto constexpr waitForInput_desc = "Waits for user input before continuing.\n"
        "Usage: input-wait [prompt]\n"
        "\n"
        "Note: This function pauses execution until the user presses Enter\n";

    Constants::Error standardfileRenderobject();
    static auto constexpr standardfileRenderobject_name = "standardfile renderobject";
    static auto constexpr standardfileRenderobject_desc = "Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.\n"
        "Usage: standardfile renderobject\n"
        "\n"
        "Note: This function creates or overwrites the file 'standard.jsonc' in the './Resources/Renderobjects/' directory.\n";

    //------------------------------------------
    // Category names
    static auto constexpr log_name = "log";
    static auto constexpr log_desc = "Functions for logging various states and documents to files.";

    static auto constexpr standardfile_name = "standardfile";
    static auto constexpr standardfile_desc = "Functions for generating standard files for common resources.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Debug) {
        //------------------------------------------
        // Show platform info
        setupPlatformInfo();

        //------------------------------------------
        // Binding functions to the FuncTree
        BINDFUNCTION(&Debug::errorlog, errorlog_name, errorlog_desc);
        BINDFUNCTION(&Debug::clearConsole, clearConsole_name, clearConsole_desc);
        BINDFUNCTION(&Debug::error, error_name, error_desc);
        BINDFUNCTION(&Debug::crash, crash_name, crash_desc);
        BINDFUNCTION(&Debug::warn, warn_name, warn_desc);
        BINDFUNCTION(&Debug::critical, critical_name, critical_desc);
        BINDFUNCTION(&Debug::waitForInput, waitForInput_name, waitForInput_desc);

        (void)bindCategory(log_name, log_desc);
        BINDFUNCTION(&Debug::log_global, log_global_name, log_global_desc);
        BINDFUNCTION(&Debug::log_state, log_state_name, log_state_desc);

        (void)bindCategory(standardfile_name, standardfile_desc);
        BINDFUNCTION(&Debug::standardfileRenderobject, standardfileRenderobject_name, standardfileRenderobject_desc);
    }

private:
    std::streambuf* originalCerrBuf = nullptr;
    std::unique_ptr<std::ofstream> errorFile;

    // Current status of error logging
    // false : logging to cerr
    // true  : logging to file
    bool errorLogStatus = false;

    /**
     * @brief Sets up platform information in the global document.
     */
    void setupPlatformInfo();
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_DEBUG_HPP
