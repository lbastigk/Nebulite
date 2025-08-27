#pragma once

/*
================================================================================
                      COMMUNICATION DomainModule - IDEAS PHASE
================================================================================

⚠️  WARNING: This DomainModule is currently in the IDEAS PHASE and NOT IMPLEMENTED

This header file contains the proposed interface for the Nebulite Communication 
DomainModule system. None of the functions declared here have been implemented yet.

The Communication DomainModule is designed to provide minimal inter-process 
communication capabilities for the Nebulite engine:
- Basic connection management
- Status monitoring
- Command parsing and execution
- Image transfer functionality
- Console output redirection

Key Communication Features (Planned):
- Connection: Establish communication with external programs
- Status: Monitor connection health and state
- Commands: Parse and execute incoming commands
- Images: Send screenshots/images to external programs
- Output: Redirect engine console output to connected clients

Protocol Options (To Be Decided):
- ZeroMQ: High-performance messaging, great for strings + binary data
- Named Pipes: Simple OS-native communication for local programs
- TCP Sockets: Custom protocol with network capability

Use Cases:
- Python script control of engine instances
- Image/screenshot transfer to external tools
- Console output monitoring
- Simple command execution

Implementation Status: PLANNING PHASE ONLY
- Header declarations: ✓ Complete
- Function implementations: ❌ Not started
- Protocol selection: ❌ Not decided
- External library integration: ❌ Not started
- Testing: ❌ Not started

Do NOT attempt to use these functions until implementation is complete.

================================================================================
*/

#include "Constants/ErrorTypes.h"
#include "Interaction/Execution/DomainModuleWrapper.h"

//----------------------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//----------------------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Communication
 * @brief DomainModule for communication between the Nebulite engine and external processes.
 */
class Communication : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Core::GlobalSpace, Communication> {
public:
    using DomainModuleWrapper<Nebulite::Core::GlobalSpace, Communication>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Connection Management

    // Basic connection functions
    Nebulite::Constants::ERROR_TYPE connect(int argc, char* argv[]);          // Establish connection: connect [protocol] [address] [port]
    Nebulite::Constants::ERROR_TYPE disconnect(int argc, char* argv[]);       // Close connection: disconnect
    Nebulite::Constants::ERROR_TYPE reconnect(int argc, char* argv[]);        // Reconnect using last settings: reconnect

    //----------------------------------------
    // Status Monitoring

    // Connection status and health
    Nebulite::Constants::ERROR_TYPE status(int argc, char* argv[]);           // Show connection status: status
    Nebulite::Constants::ERROR_TYPE ping(int argc, char* argv[]);             // Test connection: ping
    Nebulite::Constants::ERROR_TYPE isConnected(int argc, char* argv[]);      // Check if connected: is-connected

    //----------------------------------------
    // Command Parsing and Execution

    // Command handling
    Nebulite::Constants::ERROR_TYPE parseCommand(int argc, char* argv[]);     // Parse incoming command: parse-command <command_string>
    Nebulite::Constants::ERROR_TYPE executeRemote(int argc, char* argv[]);    // Execute command from remote: execute-remote <command>
    Nebulite::Constants::ERROR_TYPE setCommandHandler(int argc, char* argv[]); // Set command callback: set-command-handler <callback_command>

    //----------------------------------------
    // Image Transfer

    // Image sending functionality
    Nebulite::Constants::ERROR_TYPE sendImage(int argc, char* argv[]);        // Send image of current renderer to connected client.

    //----------------------------------------
    // Console Output Redirection

    // Output redirection
    Nebulite::Constants::ERROR_TYPE redirectOutput(int argc, char* argv[]);   // Redirect cout to connection: redirect-output <enable/disable>

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        // Connection management
        bindFunction(&Communication::connect,            "connect",              "Establish connection");
        bindFunction(&Communication::disconnect,         "disconnect",           "Close connection");
        bindFunction(&Communication::reconnect,          "reconnect",            "Reconnect using last settings");

        // Status monitoring
        bindFunction(&Communication::status,             "status",               "Show connection status");
        bindFunction(&Communication::ping,               "ping",                 "Test connection");
        bindFunction(&Communication::isConnected,        "is-connected",         "Check if connected");

        // Command parsing
        bindFunction(&Communication::parseCommand,       "parse-command",        "Parse incoming command");
        bindFunction(&Communication::executeRemote,      "execute-remote",       "Execute command from remote");
        bindFunction(&Communication::setCommandHandler,  "set-command-handler",  "Set command callback");

        // Image transfer
        bindFunction(&Communication::sendImage,          "send-image",           "Send image file");

        // Output redirection
        bindFunction(&Communication::redirectOutput,     "redirect-output",      "Redirect cout to connection <on/off>");
    }
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite
