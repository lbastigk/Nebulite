/**
 * @file StateUpdate.hpp
 * @brief Contains the RenderObject DomainModule for updating the state of render objects.
 * @todo If more flags are not added, we may merge them into a General DomainModule.
 *       Later on, the text update function should be moved to the texture domain once that works properly.
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
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_STATEUPDATE_HPP
