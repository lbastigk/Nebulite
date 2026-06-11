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
    for (auto& entry : registeredEntries | std::views::values) {
        entry.resolve(interface);
    }
}

void Conditional::OnElementCreate(Rml::Element* element){
    if (!element) return;

    // Check for if-attribute
    if (Attribute::hasSupportedAttribute(element)) {
        auto const condition = element->GetAttribute(Attribute::conditional)->Get<Rml::String>();
        registeredEntries.emplace(element, RegisteredEntry{
            .element = element,
            .originalDisplay = element->GetDisplay(),
            .condition = Interaction::Logic::Expression(condition)
        });
    }
}

void Conditional::OnElementDestroy(Rml::Element* element){
    if (!element) return;

    // Check for if-attribute
    if (Attribute::hasSupportedAttribute(element)) {
        registeredEntries.erase(element);
    }
}

void Conditional::RegisteredEntry::resolve(Graphics::RmlInterface& interface) const {
    // Get context/scope for evaluation
    auto const ctxAndScope = [&] -> std::optional<Graphics::RmlInterface::ContextAndScope> {
        if (!Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(element)) {
            return std::nullopt; // Or should we set an identifier?
        }
        auto const id = Graphics::RmlInterface::RmlElementIdentifier(element);
        return interface.getRmlElementContextAndScope(id);
    }();

    // Show/Hide content
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
