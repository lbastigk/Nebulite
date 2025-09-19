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
    CRITICAL_SDL_RENDERER_TARGET_FAILED,
    
    // [Texture]
    CRITICAL_TEXTURE_NOT_FOUND,
    CRITICAL_TEXTURE_COPY_FAILED,
    CRITICAL_TEXTURE_COLOR_UNSUPPORTED,
    CRITICAL_TEXTURE_LOCK_FAILED,
    CRITICAL_TEXTURE_QUERY_FAILED,
    CRITICAL_TEXTURE_MODIFICATION_FAILED,

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


//------------------------------------------
// New proposed structure for error handling

// Basic struct for an error
struct ERROR{
private:
    std::string* description;
    enum Type{
        // Perhaps some more distinction is necessary here
        // if not, condense to bool isCritical
        CRITICAL,
        NON_CRITICAL
    } type;
    friend class Table;
public:
    std::string getDescription() const {
        return *description;
    }
    bool isCritical() const {
        return type == CRITICAL;
    }
};

// Error table class for holding all errors
// If all domains have access to globalspace, we can make this a member of globalspace
// and access it via domain->getGlobalSpace()->addError(description);
// as well as the Table instance itself
// and the predefined errors via domain->getGlobalSpace()->PREMADE_ERRORS::...
class Table{
private:
    std::vector<ERROR> errors;
    uint16_t count = 0;
public:
    ERROR addError(const std::string& description) {
        if (count == UINT16_MAX) {
            // Handle error: maximum number of errors reached
            // Exit entirely as this should never happen
            std::exit(EXIT_FAILURE);
        }
        errors.emplace_back(new std::string(description));
        count++;
        return errors.back();
    }
} ERRTABLE;

struct PREMADE_ERRORS{
    struct SDL{
        ERROR CRITICAL_SDL_RENDERER_INIT_FAILED = ERRTABLE.addError("Critical Error: SDL Renderer could not be initialized.");
        ERROR CRITICAL_SDL_RENDERER_TARGET_FAILED = ERRTABLE.addError("Critical Error: SDL Renderer target could not be set.");
    } SDL;

    struct TEXTURE{
        ERROR CRITICAL_TEXTURE_NOT_FOUND = ERRTABLE.addError("Critical Error: Texture not found.");
        ERROR CRITICAL_TEXTURE_COPY_FAILED = ERRTABLE.addError("Critical Error: Texture copy failed.");
        ERROR CRITICAL_TEXTURE_COLOR_UNSUPPORTED = ERRTABLE.addError("Critical Error: Texture color format unsupported.");
        ERROR CRITICAL_TEXTURE_LOCK_FAILED = ERRTABLE.addError("Critical Error: Texture lock failed.");
        ERROR CRITICAL_TEXTURE_QUERY_FAILED = ERRTABLE.addError("Critical Error: Texture query failed.");
        ERROR CRITICAL_TEXTURE_MODIFICATION_FAILED = ERRTABLE.addError("Critical Error: Texture modification failed.");
    } TEXTURE;

    struct AUDIO{
        ERROR CRITICAL_AUDIO_DEVICE_INIT_FAILED = ERRTABLE.addError("Critical Error: Audio device could not be initialized.");
    } AUDIO;

    struct FUNCTIONALL{
        ERROR CRITICAL_FUNCTION_NOT_IMPLEMENTED = ERRTABLE.addError("Requested function not implemented.");
        ERROR CRITICAL_FUNCTIONCALL_INVALID = ERRTABLE.addError("Requested function call is invalid.");
        ERROR CRITICAL_INVALID_ARGC_ARGV_PARSING = ERRTABLE.addError("argc/argv parsing error.");
        ERROR TOO_MANY_ARGS = ERRTABLE.addError("Too Many Arguments in function call");
        ERROR TOO_FEW_ARGS = ERRTABLE.addError("Too Few Arguments in function call");
        ERROR UNKNOWN_ARG = ERRTABLE.addError("Unknown Argument Error");
        ERROR FEATURE_NOT_IMPLEMENTED = ERRTABLE.addError("Requested feature of functioncall is not implemented");
    } FUNCTIONALL;

    struct FILE{
        ERROR FILE_NOT_FOUND = ERRTABLE.addError("Requested file not found");
        ERROR CRITICAL_INVALID_FILE = ERRTABLE.addError("Requested file is invalid.");
    } FILE;

    ERROR CRITICAL_GENERAL = ERRTABLE.addError("General, critical error. It is recommended to NOT use this error type in production.");
    ERROR CRITICAL_CUSTOM_ASSERT = ERRTABLE.addError("A custom assertion failed.");
    
    
} PREMADE_ERRORS;