//------------------------------------------
// Includes

// Nebulite
#include "Core/RenderObject.hpp"
#include "Module/Domain/RenderObject/StateUpdate.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::RenderObject {

//------------------------------------------
// Update
Constants::Event StateUpdate::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event StateUpdate::deleteObject() const {
    domain.flag.deleteFromScene = true;
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::RenderObject
