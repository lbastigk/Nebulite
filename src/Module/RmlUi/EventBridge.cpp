//------------------------------------------
// Includes

// Standard library
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

// External
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <SDL3/SDL_events.h>

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

} // namespace Nebulite::Module::RmlUi
