/**
 * @file Parenting.hpp
 * @brief Header file for the Parenting DomainModule of the RenderObject tree.
 * 
 * @note WORK IN PROGRESS, this DomainModule is in the ideas stage.
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
 * @class Nebulite::DomainModule::RenderObject::Parenting
 * @brief Parenting DomainModule of the RenderObject tree.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Parenting) {
public:
    /**
     * @brief Override of update.
     */
    Nebulite::Constants::Event update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Adds a child to the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occurred on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Event addChildren(int argc, char* argv[]);

    /**
     * @brief Removes a child from the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occurred on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Event removeChildren(int argc, char* argv[]);

    /**
     * @brief Removes all children from the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occurred on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Event removeAllChildren(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Parenting) {
        bindSubtree("parenting", "Functions to manage RenderObject parenting");
        BIND_FUNCTION(&Parenting::addChildren,       "parenting add-children",           "Adds children of the RenderObject by name");
        BIND_FUNCTION(&Parenting::removeChildren,    "parenting remove-children",        "Removes children from the RenderObject");
        BIND_FUNCTION(&Parenting::removeAllChildren, "parenting remove-all-children",    "Removes all children from the RenderObject");
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite