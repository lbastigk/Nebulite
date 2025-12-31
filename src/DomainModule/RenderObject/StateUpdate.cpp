#include "DomainModule/RenderObject/StateUpdate.hpp"
#include "Core/RenderObject.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Update
Constants::Error StateUpdate::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Constants::Error StateUpdate::func_delete(int argc, char** argv) {
    domain->flag.deleteFromScene = true;
    return Constants::ErrorTable::NONE();
}

Constants::Error StateUpdate::updateText(int argc, char** argv) {
    domain->flag.calculateText = true;
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject
