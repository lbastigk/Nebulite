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

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Communication
 * @brief DomainModule for communication between the Nebulite engine and external processes.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Communication){
public:
    void update();

    //------------------------------------------
    // Connection Management

    // Basic connection functions
    Nebulite::Constants::Error connect(int argc, char* argv[]);          // Establish connection: connect [protocol] [address] [port]
    Nebulite::Constants::Error disconnect(int argc, char* argv[]);       // Close connection: disconnect
    Nebulite::Constants::Error reconnect(int argc, char* argv[]);        // Reconnect using last settings: reconnect

    //------------------------------------------
    // Status Monitoring

    // Connection status and health
    Nebulite::Constants::Error status(int argc, char* argv[]);           // Show connection status: status
    Nebulite::Constants::Error ping(int argc, char* argv[]);             // Test connection: ping
    Nebulite::Constants::Error isConnected(int argc, char* argv[]);      // Check if connected: is-connected

    //------------------------------------------
    // Command Parsing and Execution

    // Command handling
    Nebulite::Constants::Error parseCommand(int argc, char* argv[]);     // Parse incoming command: parse-command <command_string>
    Nebulite::Constants::Error executeRemote(int argc, char* argv[]);    // Execute command from remote: execute-remote <command>
    Nebulite::Constants::Error setCommandHandler(int argc, char* argv[]); // Set command callback: set-command-handler <callback_command>

    //------------------------------------------
    // Image Transfer

    // Image sending functionality
    Nebulite::Constants::Error sendImage(int argc, char* argv[]);        // Send image of current renderer to connected client.

    //------------------------------------------
    // Console Output Redirection

    // Output redirection
    Nebulite::Constants::Error redirectOutput(int argc, char* argv[]);   // Redirect cout to connection: redirect-output <enable/disable>

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Communication){
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
