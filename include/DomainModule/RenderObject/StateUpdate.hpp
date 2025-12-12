/**
 * @file StateUpdate.hpp
 * @brief Contains the RenderObject DomainModule for updating the state of render objects.
 */

#ifndef NEBULITE_RODM_STATEUPDATE_HPP
#define NEBULITE_RODM_STATEUPDATE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
}

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::StateUpdate
 * @brief State update DomainModule of the RenderObject tree.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, StateUpdate) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error func_delete(int argc, char** argv);
    static std::string const func_delete_name;
    static std::string const func_delete_desc;

    Constants::Error updateText(int argc, char** argv);
    static std::string const updateText_name;
    static std::string const updateText_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, StateUpdate) {
        bindFunction(&StateUpdate::func_delete, func_delete_name, &func_delete_desc);
        bindFunction(&StateUpdate::updateText, updateText_name, &updateText_desc);

        // TODO: Move to separate Invoke module
        /*
        bindCategory("invoke", "Functions for managing invokes");
        bindFunction(&StateUpdate::invoke_reload,       "reload-invokes",       "Reload all invokes");
        bindFunction(&StateUpdate::invoke_add,          "add-invoke",           "Add an invoke from a file");
        bindFunction(&StateUpdate::invoke_remove,       "remove-invoke",        "Remove an invoke by name");
        bindFunction(&StateUpdate::invoke_purge,        "remove-all-invokes",   "Remove all invokes");
        */
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_STATEUPDATE_HPP