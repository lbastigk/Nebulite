//------------------------------------------
// Includes

// External
#include <RmlUi/Core/Element.h>

// Nebulite
#include "Graphics/RmlInterface.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Module/RmlUi/ContextManager.hpp"
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Module/RmlUi/Reflection.hpp"
#include "Module/RmlUi/Ruleset.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ContextManager::ContextManager(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

void ContextManager::update() {
    auto anySupportedAttribute = [] (Rml::Element* element) {
        return element->GetAttribute(ExpressionManager::evalAttribute)
            || element->GetAttribute(ExpressionManager::conditionalAttribute)
            || element->GetAttribute(Ruleset::rulesetAttributeOnDestroy)
            || element->GetAttribute(Ruleset::parseOnDestroy);
    };

    for (auto const& document : interface.getOpenedDocuments()) {
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* parent) {
            if (anySupportedAttribute(element)) {
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
