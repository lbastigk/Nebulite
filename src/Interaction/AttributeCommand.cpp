//------------------------------------------
// Includes

// Standard library
#include <optional>
#include <string>
#include <string_view>
#include <utility>

// External
#include <RmlUi/Core/Element.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Graphics/RmlInterface.hpp"
#include "Interaction/AttributeCommand.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Interaction {

static_assert(AttributeCommand<"">::specializationCount == 3, "If you added a new Rml attribute command specialization, make sure to add it to the Actions struct as well!");

void ActionsImpl::applyRuleset(std::string_view const ruleset, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope) {
    auto& [ctx, scope] = ctxAndScope;
    if (auto const rs = Rules::Construction::RulesetCompiler::parseSingle(ruleset, ctx.self); rs) {
        rs.value()->apply(ctx, scope);
    }
    else {
        cap.warning.println("Could not find ruleset with identifier '", ruleset, "'. Skipping ruleset invocation.");
    }
}

void ActionsImpl::parseString(std::string_view const stringToParse, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope) {
    // TODO use a taskqueue instead? Some refactoring of taskQueue for accepting a long string with ';' is required
    auto& [ctx, scope] = ctxAndScope;
    for (auto const& task : Utility::StringHandler::split(stringToParse, ';')) {
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

void ActionsImpl::applySpecialAction(SpecialAction::Type const& action, Graphics::RmlInterface& manager, Rml::Element* element, Rml::ElementDocument* document) {
    switch (action) {
    case SpecialAction::Type::blurElement:
        if (element) element->Blur();
        break;
    case SpecialAction::Type::deleteDocument:
        if (document) manager.removeDocument(document);
        break;
    default:
        std::unreachable();
    }
}

} // namespace Nebulite::Interaction
