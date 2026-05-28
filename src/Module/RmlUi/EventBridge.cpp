//------------------------------------------
// Includes

// External
#include <RmlUi/Core/Element.h>
#include <SDL3/SDL_events.h>

// Nebulite
#include "Graphics/RmlInterface.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Module/RmlUi/EventBridge.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

EventBridge::EventBridge(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

void EventBridge::update() {}

void EventBridge::processRmlUiEvent(SDL_Event const& event, int const keyModifiers, Rml::Element* focusElement){
    Attribute::OnEnter::processTrigger(interface, capture, event, keyModifiers, focusElement);
    Attribute::OnClick::processTrigger(interface, capture, event, keyModifiers, focusElement);
}

void EventBridge::OnElementDestroy(Rml::Element* element){
    Attribute::OnDestroy::processTrigger(interface, capture, element);
}

} // namespace Nebulite::Module::RmlUi
