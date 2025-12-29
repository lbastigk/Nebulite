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
    static std::string_view constexpr errorlog_name = "errorlog";
    static std::string_view constexpr errorlog_desc = "Activates or deactivates error logging to a file.\n"
        "Usage: errorlog <on/off>\n"
        "\n"
        "- on:  Activates error logging to 'error.log' in the working directory.\n"
        "- off: Deactivates error logging, reverting to standard error output.\n"
        "Note: Ensure you have write permissions in the working directory when activating error logging.";

    Constants::Error clearConsole(int argc, char** argv);
    static std::string_view constexpr clearConsole_name = "clear";
    static std::string_view constexpr clearConsole_desc = "Clears the console screen.\n"
        "Usage: clear\n"
        "\n"
        "Note: This function attempts to clear the console screen using system-specific commands.\n"
        "      It may not work in all environments or IDEs.";

    Constants::Error log_global(int argc, char** argv);
    static std::string_view constexpr log_global_name = "log global";
    static std::string_view constexpr log_global_desc = "Logs the global document to a file.\n"
        "Usage: log global [<filenames>...]\n"
        "\n"
        "- <filenames>: Optional. One or more filenames to log the global document to.\n"
        "               If no filenames are provided, defaults to 'global.log.jsonc'.";

    Constants::Error log_state(int argc, char** argv);
    static std::string_view constexpr log_state_name = "log state";
    static std::string_view constexpr log_state_desc = "Logs the current state of the renderer to a file.\n"
        "Usage: log state [<filenames>...]\n"
        "\n"
        "- <filenames>: Optional. One or more filenames to log the renderer state to.\n"
        "               If no filenames are provided, defaults to 'state.log.jsonc'.";

    Constants::Error crash(int argc, char** argv);
    static std::string_view constexpr crash_name = "crash";
    static std::string_view constexpr crash_desc = "Crashes the program, useful for checking if the testing suite can catch crashes.\n"
        "Usage: crash [<type>]\n"
        "\n"
        "- <type>: Optional. The type of crash to induce. Options are:\n"
        "    - segfault   : Causes a segmentation fault (default)\n"
        "    - abort      : Calls std::abort()\n"
        "    - terminate  : Calls std::terminate()\n"
        "    - throw      : Throws an uncaught exception";

    Constants::Error error(int argc, char** argv);
    static std::string_view constexpr error_name = "error";
    static std::string_view constexpr error_desc = "Echoes all arguments as string to the standard error.\n"
        "Usage: error <string...>\n"
        "\n"
        "- <string...>: One or more strings to echo to the standard error.";

    Constants::Error warn(int argc, char** argv);
    static std::string_view constexpr warn_name = "warn";
    static std::string_view constexpr warn_desc = "Returns a warning: a custom, noncritical error.\n"
        "Usage: warn <string>\n"
        "\n"
        "- <string>: The warning message.";

    Constants::Error critical(int argc, char** argv);
    static std::string_view constexpr critical_name = "critical";
    static std::string_view constexpr critical_desc = "Returns a critical error.\n"
        "Usage: critical <string>\n"
        "\n"
        "- <string>: The critical error message.";

    Constants::Error waitForInput(int argc, char** argv);
    static std::string_view constexpr waitForInput_name = "input-wait";
    static std::string_view constexpr waitForInput_desc = "Waits for user input before continuing.\n"
        "Usage: input-wait [prompt]\n"
        "\n"
        "Note: This function pauses execution until the user presses Enter";

    Constants::Error standardfile_renderobject(int argc, char** argv);
    static std::string_view constexpr standardfile_renderobject_name = "standardfile renderobject";
    static std::string_view constexpr standardfile_renderobject_desc = "Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.\n"
        "Usage: standardfile renderobject\n"
        "\n"
        "Note: This function creates or overwrites the file 'standard.jsonc' in the './Resources/Renderobjects/' directory.";

    //------------------------------------------
    // Category names
    static std::string_view constexpr log_name = "log";
    static std::string_view constexpr log_desc = "Functions for logging various states and documents to files.";

    static std::string_view constexpr standardfile_name = "standardfile";
    static std::string_view constexpr standardfile_desc = "Functions for generating standard files for common resources.";

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
        bindFunction(&Debug::errorlog, errorlog_name, errorlog_desc);
        bindFunction(&Debug::clearConsole, clearConsole_name, clearConsole_desc);
        bindFunction(&Debug::error, error_name, error_desc);
        bindFunction(&Debug::crash, crash_name, crash_desc);
        bindFunction(&Debug::warn, warn_name, warn_desc);
        bindFunction(&Debug::critical, critical_name, critical_desc);
        bindFunction(&Debug::waitForInput, waitForInput_name, waitForInput_desc);

        (void)bindCategory(log_name, log_desc);
        bindFunction(&Debug::log_global, log_global_name, log_global_desc);
        bindFunction(&Debug::log_state, log_state_name, log_state_desc);

        (void)bindCategory(standardfile_name, standardfile_desc);
        bindFunction(&Debug::standardfile_renderobject, standardfile_renderobject_name, standardfile_renderobject_desc);
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
