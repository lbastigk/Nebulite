/**
 * @file RDM_Logging.hpp
 * @brief Header file for the Logging DomainModule of the RenderObject tree.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class RenderObject; // Forward declaration of domain class RenderObject
    }
}

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
namespace RenderObject{
/**
 * @class Nebulite::DomainModule::RenderObject::Logging
 * @brief Logging DomainModule of the RenderObject tree.
 * 
 * Contains RenderObject-specific logging functionality.
 */
class Logging : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::RenderObject> {
public:
    /**
     * @brief Overridden update function.
     */
    void update();

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
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE echo(int argc, char* argv[]);

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
    Nebulite::Constants::ERROR_TYPE log(int argc, char* argv[]);

    /**
     * @brief Logs a value to a given file
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <file>
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE logValue(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Logging(Nebulite::Core::RenderObject* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(domain, funcTreePtr) {
        bindFunction(&Logging::echo,        "echo",         "Prints the arguments to the console");
        bindFunction(&Logging::log,         "log",          "Logs the RenderObject to a file");
        bindFunction(&Logging::logValue,    "log-value",    "Logs a specific value: <key> <file>");
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite