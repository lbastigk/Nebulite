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
 * @details Contains RenderObject-specific logging functionality.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Logging) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // Same as GlobalSpace echo, but perhaps useful to quickly check if a RenderObjects ruleset is triggered.
    // Compared to the global echo, this one is not delayed by any taskqueue.
    static Constants::Error echo(std::span<std::string const> const& args);
    static auto constexpr echo_name = "echo";
    static auto constexpr echo_desc = "Echoes all arguments as string to the standard output.\n"
        "\n"
        "Usage: echo <string>\n";

    static Constants::Error log_all(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope);
    static auto constexpr log_all_name = "log all";
    static auto constexpr log_all_desc = "Logs the entire RenderObject to a file.\n"
        "\n"
        "Usage: log [filename]\n"
        "\n"
        "Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.\n";

    static Constants::Error log_key(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope);
    static auto constexpr log_key_name = "log key";
    static auto constexpr log_key_desc = "Logs a specific key's value to a file.\n"
        "\n"
        "Usage: log key <key> [filename]\n"
        "\n"
        "Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.\n";

    //------------------------------------------
    // Category names
    static auto constexpr log_name = "log";
    static auto constexpr log_desc = "Logging utilities";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Logging) {
        BIND_FUNCTION(&Logging::echo, echo_name, echo_desc);

        bindCategory(log_name, log_desc);
        BIND_FUNCTION(&Logging::log_all, log_all_name, log_all_desc);
        BIND_FUNCTION(&Logging::log_key, log_key_name, log_key_desc);
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_LOGGING_HPP
