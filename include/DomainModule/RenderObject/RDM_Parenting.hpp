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
class Parenting : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::RenderObject> {
public:
    /**
     * @brief Overridden update function.
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
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Parenting(std::string moduleName, Nebulite::Core::RenderObject* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
        bindFunction(&Parenting::addChildren, "add-children", "Adds children of the RenderObject by name");
        bindFunction(&Parenting::removeChildren, "remove-children", "Removes children from the RenderObject");
        bindFunction(&Parenting::removeAllChildren, "remove-all-children", "Removes all children from the RenderObject");
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite