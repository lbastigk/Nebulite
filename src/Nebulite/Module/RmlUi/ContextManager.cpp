//------------------------------------------
// Includes

// External
#include <RmlUi/Core/Element.h>

// Nebulite
#include "Nebulite/Graphics/RmlInterface.hpp"
#include "Nebulite/Module/Base/RmlUiModule.hpp"
#include "Nebulite/Module/RmlUi/Conditional.hpp"
#include "Nebulite/Module/RmlUi/ContextManager.hpp"
#include "Nebulite/Module/RmlUi/EventBridge.hpp"
#include "Nebulite/Module/RmlUi/ExpressionManager.hpp"
#include "Nebulite/Module/RmlUi/Reflection.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ContextManager::ContextManager(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

void ContextManager::update() {
    auto anySupportedAttribute = [] (Rml::Element* element, Rml::Element* parent) {
        // Skip elements that are part of a reflection, as they will be handled by the Reflection module
        // In fact, skip any elements that are a child/grandchild/... of a reflection
        auto* escalatedParent = parent;
        while (escalatedParent) {
            if (Reflection::Attribute::hasSupportedAttribute(escalatedParent)) {
                return false;
            }
            escalatedParent = escalatedParent->GetParentNode();
        }

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
