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
#include <absl/container/flat_hash_map.h>

// Return values from main Tree functions
namespace Nebulite{

// NOTE: Remember to add error type descriptions to the ErrorTable class!
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

class ErrorTable {
public:
    ErrorTable(){
        errorTypeToString[Nebulite::ERROR_TYPE::CRITICAL_GENERAL] = 
            "General, critical error. It is recommended to NOT use this error type in production.";
        errorTypeToString[Nebulite::ERROR_TYPE::CRITICAL_CUSTOM_ASSERT] = 
            "A custom assertion failed.";
        errorTypeToString[Nebulite::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED] = 
            "Requested function not implemented.";
        errorTypeToString[Nebulite::ERROR_TYPE::CRITICAL_INVALID_FILE] = 
            "Requested file is invalid.";
        errorTypeToString[Nebulite::ERROR_TYPE::CRITICAL_INVALID_ARGC_ARGV_PARSING] = 
            "argc/argv parsing error.";
        errorTypeToString[Nebulite::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID] = 
            "Requested function call is invalid.";
        errorTypeToString[Nebulite::ERROR_TYPE::NONE] = 
            "No Error";
        errorTypeToString[Nebulite::ERROR_TYPE::CUSTOM_ERROR] = 
            "Custom Error return value";
        errorTypeToString[Nebulite::ERROR_TYPE::TOO_MANY_ARGS] = 
            "Too Many Arguments in function call";
        errorTypeToString[Nebulite::ERROR_TYPE::TOO_FEW_ARGS] = 
            "Too Few Arguments in function call";
        errorTypeToString[Nebulite::ERROR_TYPE::UNKNOWN_ARG] = 
            "Unknown Argument Error";
        errorTypeToString[Nebulite::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED] = 
            "Requested feature of functioncall is not implemented";
        errorTypeToString[Nebulite::ERROR_TYPE::SNAPSHOT_FAILED] = 
            "Snapshot Failed Error";
        errorTypeToString[Nebulite::ERROR_TYPE::FILE_NOT_FOUND] = 
            "Requested file not found";
    }

    std::string getErrorDescription(Nebulite::ERROR_TYPE errorType) {
        auto it = errorTypeToString.find(errorType);
        if (it != errorTypeToString.end()) {
            return it->second;
        }
        return "Undocumented Error!";
    }

private:
    absl::flat_hash_map<Nebulite::ERROR_TYPE, std::string> errorTypeToString;
};