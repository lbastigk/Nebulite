/**
 * @file RDM_StateUpdate.h
 * 
 * Contains the RenderObjectTreeDomainModule for updating the state of render objects.
 */

#pragma once

#include "Constants/ErrorTypes.h"
#include "Interaction/Execution/DomainModuleWrapper.h"

//----------------------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class RenderObject; // Forward declaration of domain class RenderObject
    }
}

//----------------------------------------------------------
namespace Nebulite{
namespace DomainModule{
namespace RenderObject{
/**
 * @class Nebulite::DomainModule::RenderObject::StateUpdate
 * @brief State update DomainModule of the RenderObject tree.
 */
class StateUpdate : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Core::RenderObject, StateUpdate> {
public:
    using DomainModuleWrapper<Nebulite::Core::RenderObject, StateUpdate>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    /**
     * @brief Marks the object for deletion.
     * 
     * @param argc The argument count
     * @param argv The argument vector: 
     */
    Nebulite::Constants::ERROR_TYPE deleteObject(int argc, char* argv[]);

    /**
     * @brief Updates the text texture of the object.
     */
    Nebulite::Constants::ERROR_TYPE updateText(int argc, char* argv[]);

    /**
     * @brief Reloads all invokes for the object.
     */
    Nebulite::Constants::ERROR_TYPE reloadInvokes(int argc, char* argv[]);

    /**
     * @brief Adds an invoke for the object.
     */
    Nebulite::Constants::ERROR_TYPE addInvoke(int argc, char* argv[]);
    Nebulite::Constants::ERROR_TYPE removeInvoke(int argc, char* argv[]);
    Nebulite::Constants::ERROR_TYPE removeAllInvokes(int argc, char* argv[]);

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
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