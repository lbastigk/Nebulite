//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/General.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::GlobalSpace {

//------------------------------------------
// Update

Constants::Event General::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event General::exit() const {
    // Clear all task queues to prevent further execution
    domain.tasks.clearAllTaskQueues();

    // Set the renderer to quit
    domain.quitRenderer();
    return Constants::Event::success;
}


} // namespace Nebulite::Module::Domain::GlobalSpace
