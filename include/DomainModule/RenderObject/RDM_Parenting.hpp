/**
 * @file RDM_Parenting.h
 * @brief Header file for the Parenting DomainModule of the RenderObject tree.
 * 
 * @note WORK IN PROGRESS, this DomainModule is in the ideas stage.
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
 * @class Nebulite::DomainModule::RenderObject::Parenting
 * @brief Parenting DomainModule of the RenderObject tree.
 */
class Parenting : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Core::RenderObject, Parenting> {
public:
    using DomainModuleWrapper<Nebulite::Core::RenderObject, Parenting>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Adds a child to the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE addChildren(int argc, char* argv[]);

    /**
     * @brief Removes a child from the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE removeChildren(int argc, char* argv[]);

    /**
     * @brief Removes all children from the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE removeAllChildren(int argc, char* argv[]);


    //------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&Parenting::addChildren, "add-children", "Adds children of the RenderObject by name");
        bindFunction(&Parenting::removeChildren, "remove-children", "Removes children from the RenderObject");
        bindFunction(&Parenting::removeAllChildren, "remove-all-children", "Removes all children from the RenderObject");
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite