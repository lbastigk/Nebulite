

//------------------------------------------
// Includes

// Nebulite
#include "Module/RmlUi/ContextManager.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ContextManager::ContextManager(Utility::Capture& c, Core::Renderer& r) : RmlUiModule(c,r) {
    
}

void ContextManager::update() {
    for (auto const& document : documents) {
        updateElement(document, [&](Rml::Element* element, Rml::Element* parent, size_t const& index) {
            if (element->GetAttribute("data-eval") || element->GetAttribute("data-if")) {
                // Skip elements that are part of a reflection, as they will be handled by the Reflection module
                if (!parent->GetAttribute("data-reflect")) {
                    if (Core::Renderer::RmlInterface::RmlElementIdentifier const elementId(parent, index, element); !renderer.getRmlElementContextScope(elementId).has_value()) {
                        if (auto const ctx = renderer.getRmlDocumentContextScope(document); ctx.has_value()) {
                            renderer.setRmlElementContextScope(elementId, ctx.value());
                        }
                    }
                }
            }
        });
    }
}

void ContextManager::OnInitialise() {

}

void ContextManager::OnShutdown() {

}

// NOLINTNEXTLINE
void ContextManager::OnDocumentOpen(Rml::Context* /*context*/, const Rml::String& /*document_path*/) {
}

void ContextManager::OnDocumentLoad(Rml::ElementDocument* document) {
    if (!document) return;
    // For now, we set the every document context to {global, global, global}
    renderer.setRmlDocumentContextScope(document, {
        .self = global,
        .other = global,
        .global = global
    });
    documents.push_back(document);
}

void ContextManager::OnDocumentUnload(Rml::ElementDocument* document) {
    std::erase(documents, document);
}

void ContextManager::OnContextCreate(Rml::Context* /*context*/) {

}

void ContextManager::OnContextDestroy(Rml::Context* /*context*/) {

}

void ContextManager::OnElementCreate(Rml::Element* element) {
    toRegister.push_back(element);
}

void ContextManager::OnElementDestroy(Rml::Element* element) {
    std::erase(toRegister, element);
}

} // namespace Nebulite::Module::RmlUi
