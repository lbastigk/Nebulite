/**
 * @file RDM_Debug.hpp
 * 
 * Debug functions for the the domain RenderObject.
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
 * @class Nebulite::DomainModule::RenderObject::Debug
 * @brief Debug management for the RenderObject tree DomainModule.
 */
class Debug : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::RenderObject> {
public:
    /**
     * @brief Overridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Prints the source rectangle of the spritesheet to console
     * 
     * @param argc The argument count
     * @param argv The argument vector: None
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error printSrcRect(int argc, char* argv[]);

    /**
     * @brief Prints the destination rectangle of the spritesheet to console
     * 
     * @param argc The argument count
     * @param argv The argument vector: None
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error printDstRect(int argc, char* argv[]);

    /**
     * @brief Prints the document to cout
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <file>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error print(int argc, char* argv[]);

    /**
     * @brief Prints a value to cout
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <file>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error printValue(int argc, char* argv[]);

    /**
     * @brief Prints the texture status to cout
     */
    Nebulite::Constants::Error textureStatus(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Debug(std::string moduleName, Nebulite::Core::RenderObject* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
        bindSubtree("debug", "Debugging functions for RenderObject");
        bindFunction(&Debug::printSrcRect,  "debug print-src-rect",     "Prints the source rectangle of the spritesheet to console");
        bindFunction(&Debug::printDstRect,  "debug print-dst-rect",     "Prints the destination rectangle of the spritesheet to console");

        bindFunction(&Debug::print,         "debug print",              "Prints the document to the console");
        bindFunction(&Debug::printValue,    "debug print-value",        "Prints a specific value: <key>");

        bindFunction(&Debug::textureStatus, "debug texture-status",     "Prints texture status to the console");


    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite