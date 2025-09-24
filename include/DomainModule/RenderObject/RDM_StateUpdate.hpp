/**
 * @file RDM_StateUpdate.hpp
 * 
 * Contains the RenderObject DomainModule for updating the state of render objects.
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
 * @class Nebulite::DomainModule::RenderObject::StateUpdate
 * @brief State update DomainModule of the RenderObject tree.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, StateUpdate) {
public:
    /**
     * @brief Overwridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Marks the object for deletion.
     * 
     * @param argc The argument count
     * @param argv The argument vector: 
     */
    Nebulite::Constants::Error deleteObject(int argc, char* argv[]);

    /**
     * @brief Updates the text texture of the object.
     */
    Nebulite::Constants::Error updateText(int argc, char* argv[]);

    /**
     * @brief Reloads all invokes for the object.
     */
    Nebulite::Constants::Error reloadInvokes(int argc, char* argv[]);

    /**
     * @brief Adds an invoke for the object.
     */
    Nebulite::Constants::Error addInvoke(int argc, char* argv[]);
    Nebulite::Constants::Error removeInvoke(int argc, char* argv[]);
    Nebulite::Constants::Error removeAllInvokes(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, StateUpdate) {
        bindFunction(&StateUpdate::deleteObject,        "delete",               "Marks object for deletion");
        bindFunction(&StateUpdate::updateText,          "update-text",          "Calculate text texture");
        bindFunction(&StateUpdate::reloadInvokes,       "reload-invokes",       "Reload all invokes");
        bindFunction(&StateUpdate::addInvoke,           "add-invoke",           "Add an invoke from a file");
        bindFunction(&StateUpdate::removeInvoke,        "remove-invoke",        "Remove an invoke by name");
        bindFunction(&StateUpdate::removeAllInvokes,    "remove-all-invokes",   "Remove all invokes");
    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite