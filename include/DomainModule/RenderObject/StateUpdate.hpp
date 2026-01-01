/**
 * @file StateUpdate.hpp
 * @brief Contains the RenderObject DomainModule for updating the state of render objects.
 * @todo If more flags are not added, we may merge them into a General DomainModule.
 *       Later on, the text update function should be moved to the texture domain once that works properly.
 */

#ifndef NEBULITE_RODM_STATE_UPDATE_HPP
#define NEBULITE_RODM_STATE_UPDATE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
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
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error deleteObject();
    static std::string_view constexpr deleteObject_name = "delete";
    static std::string_view constexpr deleteObject_desc = "Marks object for deletion\n"
        "\n"
        "Usage: delete\n"
        "\n"
        "Marks the object for deletion on the next update cycle.\n";

    Constants::Error updateText();
    static std::string_view constexpr updateText_name = "update-text";
    static std::string_view constexpr updateText_desc = "Calculate text texture\n"
        "\n"
        "Usage: update-text\n"
        "\n"
        "Triggers a recalculation of the text texture.\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, StateUpdate) {
        BINDFUNCTION(&StateUpdate::deleteObject, deleteObject_name, deleteObject_desc);
        BINDFUNCTION(&StateUpdate::updateText, updateText_name, updateText_desc);
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_STATE_UPDATE_HPP
