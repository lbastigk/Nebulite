/**
 * @file RDM_Parenting.hpp
 * @brief Header file for the Parenting DomainModule of the RenderObject tree.
 * 
 * @note WORK IN PROGRESS, this DomainModule is in the ideas stage.
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
 * @class Nebulite::DomainModule::RenderObject::Parenting
 * @brief Parenting DomainModule of the RenderObject tree.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Parenting) {
public:
    /**
     * @brief Override of update.
     */
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
    Nebulite::Constants::Error addChildren(int argc, char* argv[]);

    /**
     * @brief Removes a child from the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error removeChildren(int argc, char* argv[]);

    /**
     * @brief Removes all children from the RenderObject
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error removeAllChildren(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Parenting) {
        bindSubtree("parenting", "Functions to manage RenderObject parenting");
        bindFunction(&Parenting::addChildren,       "parenting add-children",           "Adds children of the RenderObject by name");
        bindFunction(&Parenting::removeChildren,    "parenting remove-children",        "Removes children from the RenderObject");
        bindFunction(&Parenting::removeAllChildren, "parenting remove-all-children",    "Removes all children from the RenderObject");
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite