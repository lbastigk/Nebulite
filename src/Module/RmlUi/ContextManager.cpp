//------------------------------------------
// Includes

// External
#include <RmlUi/Core/Element.h>

// Nebulite
#include "Graphics/RmlInterface.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Module/RmlUi/Conditional.hpp"
#include "Module/RmlUi/ContextManager.hpp"
#include "Module/RmlUi/EventBridge.hpp"
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Module/RmlUi/Reflection.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ContextManager::ContextManager(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

void ContextManager::update() {
    auto anySupportedAttribute = [] (Rml::Element* element, Rml::Element* parent) {
        // Skip elements that are part of a reflection, as they will be handled by the Reflection module
        // TODO: is a parent escalation necessary? Checking parent of parent etc ... ?
        if (Reflection::Attribute::hasSupportedAttribute(parent)) return false;

        // Check for supported attributes
        return ExpressionManager::Attribute::hasSupportedAttribute(element) // Any expression requires context
            || EventBridge::Attribute::hasSupportedAttribute(element)  // Any interactive event requires context
            || Conditional::Attribute::hasSupportedAttribute(element)  // Conditionals use Expressions -> requires context
            // ^ Add any new attributes here ^
        ;
    };

    for (auto const& document : interface.getOpenedDocuments()) {
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* parent) {
            if (anySupportedAttribute(element, parent)) {
                if (Graphics::RmlInterface::RmlElementIdentifier const elementId(element); !interface.getRmlElementContextAndScope(elementId).has_value()) {
                    if (auto const ctx = interface.getRmlDocumentContextAndScope(document); ctx.has_value()) {
                        interface.setRmlElementContextAndScope(elementId, ctx.value());
                    }
                }
            }
        });
    }
}

} // namespace Nebulite::Module::RmlUi
