/*
 * ErrorTypes.h
 * ------------
 * Defines the Nebulite::ERROR_TYPE enumeration, which standardizes error codes
 * returned by core engine functions and task queue processing.
 *
 * Usage:
 *   - Functions such as Nebulite::resolveTaskQueue and other main tree functions
 *     return or report an ERROR_TYPE value to indicate the result of execution.
 *   - Critical errors (negative values) signal unrecoverable states and are used
 *     in main.cpp to determine if the engine should halt execution (see
 *     lastCriticalResult and critical_stop logic).
 *   - Non-critical errors (positive values) represent recoverable or minor issues,
 *     such as argument mismatches or unimplemented features.
 *   - The NONE value (0) indicates successful execution with no errors.
 *
 * Example:
 *   Nebulite::ERROR_TYPE result = Nebulite::resolveTaskQueue(...);
 *   if (result < 0) {
 *       // Handle critical error
 *   }
 *
 * See main.cpp for detailed usage in the main engine loop and error handling.
 */
#pragma once

// TODO: Further categorizing into:
// - "Normal" errors/warnings (positive values)
// - "Critical" errors (negative values)
// - Recoverable errors (perhaps > 1000)
// Recoverable errors are then used for headless rendering while editing files
// So we start a nebulite instance with a script to preview a level/file/similiar
// and add "--recover" to the command line arguments
// This will then not crash the engine, but perhaps just inform the user:
// Hey, there is an error in the file you are editing, but we can continue
// Perhaps even in a way where all critical errors are deemed as recoverable

// Return values from main Tree functions
namespace Nebulite{
enum ERROR_TYPE{
    // Critical Errors first with negative value
    CRITICAL_GENERAL = -1000,
    CRITICAL_CUSTOM_ASSERT,
    CRITICAL_FUNCTION_NOT_IMPLEMENTED,
    CRITICAL_INVALID_FILE,
    CRITICAL_INVALID_ARGC_ARGV_PARSING,
    CRITICAL_FUNCTIONCALL_INVALID,
    // Non-critical errors positive
    NONE = 0,
    CUSTOM_ERROR,               // used for functioncall "error"
    TOO_MANY_ARGS,              // argc > expected
    TOO_FEW_ARGS,               // argc < expected
    UNKNOWN_ARG,                
    FEATURE_NOT_IMPLEMENTED,
    SNAPSHOT_FAILED,            // Used in Renderer::snapshot
    FILE_NOT_FOUND,
};
}
