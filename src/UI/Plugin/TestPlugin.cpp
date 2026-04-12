
//------------------------------------------
// Includes

// Standard library
#include <numeric>

// External

// Nebulite
#include "Module/RmlUi/TestPlugin.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::UI::Plugin {

void TestPlugin::update() {
    Interaction::ContextScope const ctx{
        .self = Global::shareScope(accessToken),
        .other = Global::shareScope(accessToken),
        .global = Global::shareScope(accessToken)
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

// TODO: Add attributes such as data-reflect that loop through each member of a given scope to, for example, dynamically generate lists
void TestPlugin::compileDocument(Rml::ElementDocument* root, Rml::Element* element, size_t const& depth) {
    if (!element) return;
    if (element->GetAttribute("data-eval")) {
        // Nebulite Expression can handle text, so we don't need to sanitize the inner RML in any way.
        expressions[root].emplace(element, Interaction::Logic::Expression(element->GetInnerRML()));
    }
    else { // Do not evaluate any inner elements, as this could cause issues with pointers: parent element gets re-evaluated, potentially breaking element pointers of children.
        auto const count = static_cast<size_t>(element->GetNumChildren());
        std::vector<int> indices(count);
        std::ranges::iota(indices, 0);
        std::ranges::for_each(indices.begin(), indices.end(),[&](int const& idx){
            compileDocument(root, element->GetChild(idx), depth + 1);
        });
    }
}

} // namespace Nebulite::UI::Plugin
