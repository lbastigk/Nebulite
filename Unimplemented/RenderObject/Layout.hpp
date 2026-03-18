/**
 * @file_Layout.hpp
 * 
 * Layout management for the RenderObject tree DomainModule.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/StandardCapture.hpp"
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
 * @class Nebulite::DomainModule::RenderObject::Layout
 * @brief Layout management for the RenderObject tree DomainModule.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Layout) {
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Event update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Aligns text to object dimensions
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occurred on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Event alignText(int argc, char* argv[]);

    /**
     * @brief Creates a box based on text dimensions
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occurred on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Event makeBox(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Layout) {
        BIND_FUNCTION(&Layout::alignText, "align-text", "Aligns text to object dimensions");
        BIND_FUNCTION(&Layout::makeBox, "make-box", "Creates a box based on text dimensions");
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite