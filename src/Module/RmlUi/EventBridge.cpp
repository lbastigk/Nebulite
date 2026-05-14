//------------------------------------------
// Includes

// Standard library
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

// External
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Graphics/RmlInterface.hpp"
#include "Interaction/AttributeCommand.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Module/RmlUi/EventBridge.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

EventBridge::EventBridge(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

void EventBridge::update() {}

void EventBridge::processRmlUiEvent(SDL_Event const& event, int const keyModifiers, Rml::Element* focusElement){
    Attribute::OnEnter::processTrigger(interface, capture, event, keyModifiers, focusElement);
}

void EventBridge::OnElementDestroy(Rml::Element* element){
    Attribute::OnDestroy::processTrigger(interface, capture, element);
}

void EventBridge::Attribute::OnDestroy::processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, Rml::Element* element){
    if (!element) return;
    if (hasSupportedAttribute(element)) {
        if (std::string const tag = element->GetTagName(); tag == "rml" || tag == "head" || tag == "body") {
            capture.warning.println("Unsupported EventBridge invocation position at tag: ", tag, ". Tags rml, head and body are not supported!");
            capture.warning.println("Please place the tag in a separate div inside the body tag.");
            return;
        }

        DeletedElement toAdd;
        if (Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(element)) {
            toAdd.elementIdentifier = Graphics::RmlInterface::RmlElementIdentifier(element);
        }
        toAdd.owner = element->GetOwnerDocument();

        // Process
        static_assert(AttributeCommand<"">::specializationCount == 3, "If you added a new Rml attribute command specialization, make sure to add it to the trigger processing!");
        if (auto const* var = element->GetAttribute(ruleset.toString()); var) {
            toAdd.actions.rulesetLink = var->Get<Rml::String>();
        }
        if (auto const* val = element->GetAttribute(parse.toString()); val) {
            toAdd.actions.stringToParse = val->Get<Rml::String>();
        }
        if (auto const* val = element->GetAttribute(special.toString()); val) {
            toAdd.actions.specialAction = Interaction::SpecialAction::get(val->Get<Rml::String>());
        }
        toAdd.apply(manager, capture);
    }
}

void EventBridge::Attribute::OnEnter::processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, SDL_Event const& event, int /*keyModifiers*/, Rml::Element* focusElement){
    if (!focusElement) return;
    if (event.type != SDL_EVENT_KEY_DOWN) return;
    if (event.key.scancode != SDL_SCANCODE_RETURN && event.key.scancode != SDL_SCANCODE_KP_ENTER) return;

    auto ctxAndScope = [&] {
        if (Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(focusElement)) {
            if (auto const val = manager.getRmlElementContextAndScope(Graphics::RmlInterface::RmlElementIdentifier(focusElement)); val) return val;
        }
        if (focusElement->GetOwnerDocument()) {
            return manager.getRmlDocumentContextAndScope(focusElement->GetOwnerDocument());
        }
        return std::optional<Graphics::RmlInterface::ContextAndScope>{};
    }();
    // Process
    static_assert(AttributeCommand<"">::specializationCount == 3, "If you added a new Rml attribute command specialization, make sure to add it to the trigger processing!");
    if (auto const* val = focusElement->GetAttribute(ruleset.toString())) {
        if (ctxAndScope) {
            Actions::applyRuleset(val->Get<Rml::String>(), capture, ctxAndScope.value());
        }
        else {
            capture.warning.println("Element context could not be determined! Cannot apply ruleset: ", val->Get<Rml::String>());
        }
    }
    if (auto const* val = focusElement->GetAttribute(parse.toString())) {
        if (ctxAndScope) {
            Actions::parseString(val->Get<Rml::String>(), capture, ctxAndScope.value());
        }
        else {
            capture.warning.println("Element context could not be determined! Cannot parse string: ", val->Get<Rml::String>());
        }
    }
    if (auto const* val = focusElement->GetAttribute(special.toString())) {
        auto const action = Interaction::SpecialAction::get(val->Get<Rml::String>());
        Actions::applySpecialAction(action, manager, capture, focusElement, focusElement->GetOwnerDocument());
    }
}

void EventBridge::Actions::applyRuleset(std::optional<std::string> const& rulesetLink, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope) {
    if (rulesetLink) {
        auto& [ctx, scope] = ctxAndScope;
        if (auto const ruleset = Interaction::Rules::Construction::RulesetCompiler::parseSingle(rulesetLink.value(), ctx.self); ruleset) {
            ruleset.value()->apply(ctx, scope);
        }
        else {
            cap.warning.println("Could not find ruleset with identifier '", rulesetLink.value(), "'. Skipping ruleset invocation.");
        }
    }
}

void EventBridge::Actions::parseString(std::optional<std::string> const& stringToParse, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope) {
    // TODO use a taskqueue instead? Some refactoring of taskQueue for accepting a long string with ';' is required
    if (stringToParse) {
        auto& [ctx, scope] = ctxAndScope;
        for (auto& task : Utility::StringHandler::split(stringToParse.value(), ';')) {
            std::string_view taskView = task;
            Utility::StringHandler::lStrip(taskView);
            switch (std::string const str = std::string(__FUNCTION__) + " " + taskView; ctx.self.parseStr(str,ctx, scope)) {
            case Constants::Event::Warning:
                cap.warning.println("Parsing string resulted in warning. String: ", str);
                break;
            case Constants::Event::Error:
                cap.error.println("Parsing string resulted in error. String: ", str);
                break;
            case Constants::Event::Success:
                break;
            default:
                std::unreachable();
            }
        }
    }
}

void EventBridge::Actions::applySpecialAction(std::optional<Interaction::SpecialAction::Type> const& action, Graphics::RmlInterface& manager, Utility::IO::Capture& capture, Rml::Element* element, Rml::ElementDocument* document) {
    if (!action) return;
    switch (action.value()) {
    case Interaction::SpecialAction::Type::debugLog:
        capture.warning.println("Rml attribute command debug action triggered: Logging");
        break;
    case Interaction::SpecialAction::Type::debugWarning:
        capture.warning.println("Rml attribute command debug action triggered: Warning");
        break;
    case Interaction::SpecialAction::Type::debugError:
        capture.warning.println("Rml attribute command debug action triggered: Error");
        break;
    case Interaction::SpecialAction::Type::crash:
        throw std::logic_error("Rml attribute command debug action triggered: Crash");
    case Interaction::SpecialAction::Type::blurElement:
        if (element) element->Blur();
        break;
    case Interaction::SpecialAction::Type::deleteDocument:
        if (document) manager.removeDocument(document);
        break;
    default:
        std::unreachable();
    }
}

void EventBridge::DeletedElement::apply(Graphics::RmlInterface& manager, Utility::IO::Capture& capture) const {
    auto ctxAndScope = [&] -> std::optional<Graphics::RmlInterface::ContextAndScope> {
        if (elementIdentifier.has_value()) {
            return manager.getRmlElementContextAndScope(elementIdentifier.value());
        }
        if (owner) {
            return manager.getRmlDocumentContextAndScope(owner);
        }
        return std::nullopt;
    }();
    if (!ctxAndScope) {
        // For some reason, EventBridges are executed twice if we delete a domain with an open document. This catches the second call.
        return;
    }

    Actions::applyRuleset(actions.rulesetLink, capture, ctxAndScope.value());
    Actions::parseString(actions.stringToParse, capture, ctxAndScope.value());
    Actions::applySpecialAction(actions.specialAction, manager, capture, nullptr, owner);
}

} // namespace Nebulite::Module::RmlUi
