/**
 * @file Debug.hpp
 * @brief Contains the declaration of the Debug DomainModule for the GlobalSpace domain.
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBALSPACE_DEBUG_HPP
#define NEBULITE_DOMAINMODULE_GLOBALSPACE_DEBUG_HPP

//------------------------------------------
// Includes

// Standard library
#include <fstream>
#include <memory>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"


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
     * @todo: errorLog on causes crash with wine
     *        wine: Unhandled page fault on write access to 0000000000000000 at address 0000000140167A65 (thread 0110), starting debugger...
     */
    Constants::Error errorLog(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr errorLog_name = "error-log";
    static auto constexpr errorLog_desc = "Activates or deactivates error logging to a file.\n"
        "Usage: error-log <on/off>\n"
        "\n"
        "- on:  Activates error logging to 'error.log' in the working directory.\n"
        "- off: Deactivates error logging, reverting to standard error output.\n"
        "Note: Ensure you have write permissions in the working directory when activating error logging.\n";

    // TODO: offer a per-domain clear option in Nebulite::DomainModule::Common::Debug. This clears the global capture.
    //       perhaps naming them clear-all and clear respectively?
    static Constants::Error clearConsole(std::span<std::string const> const& args);
    static auto constexpr clearConsole_name = "clear";
    static auto constexpr clearConsole_desc = "Clears the console screen.\n"
        "Usage: clear\n"
        "\n"
        "Note: This function attempts to clear the console screen using system-specific commands.\n"
        "      It may not work in all environments or IDEs.\n";

    Constants::Error log_global(int argc, char** argv) const ;
    static auto constexpr log_global_name = "log global";
    static auto constexpr log_global_desc = "Logs the global document to a file.\n"
        "Usage: log global [<filenames>...]\n"
        "\n"
        "- <filenames>: Optional. One or more filenames to log the global document to.\n"
        "               If no filenames are provided, defaults to 'global.log.jsonc'.\n";

    Constants::Error log_state(int argc, char** argv) const ;
    static auto constexpr log_state_name = "log state";
    static auto constexpr log_state_desc = "Logs the current state of the renderer to a file.\n"
        "Usage: log state [<filenames>...]\n"
        "\n"
        "- <filenames>: Optional. One or more filenames to log the renderer state to.\n"
        "               If no filenames are provided, defaults to 'state.log.jsonc'.\n";

    static Constants::Error crash(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr crash_name = "crash";
    static auto constexpr crash_desc = "Crashes the program, useful for checking if the testing suite can catch crashes.\n"
        "Usage: crash [<type>]\n"
        "\n"
        "- <type>: Optional. The type of crash to induce. Options are:\n"
        "    - segfault   : Causes a segmentation fault (default)\n"
        "    - abort      : Calls std::abort()\n"
        "    - terminate  : Calls std::terminate()\n"
        "    - throw      : Throws an uncaught exception\n";

    static Constants::Error waitForInput(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr waitForInput_name = "input-wait";
    static auto constexpr waitForInput_desc = "Waits for user input before continuing.\n"
        "Usage: input-wait [prompt]\n"
        "\n"
        "Note: This function pauses execution until the user presses Enter\n";

    Constants::Error standardFileRenderObject(std::span<std::string const> const& args) const ;
    static auto constexpr standardFileRenderObject_name = "standard-file render-object";
    static auto constexpr standardFileRenderObject_desc = "Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.\n"
        "Usage: standard-file render-object\n"
        "\n"
        "Note: This function creates or overwrites the file 'standard.jsonc' in the './Resources/Renderobjects/' directory.\n";

    static Constants::Error listExpressionFunctions(std::span<std::string const> const& args);
    static auto constexpr listExpressionFunctions_name = "expression-help";
    static auto constexpr listExpressionFunctions_desc = "Lists all available expression functions with their descriptions.\n"
        "Usage: expression-help\n"
        "\n"
        "Note: This function provides a comprehensive list of all functions that can be used within expressions, along with their usage and descriptions.\n";

    //------------------------------------------
    // Category names
    static auto constexpr log_name = "log";
    static auto constexpr log_desc = "Functions for logging various states and documents to files.";

    static auto constexpr standardFile_name = "standard-file";
    static auto constexpr standardFile_desc = "Functions for generating standard files for common resources.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Debug) {
        //------------------------------------------
        // Setup key information in the global document
        setupPlatformInfo();
        setupDebugInfo();

        //------------------------------------------
        // Binding functions to the FuncTree
        BIND_FUNCTION(&Debug::errorLog, errorLog_name, errorLog_desc);
        BIND_FUNCTION(&Debug::clearConsole, clearConsole_name, clearConsole_desc);
        BIND_FUNCTION(&Debug::crash, crash_name, crash_desc);
        BIND_FUNCTION(&Debug::waitForInput, waitForInput_name, waitForInput_desc);
        BIND_FUNCTION(&Debug::listExpressionFunctions, listExpressionFunctions_name, listExpressionFunctions_desc);

        bindCategory(log_name, log_desc);
        BIND_FUNCTION(&Debug::log_global, log_global_name, log_global_desc);
        BIND_FUNCTION(&Debug::log_state, log_state_name, log_state_desc);

        bindCategory(standardFile_name, standardFile_desc);
        BIND_FUNCTION(&Debug::standardFileRenderObject, standardFileRenderObject_name, standardFileRenderObject_desc);

        // Add routines
        initRoutines();
    }

    struct Key : Data::KeyGroup<"debug."> {
        static auto constexpr platform = makeScoped("platform");
        static auto constexpr buildType = makeScoped("buildType");
    };

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
    void setupPlatformInfo() const ;

    /**
     * @brief Sets up debug information in the global document.
     */
    void setupDebugInfo() const ;

    /**
     * @brief List of timed routines for performance monitoring and debugging purposes.
     */
    std::vector<Utility::Coordination::TimedRoutine> routines;

    void initRoutines();
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_DEBUG_HPP
