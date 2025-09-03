/**
 * @file RDM_Layout.h
 * 
 * Layout management for the RenderObject tree DomainModule.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModuleWrapper.hpp"

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
class Layout : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Core::RenderObject, Layout> {
public:
    using DomainModuleWrapper<Nebulite::Core::RenderObject, Layout>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Aligns text to object dimensions
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE alignText(int argc, char* argv[]);

    /**
     * @brief Creates a box based on text dimensions
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE makeBox(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&Layout::alignText, "align-text", "Aligns text to object dimensions");
        bindFunction(&Layout::makeBox, "make-box", "Creates a box based on text dimensions");
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite