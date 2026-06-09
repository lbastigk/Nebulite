//------------------------------------------
// Includes

// Standard library
#include <string>
#include <utility>

// External
#include <Core/ElementStyle.h>
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Element.h>

// Nebulite
#include "Graphics/RmlInterface.hpp"
#include "Interaction/Logic/Expression.hpp"
#include "Math/Equality.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Module/RmlUi/Conditional.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

Conditional::Conditional(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {}

void Conditional::update() {
    for (auto& [id, entry] : registeredEntries) {
        entry.resolve(id, interface);
    }
}

void Conditional::OnElementCreate(Rml::Element* element){
    if (!element) return;

    // Check for if-attribute
    if (Attribute::hasSupportedAttribute(element)) {
        auto id = Graphics::RmlInterface::RmlElementIdentifier(element);
        auto const condition = element->GetAttribute(Attribute::conditional)->Get<Rml::String>();
        registeredEntries.emplace(id, RegisteredEntry{
            .originalDisplay = Rml::Style::Display::Block, // default value, will be overridden on first resolve
            .condition = Interaction::Logic::Expression(condition),
            .element = element,
            .newEntry = true
        });
    }
}

void Conditional::OnElementDestroy(Rml::Element* element){
    if (!element) return;

    // Check for if-attribute
    if (Attribute::hasSupportedAttribute(element)) {
        auto const id = Graphics::RmlInterface::RmlElementIdentifier(element);
        registeredEntries.erase(id);
    }
}

void Conditional::RegisteredEntry::resolve(Graphics::RmlInterface::RmlElementIdentifier const& id, Graphics::RmlInterface& interface){
    if (newEntry) {
        newEntry = false;
        originalDisplay = element->GetDisplay();
    }
    auto const ctxAndScope = interface.getRmlElementContextAndScope(id);
    if (!ctxAndScope) {
        hideElement();
        return;
    }
    if (!Math::isZero(std::stod(condition.eval(ctxAndScope->ctxScope)))) {
        showElement();
    }
    else {
        hideElement();
    }
}

void Conditional::RegisteredEntry::showElement() const {
    element->GetStyle()->SetProperty(Rml::PropertyId::Display, originalDisplay);
}

void Conditional::RegisteredEntry::hideElement() const {
    element->GetStyle()->SetProperty(Rml::PropertyId::Display, Rml::Style::Display::None);
}

} // namespace Nebulite::Module::RmlUi
