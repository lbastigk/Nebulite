//------------------------------------------
// Includes

// External
#include <RmlUi/Core/Element.h>
#include <SDL3/SDL_events.h>

// Nebulite
#include "Nebulite/Graphics/RmlUi/Interface.hpp"
#include "Nebulite/Module/Base/RmlUiModule.hpp"
#include "Nebulite/Module/RmlUi/EventBridge.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

EventBridge::EventBridge(Utility::Io::Capture& c, Graphics::RmlUi::Interface& i) : RmlUiModule(c,i) {}

void EventBridge::update() {}

void EventBridge::processRmlUiEvent(SDL_Event const& event, int const keyModifiers, Rml::Element* focusElement){
    // TODO: use RmlUi event listeners instead!
    Attribute::OnEnter::processTrigger(interface, capture, event, keyModifiers, focusElement);
    Attribute::OnClick::processTrigger(interface, capture, event, keyModifiers, focusElement);
}

void EventBridge::OnElementDestroy(Rml::Element* element){
    Attribute::OnDestroy::processTrigger(interface, capture, element);
}

} // namespace Nebulite::Module::RmlUi
