#include "DomainModule/GlobalSpace/GDM_GUI.hpp"
#include "Core/GlobalSpace.hpp"       // Global Space for Nebulite

//------------------------------------------
// Update
void Nebulite::DomainModule::GlobalSpace::GUI::update() {
    // Add FuncTree-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY

    // We can also update internal states from the local level:
    // int displaySizeX = self->global.get<int>(Nebulite::Constants::keyName.renderer.dispResX.c_str(), 0);
}


Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::GlobalSpace::GUI::example(int argc, char* argv[]) {
    // Goal is to create an SDL texture with DearImgui and attach it to the renderer queue
    // We might wish to create container variables local to the GUI class to hold DearImgui elements
    // so that we might call update on them, or modify them in any way

    // Creation
    /*...*/

    // Attachment happens once, renderer stores pointer but does not own texture!
    domain->getRenderer()->attachTextureAboveLayer(
        Nebulite::Core::Environment::Layers::menue, 
        "circle_texture", 
        nullptr /* SDL_Texture* created from DearImgui */
    );

    return Nebulite::Constants::ERROR_TYPE::CRITICAL_FUNCTION_NOT_IMPLEMENTED;
}
