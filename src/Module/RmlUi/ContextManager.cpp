//------------------------------------------
// Includes

// Nebulite
#include "Module/RmlUi/ContextManager.hpp"
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Module/RmlUi/Reflection.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ContextManager::ContextManager(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {
    
}

void ContextManager::update() {
    for (auto const& document : documents) {
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* parent, size_t const& index) {
            if (element->GetAttribute(ExpressionManager::evalAttribute) || element->GetAttribute(ExpressionManager::conditionalAttribute)) {
                // Skip elements that are part of a reflection, as they will be handled by the Reflection module
                // But not reflectionOnceAttribute, as those are only reflected once and then should be handled like normal elements
                if (!parent->GetAttribute(Reflection::reflectionAttribute)) {
                    if (Graphics::RmlInterface::RmlElementIdentifier const elementId(parent, index, element); !interface.getRmlElementContextAndScope(elementId).has_value()) {
                        if (auto const ctx = interface.getRmlDocumentContextAndScope(document); ctx.has_value()) {
                            interface.setRmlElementContextAndScope(elementId, ctx.value());
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
