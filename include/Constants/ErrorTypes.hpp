/**
 * @file ErrorTypes.hpp
 * @brief Defines the Nebulite::Constants::ERROR_TYPE enumeration for standardized error codes
 * and the Nebulite::Constants::ErrorTable class for mapping error codes to their string descriptions.
 *
 * Functions bound via the FuncTree system utilize a `Nebulite::Constants::ERROR_TYPE foo(int argc, char** argv)` signature.
 *
 * Usage:
 *   - Functions such as Nebulite::resolveTaskQueue executes main tree functions
 *     which return an ERROR_TYPE value to indicate the result of execution.
 *   - Critical errors (negative values) signal unrecoverable states and are used
 *     in main.cpp to determine if the engine should halt execution (see
 *     lastCriticalResult and critical_stop logic).
 *   - Non-critical errors (positive values) represent recoverable or minor issues,
 *     such as argument mismatches or unimplemented features.
 *   - The NONE value (0) indicates successful execution with no errors.
 *
 * Example:
 *   Nebulite::Constants::ERROR_TYPE result = Nebulite::resolveTaskQueue(...);
 *   if (result < 0) {
 *       // Handle critical error
 *   }
 *
 * See main.cpp for detailed usage in the main engine loop and error handling.
 */

#pragma once

//------------------------------------------
// Includes

// External
#include <absl/container/flat_hash_map.h>   // For error type to string mapping

// Nebulite
#include "Nebulite.hpp"                       // Namespace Documentation

//------------------------------------------
namespace Nebulite{
namespace Constants {

/**
 * @enum Nebulite::Constants::ERROR_TYPE
 * @brief Enumeration for standardized error codes in the Nebulite engine.
 * 
 * @note Remember to add error type descriptions to the ErrorTable class!
 */
enum ERROR_TYPE{
    //------------------------------------------
    // Critical Errors first with negative value
    CRITICAL_GENERAL = -1000,
    //------------------------------------------

    // [Custom]
    CRITICAL_CUSTOM_ASSERT,

    // [Function]
    CRITICAL_FUNCTION_NOT_IMPLEMENTED,
    CRITICAL_FUNCTIONCALL_INVALID,
    CRITICAL_INVALID_ARGC_ARGV_PARSING,

    // [File]
    CRITICAL_INVALID_FILE,

    // [SDL]
    CRITICAL_SDL_RENDERER_INIT_FAILED,
    
    // [Texture]
    CRITICAL_TEXTURE_NOT_FOUND,
    CRITICAL_TEXTURE_COPY_FAILED,
    CRITICAL_TEXTURE_COLOR_UNSUPPORTED,
    CRITICAL_TEXTURE_LOCK_FAILED,

    //------------------------------------------
    // Non-critical errors positive
    NONE = 0,
    //------------------------------------------

    // [Custom]
    CUSTOM_ERROR,               // used for functioncall "error"

    // [File]
    FILE_NOT_FOUND,

    // [Function]
    TOO_MANY_ARGS,              // argc > expected
    TOO_FEW_ARGS,               // argc < expected
    UNKNOWN_ARG,                
    FEATURE_NOT_IMPLEMENTED,

    // [SDL]
    SNAPSHOT_FAILED,            // Used in Nebulite::DomainModule::GlobalSpace::Renderer::snapshot
};



/**
 * @class Nebulite::Constants::ErrorTable
 * @brief Class for mapping Nebulite error types to their string descriptions.
 */
class ErrorTable {
public:

    /**
     * @brief Constructor for ErrorTable.
     * 
     * On construction, all error types are mapped to their string descriptions.
     * Add new values here as needed.
     * 
     * @todo Refactor enum and descriptions to perhaps be directly linked, avoiding duplication.
     *       Perhaps return of type std::pair<int, std::string*>
     *       With -1 for critical errors, 0 for none, +1 for non-critical errors.
     *       This would allow us to use existing errors from the database
     *       as well as custom errors with description.
     */
    ErrorTable(){
        //------------------------------------------
        // Critical Errors
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::CRITICAL_GENERAL] = 
            "General, critical error. It is recommended to NOT use this error type in production.";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::CRITICAL_CUSTOM_ASSERT] = 
            "A custom assertion failed.";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED] = 
            "Requested function not implemented.";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::CRITICAL_INVALID_FILE] = 
            "Requested file is invalid.";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::CRITICAL_INVALID_ARGC_ARGV_PARSING] = 
            "argc/argv parsing error.";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTIONCALL_INVALID] = 
            "Requested function call is invalid.";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::CRITICAL_SDL_RENDERER_INIT_FAILED] = 
            "Critical Error: SDL Renderer could not be initialized.";
        //------------------------------------------
        // Non-critical errors
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::NONE] = 
            "No Error";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::CUSTOM_ERROR] = 
            "Custom Error return value";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS] = 
            "Too Many Arguments in function call";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS] = 
            "Too Few Arguments in function call";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::UNKNOWN_ARG] = 
            "Unknown Argument Error";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::FEATURE_NOT_IMPLEMENTED] = 
            "Requested feature of functioncall is not implemented";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::SNAPSHOT_FAILED] = 
            "Snapshot Failed Error";
        errorTypeToString[Nebulite::Constants::ERROR_TYPE::FILE_NOT_FOUND] = 
            "Requested file not found";
    }

    /**
     * @brief Get the error description for a given error type.
     * 
     * @param errorType The error type to retrieve the description for.
     * @return The string description of the error type. Returns "Undocumented Error!" if no description is found.
     */
    std::string getErrorDescription(Nebulite::Constants::ERROR_TYPE errorType) {
        auto it = errorTypeToString.find(errorType);
        if (it != errorTypeToString.end()) {
            return it->second;
        }
        return "Undocumented Error! ID: " + std::to_string(static_cast<int>(errorType));
    }

private:
    // hashtable for error type to string mapping
    absl::flat_hash_map<Nebulite::Constants::ERROR_TYPE, std::string> errorTypeToString;
};
} // namespace Constants
} // namespace Nebulite