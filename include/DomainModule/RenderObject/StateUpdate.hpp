/**
 * @file StateUpdate.hpp
 * @brief Contains the RenderObject DomainModule for updating the state of render objects.
 * @todo If more flags are not added, we may merge them into a General DomainModule.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDEROBJECT_STATE_UPDATE_HPP
#define NEBULITE_DOMAINMODULE_RENDEROBJECT_STATE_UPDATE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::StateUpdate
 * @brief State update DomainModule of the RenderObject tree.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, StateUpdate) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event deleteObject() const ;
    static auto constexpr deleteObject_name = "delete";
    static auto constexpr deleteObject_desc = "Marks object for deletion\n"
        "\n"
        "Usage: delete\n"
        "\n"
        "Marks the object for deletion on the next update cycle.\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, StateUpdate) {
        BIND_FUNCTION(&StateUpdate::deleteObject, deleteObject_name, deleteObject_desc);
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_DOMAINMODULE_RENDEROBJECT_STATE_UPDATE_HPP
