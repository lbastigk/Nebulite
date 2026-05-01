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
    for (auto const& document : interface.getOpenedDocuments()) {
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* parent) {
            if (element->GetAttribute(ExpressionManager::evalAttribute) || element->GetAttribute(ExpressionManager::conditionalAttribute)) {
                // Skip elements that are part of a reflection, as they will be handled by the Reflection module
                // But not reflectionOnceAttribute, as those are only reflected once and then should be handled like normal elements
                if (!parent->GetAttribute(Reflection::reflectionAttribute)) {
                    if (Graphics::RmlInterface::RmlElementIdentifier const elementId(element); !interface.getRmlElementContextAndScope(elementId).has_value()) {
                        if (auto const ctx = interface.getRmlDocumentContextAndScope(document); ctx.has_value()) {
                            interface.setRmlElementContextAndScope(elementId, ctx.value());
                        }
                    }
                }
            }
        });
    }
}

} // namespace Nebulite::Module::RmlUi
