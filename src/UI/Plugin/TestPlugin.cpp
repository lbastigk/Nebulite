#include "UI/Plugin/TestPlugin.hpp"

#include "Nebulite.hpp"
#include "ScopeAccessor.hpp"
#include "Utility/StringHandler.hpp"

#include <iostream>
#include <numeric>

namespace Nebulite::UI::Plugin {

void TestPlugin::update() {
    auto const at = ScopeAccessor::Full();
    Interaction::ContextScope const ctx{
        Global::shareScope(at),
        Global::shareScope(at),
        Global::shareScope(at)
    };

    for (auto const& elements : std::views::values(expressions)) {
        for (auto& [element, expression] : elements) {
            if (!element) continue;
            auto const result = expression.eval(ctx);
            element->SetInnerRML(result);
        }
    }
}

void TestPlugin::OnInitialise() {

}

void TestPlugin::OnShutdown() {

}

// NOLINTNEXTLINE
void TestPlugin::OnDocumentOpen(Rml::Context* /*context*/, const Rml::String& /*document_path*/) {

}

void TestPlugin::OnDocumentLoad(Rml::ElementDocument* document) {
    if (!document) return;
    compileDocument(document, document, 0);
}

void TestPlugin::OnDocumentUnload(Rml::ElementDocument* document) {
    expressions.erase(document);
}

void TestPlugin::OnContextCreate(Rml::Context* /*context*/) {

}

void TestPlugin::OnContextDestroy(Rml::Context* /*context*/) {

}

void TestPlugin::OnElementCreate(Rml::Element* /*element*/) {

}

void TestPlugin::OnElementDestroy(Rml::Element* /*element*/) {

}

void TestPlugin::compileDocument(Rml::ElementDocument* root, Rml::Element* element, size_t const& depth) {
    if (!element) return;

    // Show all strings with data-eval attribute, which need to be stored as expressions
    if (std::string const inner = element->GetInnerRML(); !inner.empty() && element->GetAttribute("data-eval")) {
        std::cout << Utility::StringHandler::replaceAll(inner, "\n", "\\n") << std::endl;
        expressions[root].emplace(element, Interaction::Logic::Expression(inner));
    }

    auto const count = static_cast<size_t>(element->GetNumChildren());
    std::vector<int> indices(count);
    std::iota(indices.begin(), indices.end(), 0);
    std::ranges::for_each(indices.begin(), indices.end(),[&](size_t const i){
        compileDocument(root, element->GetChild(static_cast<int>(i)), depth + 1);
    });
}

} // namespace Nebulite::UI::Plugin
