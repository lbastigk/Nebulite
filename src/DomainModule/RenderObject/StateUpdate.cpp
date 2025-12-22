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

// NOLINTNEXTLINE
Constants::Error StateUpdate::func_delete(int argc, char** argv) {
    domain->flag.deleteFromScene = true;
    return Constants::ErrorTable::NONE();
}

std::string const StateUpdate::func_delete_name = "delete";
std::string const StateUpdate::func_delete_desc = R"(Marks object for deletion

Usage: delete

Marks the object for deletion on the next update cycle.
)";

// NOLINTNEXTLINE
Constants::Error StateUpdate::updateText(int argc, char** argv) {
    domain->flag.calculateText = true;
    return Constants::ErrorTable::NONE();
}

std::string const StateUpdate::updateText_name = "update-text";
std::string const StateUpdate::updateText_desc = R"(Calculate text texture

Usage: update-text

Triggers a recalculation of the text texture.
)";

} // namespace Nebulite::DomainModule::RenderObject