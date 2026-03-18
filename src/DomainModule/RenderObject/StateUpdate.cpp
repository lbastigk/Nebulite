#include "DomainModule/RenderObject/StateUpdate.hpp"
#include "Core/RenderObject.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Update
Constants::Event StateUpdate::update() {
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

} // namespace Nebulite::DomainModule::RenderObject
