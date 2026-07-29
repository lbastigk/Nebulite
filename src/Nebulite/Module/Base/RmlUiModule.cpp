//------------------------------------------
// Includes

// External
#include <RmlUi/Core/Element.h>
#include <SDL3/SDL_events.h>

// Nebulite
#include "Nebulite/Graphics/RmlInterface.hpp"
#include "Nebulite/Module/Base/RmlUiModule.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::Base {

RmlUiModule::RmlUiModule(Utility::IO::Capture& c, Graphics::RmlInterface& i) :
    capture(c),
    interface(i)
{}

void RmlUiModule::update() {}

void RmlUiModule::postRenderUpdate() {}

void RmlUiModule::processRmlUiEvent(SDL_Event const& event, int const keyModifiers, Rml::Element* focusElement) {
    (void) event;
    (void) keyModifiers;
    (void) focusElement;
}

} // namespace Nebulite::Module::Base
