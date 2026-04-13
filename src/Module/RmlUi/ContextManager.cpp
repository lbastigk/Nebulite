

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
    for (auto const& element : toRegister) {
        if (renderer.getRmlElementContextScope(element).has_value()) {
            capture.warning.println("Element already has a context. Skipping context assignment for element: " + std::string(element->GetTagName()));
            continue;
        }

        // See if a document is available now
        if (element->GetOwnerDocument()) {
            if (auto context = renderer.getRmlDocumentContextScope(element->GetOwnerDocument()); context.has_value()) {
                renderer.setRmlElementContextScope(element, context.value());
            }
            // Edge case: document and element have the same tag name?
            else if (element->GetTagName() == element->GetOwnerDocument()->GetTagName()) {
                renderer.setRmlElementContextScope(element, {
                    .self = global,
                    .other = global,
                    .global = global
                });
            }
            else {
                capture.warning.println("Document does not have a context. Skipping context assignment for element: " + std::string(element->GetTagName()));
                capture.warning.println("Document Tag name:", element->GetOwnerDocument()->GetTagName());
            }
        }
    }
    toRegister.clear();
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
}

void ContextManager::OnDocumentUnload(Rml::ElementDocument* document) {

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
