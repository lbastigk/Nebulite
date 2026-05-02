#include "Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Module/RmlUi/Ruleset.hpp"

#include "Interaction/Execution/Domain.hpp"


namespace Nebulite::Module::RmlUi {

Ruleset::Ruleset(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

void Ruleset::update() {
    for (auto& toApply : rulesetsToApply) {
        toApply.applyRuleset(capture, interface);
    }
    rulesetsToApply.clear();
}

void Ruleset::OnElementDestroy(Rml::Element* element){
    if (!element) return;
    auto hasSupportedAttribute = [](Rml::Element* e) {
        return e->GetAttribute(rulesetAttributeOnDestroy) || e->GetAttribute(parseOnDestroy);
    };

    if (hasSupportedAttribute(element)) {
        if (std::string const tag = element->GetTagName(); tag == "rml" || tag == "head" || tag == "body") {
            capture.warning.println("Unsupported ruleset invocation position at tag: ", tag, ". Tags rml, head and body are not supported!");
            capture.warning.println("Please place the tag in a separate div inside the body tag.");
            return;
        }

        DeletedElement toAdd;
        if (Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(element)) {
            toAdd.identifier = Graphics::RmlInterface::RmlElementIdentifier(element);
        }
        else if (interface.getRmlDocumentContextAndScope(element->GetOwnerDocument()).has_value()) {
            toAdd.identifier = element->GetOwnerDocument();
        }
        else {
            capture.warning.println("Could not find identifier for element. Skipping ruleset invocation on element destroy.");
            return;
        }

        if (auto const var = element->GetAttribute(rulesetAttributeOnDestroy); var) {
            toAdd.rulesetLink = var->Get<Rml::String>();
        }
        if (auto const val = element->GetAttribute(parseOnDestroy); val) {
            toAdd.stringToParse = val->Get<Rml::String>();
        }

        rulesetsToApply.emplace_back(std::move(toAdd));
    }
}

void Ruleset::DeletedElement::applyRuleset(Utility::IO::Capture& capture, Graphics::RmlInterface& interface){
    auto ctxAndScope = [&] -> std::optional<Graphics::RmlInterface::ContextAndScope> {
        // NOLINTNEXTLINE
        return std::visit([&](auto&& id) -> std::optional<Graphics::RmlInterface::ContextAndScope> {
            using T = std::decay_t<decltype(id)>;
            if constexpr (std::is_same_v<T, Graphics::RmlInterface::RmlElementIdentifier>) {
                return interface.getRmlElementContextAndScope(id);
            }
            else if constexpr (std::is_same_v<T, Rml::ElementDocument*>) {
                return interface.getRmlDocumentContextAndScope(id);
            }
            else {
                std::unreachable();
            }
        }, identifier);
    }();
    if (!ctxAndScope) {
        // For some reason, rulesets are executed twice if we delete a domain with an open document. This catches the second call.
        //if (rulesetLink) capture.warning.println("Owner context was deleted, cannot apply ruleset: ", rulesetLink.value());
        //if (stringToParse) capture.warning.println("Owner context was deleted, cannot parse string: ", stringToParse.value());
        return;
    }
    auto& [ctx, scope] = ctxAndScope.value();

    // 1.) rulesetAttributeOnDestroy
    if (rulesetLink) {
        if (auto const ruleset = Interaction::Rules::Construction::RulesetCompiler::parseSingle(rulesetLink.value(), ctx.self); ruleset) {
            // TODO: add functionality to pass ctx and scope into Ruleset::apply
            ruleset.value()->apply();
        }
        else {
            capture.warning.println("Could not find ruleset with identifier '", rulesetLink.value(), "'. Skipping ruleset invocation on element destroy.");
        }
    }

    // 2.) parseOnDestroy
    if (stringToParse) {
        for (auto& task : Utility::StringHandler::split(stringToParse.value(), ';')) {
            std::string_view taskView = task;
            Utility::StringHandler::lStrip(taskView);
            switch (std::string const str = std::string(__FUNCTION__) + " " + taskView; ctx.self.parseStr(str,ctx, scope)) {
            case Constants::Event::Warning:
                capture.warning.println("Parsing string on element destroy resulted in warning. String: ", str);
                break;
            case Constants::Event::Error:
                capture.error.println("Parsing string on element destroy resulted in error. String: ", str);
                break;
            case Constants::Event::Success:
                break;
            default:
                std::unreachable();
            }
        }
    }
}

} // namespace Nebulite::Module::RmlUi
