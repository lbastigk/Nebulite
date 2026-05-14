//------------------------------------------
// Includes

// Standard library
#include <optional>
#include <string>
#include <string_view>
#include <utility>

// External
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Graphics/RmlInterface.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Module/RmlUi/Ruleset.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

Ruleset::Ruleset(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

void Ruleset::update() {
    for (auto& toApply : rulesetsToApply) {
        toApply.apply(capture, interface);
    }
    rulesetsToApply.clear();
}

void Ruleset::processRmlUiEvent(SDL_Event const& event, int const keyModifiers, Rml::Element* focusElement){
    Attribute::OnEnter::processEvent(interface, capture, event, keyModifiers, focusElement);
}

void Ruleset::OnElementDestroy(Rml::Element* element){
    if (!element) return;
    if (Attribute::OnDestroy::hasSupportedAttribute(element)) {
        if (std::string const tag = element->GetTagName(); tag == "rml" || tag == "head" || tag == "body") {
            capture.warning.println("Unsupported ruleset invocation position at tag: ", tag, ". Tags rml, head and body are not supported!");
            capture.warning.println("Please place the tag in a separate div inside the body tag.");
            return;
        }

        DeletedElement toAdd;
        if (Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(element)) {
            toAdd.elementIdentifier = Graphics::RmlInterface::RmlElementIdentifier(element);
        }
        toAdd.owner = element->GetOwnerDocument();

        // Add action

        if (auto const* var = element->GetAttribute(Attribute::OnDestroy::ruleset); var) {
            toAdd.actions.rulesetLink = var->Get<Rml::String>();
        }
        if (auto const* val = element->GetAttribute(Attribute::OnDestroy::parse); val) {
            toAdd.actions.stringToParse = val->Get<Rml::String>();
        }
        if (auto const* val = element->GetAttribute(Attribute::OnDestroy::special); val) {
            toAdd.actions.specialAction = parseSpecialAction(val->Get<Rml::String>());
        }

        rulesetsToApply.emplace_back(std::move(toAdd));
    }
}

std::optional<Ruleset::SpecialAction> Ruleset::parseSpecialAction(std::string_view const& str){
    for (auto const& [name, action] : supported) {
        if (str == name) {
            return action;
        }
    }
    return std::nullopt;
}

void Ruleset::DeletedElement::apply(Utility::IO::Capture& capture, Graphics::RmlInterface& interface) const {
    auto ctxAndScope = [&] -> std::optional<Graphics::RmlInterface::ContextAndScope> {
        if (elementIdentifier.has_value()) {
            return interface.getRmlElementContextAndScope(elementIdentifier.value());
        }
        if (owner) {
            return interface.getRmlDocumentContextAndScope(owner);
        }
        return std::nullopt;
    }();
    if (!ctxAndScope) {
        // For some reason, rulesets are executed twice if we delete a domain with an open document. This catches the second call.
        //if (rulesetLink) capture.warning.println("Owner context was deleted, cannot apply ruleset: ", rulesetLink.value());
        //if (stringToParse) capture.warning.println("Owner context was deleted, cannot parse string: ", stringToParse.value());
        return;
    }

    // 1.) rulesetAttributeOnDestroy
    Actions::applyRuleset(actions.rulesetLink, capture, ctxAndScope.value());

    // 2.) parseOnDestroy
    Actions::parseString(actions.stringToParse, capture, ctxAndScope.value());

    // 3.) Special action
    Actions::applySpecialAction(actions.specialAction, interface, nullptr, owner);
}

void Ruleset::Attribute::OnEnter::processEvent(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, SDL_Event const& event, int /*keyModifiers*/, Rml::Element* focusElement){
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
    if (auto const* val = focusElement->GetAttribute(ruleset)) {
        if (ctxAndScope) {
            Actions::applyRuleset(val->Get<Rml::String>(), capture, ctxAndScope.value());
        }
        else {
            capture.warning.println("Element context could not be determined! Cannot apply ruleset: ", val->Get<Rml::String>());
        }
    }
    if (auto const* val = focusElement->GetAttribute(parse)) {
        if (ctxAndScope) {
            Actions::parseString(val->Get<Rml::String>(), capture, ctxAndScope.value());
        }
        else {
            capture.warning.println("Element context could not be determined! Cannot parse string: ", val->Get<Rml::String>());
        }
    }
    if (auto const* val = focusElement->GetAttribute(special)) {
        auto const action = parseSpecialAction(val->Get<Rml::String>());
        Actions::applySpecialAction(action, manager, focusElement, focusElement->GetOwnerDocument());
    }
}

void Ruleset::Actions::applyRuleset(std::optional<std::string> const& rulesetLink, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope) {
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

void Ruleset::Actions::parseString(std::optional<std::string> const& stringToParse, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope) {
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

void Ruleset::Actions::applySpecialAction(std::optional<SpecialAction> const& action, Graphics::RmlInterface& manager, Rml::Element* element, Rml::ElementDocument* document) {
    if (!action) return;
    switch (action.value()) {
    case SpecialAction::blurElement:
        if (element) element->Blur();
        break;
    case SpecialAction::deleteDocument:
        if (document) manager.removeDocument(document);
        break;
    default:
        std::unreachable();
    }
}

} // namespace Nebulite::Module::RmlUi
