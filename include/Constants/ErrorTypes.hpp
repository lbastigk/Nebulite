/**
 * @file ErrorTypes.hpp
 * @brief Defines the Nebulite::Constants::Error for standardized error codes
 * and the Nebulite::Constants::ErrorTable class for mapping error codes to their string descriptions.
 *
 * Functions bound via the FuncTree system utilize a `Nebulite::Constants::Error foo([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])` signature.
 *
 * Usage:
 *   - Functions such as Nebulite::resolveTaskQueue executes main tree functions
 *     which return an Error value to indicate the result of execution.
 *   - Critical errors (negative values) signal unrecoverable states and are used
 *     in main.cpp to determine if the engine should halt execution (see
 *     lastCriticalResult and critical_stop logic).
 *   - Non-critical errors (positive values) represent recoverable or minor issues,
 *     such as argument mismatches or unimplemented features.
 *   - The NONE value (0) indicates successful execution with no errors.
 *
 * Example:
 * ```cpp
 *   Nebulite::Constants::Error result = Nebulite::resolveTaskQueue(...);
 *   if (result.isCritical()) {
 *       // Handle critical error
 *   }
 * ```
 * 
 * Predefined errors are accessed within the ErrorTable struct, e.g.,
 * ```cpp
 * Nebulite::Constants::Error lastCriticalResult = Nebulite::Constants::ErrorTable::NONE;
 * ```
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
 * @class Error
 * @brief Represents an error with a description and type.
 */
class Error{
public:
    /**
     * @enum Type
     * @brief Enumeration for error types.
     */
    enum Type{
        // Perhaps some more distinction is necessary here
        // if not, condense to bool isCritical
        CRITICAL,
        NON_CRITICAL,
        NONE
    };

    /**
     * @brief Comparison operator for Error struct.
     */
    bool operator==(const Error& other) const {
        // Only relevant part is that they have the same description pointer
        return description == other.description;
    }

    /**
     * @brief Inequality operator for Error struct.
     */
    bool operator!=(const Error& other) const {
        return !(*this == other);
    }

    /**
     * @brief Default constructor for ERROR struct.
     */
    Error(std::string* desc, Error::Type t) : description(desc), type(t) {}

    /**
     * @brief Empty Constructor for ERROR struct.
     */
    Error() : description(nullptr), type(NONE) {}

    /**
     * @brief Get the error description.
     */
    std::string getDescription() const {
        return *description;
    }
    bool isCritical() const {
        return type == Error::CRITICAL;
    }
    bool isError() const {
        return type != Error::NONE;
    }

    void print() const {
        if(description && type != NONE) std::cout << *description << std::endl;
    }
    
private:
    std::string* description;
    Type type;
};

/**
 * @class ErrorTable
 * @brief Singleton class that manages a table of errors and their descriptions.
 * 
 * This class provides a centralized way to manage error codes and their corresponding
 * descriptions. It ensures that each error is unique and provides methods to add and
 * retrieve errors.
 * 
 * Usage:
 * 
 *   - Add errors using the static method `addError`.
 * 
 *   - Retrieve predefined errors using the nested structs (e.g., `ErrorTable::SDL::CRITICAL_SDL_RENDERER_INIT_FAILED()`).
 * 
 * @todo Implement short-existing errors that are removed after some time (addError should be private, addShortLivedError public)
 *       However, we need to be careful with dangling pointers in that case.
 *       Perhaps it's best to just keep all errors for the lifetime of the program.
 *       Then, if we have more than UINT16_MAX errors, we can just exit with a message.
 */
class ErrorTable{
private:
    std::vector<Error> errors;

    /**
     * @brief Holds count of errors added.
     * 
     * There isn't necessarily a need to limit the number of errors,
     * but this makes sure that we aren't accidently writing more and more errors without deleting them,
     * preventing memory leaks.
     */
    uint16_t count;
    
    static ErrorTable& getInstance() {
        static ErrorTable instance;
        return instance;
    }

    std::vector<std::string> localDescriptions; // To own the strings 

public:
    ErrorTable() : count(0) {}

    /**
     * @brief This implementation is not recommended, as users might pass str.c_str()
     *        which will be a dangling pointer after the function call.
     */
    //static Error addError(const char* description, Error::Type type = Error::NON_CRITICAL){
    //    return getInstance().addErrorImpl(description, type);
    //}

    /**
     * @brief Adds an error to the error table and returns the corresponding Error object.
     * @param description The error description string. Note that type must be std::string to ensure
     *                    the pointer remains valid.
     * @param type The type of error (CRITICAL or NON_CRITICAL). Default is NON_CRITICAL.
     * @return The corresponding Error object.
     */
    static Error addError(std::string description, Error::Type type = Error::NON_CRITICAL){
        // Check if we already have this error
        for(const auto& err : getInstance().errors){
            if(err.getDescription() == description){
                return err; // Return existing error
            }
        }

        // Make copy of string to store pointer
        getInstance().localDescriptions.push_back(description);
        return getInstance().addErrorImpl(getInstance().localDescriptions.back().c_str(), type);
    }


private:
    Error addErrorImpl(const char* description, Error::Type type = Error::NON_CRITICAL){
        if (count == UINT16_MAX) {
            std::cerr << "ErrorTable has reached its maximum capacity of " << UINT16_MAX << " errors." << std::endl;
            std::cerr << "Make sure that new errors added are removed after some time if they are not needed anymore." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        errors.emplace_back(new std::string(description), type);
        count++;
        return errors.back();
    }

public:
    //------------------------------------------
    // Specific errors

    /**
     * @struct SDL
     * @brief Struct grouping SDL related errors.
     */
    struct SDL{
        static inline Error CRITICAL_SDL_RENDERER_INIT_FAILED(){
            static Error error = getInstance().addError("Critical Error: SDL Renderer could not be initialized.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_SDL_RENDERER_TARGET_FAILED(){
            static Error error = getInstance().addError("Critical Error: SDL Renderer target could not be set.", Error::CRITICAL);
            return error;
        }
    } SDL;

    /**
     * @struct RENDERER
     * @brief Struct grouping Nebulite::Core::Renderer related errors.
     */
    struct RENDERER{
        static inline Error CRITICAL_RENDERER_NOT_INITIALIZED(){
            static Error error = getInstance().addError("Critical Error: Renderer not initialized.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_RENDERER_SNAPSHOT_FAILED(){
            static Error error = getInstance().addError("Critical Error: Renderer snapshot failed.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_INVOKE_NULLPTR(){
            static Error error = getInstance().addError("Critical Error: Linked Invoke pointer is nullptr.", Error::CRITICAL);
            return error;
        }
    } RENDERER;

    /**
     * @struct TEXTURE
     * @brief Struct grouping Texture related errors.
     */
    struct TEXTURE{
        static inline Error CRITICAL_TEXTURE_NOT_FOUND(){
            static Error error = getInstance().addError("Critical Error: Texture not found.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_TEXTURE_COPY_FAILED(){
            static Error error = getInstance().addError("Critical Error: Texture copy failed.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_TEXTURE_COLOR_UNSUPPORTED(){
            static Error error = getInstance().addError("Critical Error: Texture color format unsupported.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_TEXTURE_LOCK_FAILED(){
            static Error error = getInstance().addError("Critical Error: Texture lock failed.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_TEXTURE_QUERY_FAILED(){
            static Error error = getInstance().addError("Critical Error: Texture query failed.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_TEXTURE_MODIFICATION_FAILED(){
            static Error error = getInstance().addError("Critical Error: Texture modification failed.", Error::CRITICAL);
            return error;
        }
    } TEXTURE;

    /**
     * @struct AUDIO
     * @brief Struct grouping Audio related errors.
     */
    struct AUDIO{
        static inline Error CRITICAL_AUDIO_DEVICE_INIT_FAILED(){
            static Error error = getInstance().addError("Critical Error: Audio device could not be initialized.", Error::CRITICAL);
            return error;
        }
    } AUDIO;

    /**
     * @struct RENDERER
     * @brief Struct grouping Renderer related errors.
     */
    struct FUNCTIONAL{
        static inline Error CRITICAL_FUNCTION_NOT_IMPLEMENTED(){
            static Error error = getInstance().addError("Requested function not implemented.", Error::CRITICAL);
            return error;
        }
        static inline Error CRITICAL_FUNCTIONCALL_INVALID(){
            static Error error = getInstance().addError("Requested function call is invalid.", Error::NON_CRITICAL);
            return error;
        }
        static inline Error CRITICAL_INVALID_ARGC_ARGV_PARSING(){
            static Error error = getInstance().addError("argc/argv parsing error.", Error::NON_CRITICAL);
            return error;
        }
        static inline Error TOO_MANY_ARGS(){
            static Error error = getInstance().addError("Too Many Arguments in function call", Error::NON_CRITICAL);
            return error;
        }
        static inline Error TOO_FEW_ARGS(){
            static Error error = getInstance().addError("Too Few Arguments in function call", Error::NON_CRITICAL);
            return error;
        }
        static inline Error UNKNOWN_ARG(){
            static Error error = getInstance().addError("Unknown Argument Error", Error::NON_CRITICAL);
            return error;
        }
        static inline Error FEATURE_NOT_IMPLEMENTED(){
            static Error error = getInstance().addError("Requested feature of functioncall is not implemented", Error::NON_CRITICAL);
            return error;
        }
    } FUNCTIONAL;

    /**
     * @struct FILE
     * @brief Struct grouping File related errors.
     */
    struct FILE{
        static inline Error CRITICAL_INVALID_FILE(){
            static Error error = getInstance().addError("Requested file is invalid.", Error::CRITICAL);
            return error;
        }
    } FILE;

    //------------------------------------------
    // Non-specific errors

    static inline Error CRITICAL_GENERAL(){
        static Error error = getInstance().addError("General, critical error. It is recommended to NOT use this error type in production.", Error::CRITICAL);
        return error;
    }
    static inline Error NONE(){
        static Error error = getInstance().addError("", Error::NONE);
        return error;
    }   
};

} // namespace Constants
} // namespace Nebulite