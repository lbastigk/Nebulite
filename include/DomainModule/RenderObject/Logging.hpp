/**
 * @file Logging.hpp
 * @brief Header file for the Logging DomainModule of the RenderObject tree.
 */

#ifndef NEBULITE_RODM_LOGGING_HPP
#define NEBULITE_RODM_LOGGING_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
}

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::Logging
 * @brief Logging DomainModule of the RenderObject Domain.
 * 
 * Contains RenderObject-specific logging functionality.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Logging) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @brief Echoes the provided arguments to cout
     * 
     * Note that this function is primarily for debugging purposes, and should not be used in production code.
     * The output may be mangled due to threading issues.
     * Try to limit the amount of RenderObjects that echo at once, to avoid thread contention.
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occurred on command execution
     */
    Constants::Error echo(int argc, char** argv);
    static std::string const echo_name;
    static std::string const echo_desc;

    /**
     * @brief
     */

    /**
     * @brief Logs the entire RenderObject to a file
     * 
     * @param argc The argument count
     * @param argv The argument vector: [filename]
     * 
     * Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.
     * 
     * @return Potential errors that occurred on command execution
     */
    Constants::Error log_all(int argc, char** argv);
    static std::string const log_all_name;
    static std::string const log_all_desc;

    /**
     * @brief Logs a value to a given file
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> [file]
     * 
     * Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.
     * 
     * @return Potential errors that occurred on command execution
     */
    Constants::Error log_key(int argc, char** argv);
    static std::string const log_key_name;
    static std::string const log_key_desc;

    //------------------------------------------
    // Category names
    static std::string const log_name;
    static std::string const log_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Logging) {
        bindFunction(&Logging::echo, echo_name, echo_desc);

        (void)bindCategory(log_name, log_desc);
        bindFunction(&Logging::log_all, log_all_name, log_all_desc);
        bindFunction(&Logging::log_key, log_key_name, log_key_desc);
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_LOGGING_HPP
