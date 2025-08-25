/**
 * @file RTE_Logging.h
 * @brief Header file for the Logging expansion of the RenderObject tree.
 */

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {

class RenderObject; // Forward declaration of Domain class RenderObject

namespace RenderObjectTreeExpansion {

/**
 * @class Logging
 * @brief Logging expansion of the RenderObject tree.
 * 
 * Contains RenderObject-specific logging functionality.
 */
class Logging : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::RenderObject, Logging> {
public:
    using Wrapper<Nebulite::RenderObject, Logging>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
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
     * @return Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE echo(int argc, char* argv[]);

    /**
     * @brief Logs the RenderObject to a file
     * 
     * @param argc The argument count
     * @param argv The argument vector: [filename]
     * 
     * Logs to `RenderObject_id<id>.log.jsonc` if no filename is provided.
     * 
     * @return Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE log(int argc, char* argv[]);

    /**
     * @brief Logs a value to a given file
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <file>
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::ERROR_TYPE logValue(int argc, char* argv[]);

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&Logging::echo,        "echo",         "Prints the arguments to the console");
        bindFunction(&Logging::log,         "log",          "Logs the RenderObject to a file");
        bindFunction(&Logging::logValue,    "log-value",    "Logs a specific value: <key> <file>");
    }
};
}   // namespace RenderObjectTreeExpansion
}   // namespace Nebulite