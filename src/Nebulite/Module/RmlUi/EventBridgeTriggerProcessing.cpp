//------------------------------------------
// Includes

// Standard library
#include <string>

// External
#include <RmlUi/Core/Element.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>

// Nebulite
#include "Nebulite/Graphics/RmlInterface.hpp"
#include "Nebulite/Module/RmlUi/EventBridge.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

void EventBridge::Attribute::OnDestroy::processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, Rml::Element* element){
    if (!element) return;
    if (!hasSupportedAttribute(element)) return;
    if (std::string const tag = element->GetTagName(); tag == "rml" || tag == "head" || tag == "body") {
        capture.warning.println("Unsupported EventBridge invocation position at tag: ", tag, ". Tags rml, head and body are not supported!");
        capture.warning.println("Please place the tag in a separate div inside the body tag.");
        return;
    }
    BridgeEntry<OnDestroy> const entry(element);
    entry.apply(manager, capture, element);
}

void EventBridge::Attribute::OnEnter::processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, SDL_Event const& event, int /*keyModifiers*/, Rml::Element* focusElement){
    if (!focusElement) return;
    if (event.type != SDL_EVENT_KEY_DOWN) return;
    if (event.key.scancode != SDL_SCANCODE_RETURN && event.key.scancode != SDL_SCANCODE_KP_ENTER) return;
    BridgeEntry<OnEnter> const entry(focusElement);
    entry.apply(manager, capture, focusElement);
}

void EventBridge::Attribute::OnClick::processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, SDL_Event const& event, int /*keyModifiers*/, Rml::Element* focusElement){
    if (!focusElement) return;
    if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN) return;
    BridgeEntry<OnClick> const entry(focusElement);
    entry.apply(manager, capture, focusElement);
}

} // namespace Nebulite::Module::RmlUi
