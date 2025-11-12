#include "DomainModule/Unimplemented/GDM_GUI.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

//------------------------------------------
// Update
Nebulite::Constants::Error GUI::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY

    // We can also update internal states from the local level:
    // int displaySizeX = self->global.get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(), 0);
    return Nebulite::Constants::ErrorTable::NONE();
}


Nebulite::Constants::Error GUI::example(int argc, char* argv[]) {
    // Goal is to create an SDL texture with DearImgui and attach it to the renderer queue
    // We might wish to create container variables local to the GUI class to hold DearImgui elements
    // so that we might call update on them, or modify them in any way

    // Creation
    /*...*/

    // Attachment happens once, renderer stores pointer but does not own texture!
    domain->getRenderer()->attachTextureAboveLayer(
        Nebulite::Core::Environment::Layer::UI, 
        "circle_texture", 
        nullptr /* SDL_Texture* created from DearImgui */
    );

    return Nebulite::Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

} // namespace Nebulite::DomainModule::GlobalSpace
