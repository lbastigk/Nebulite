//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

// External
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
    constexpr std::size_t MAX_ITERATIONS = 100;

    // Resolve known entries first
    for (auto& [id, entry] : registeredEntries) {
        entry.resolve(id, interface);
    }

    for (std::size_t i = 0; i < MAX_ITERATIONS; ++i) {
        if (newEntries.empty()) {
            return;
        }

        // Resolve new entries
        decltype(newEntries) entriesToResolve;
        std::swap(entriesToResolve, newEntries);

        for (auto& [id, entry] : entriesToResolve) {
            entry.resolve(id, interface);
            registeredEntries.emplace(id, std::move(entry));
        }
        newEntries.clear();
    }

    throw std::runtime_error("Conditional::update exceeded maximum iterations");
}

void Conditional::OnElementCreate(Rml::Element* element){
    if (!element) return;

    // Check for if-attribute
    if (Attribute::hasSupportedAttribute(element)) {
        auto id = Graphics::RmlInterface::RmlElementIdentifier(element);
        auto const condition = element->GetAttribute(Attribute::conditional)->Get<Rml::String>();
        newEntries.emplace(id, RegisteredEntry{
            .condition = Interaction::Logic::Expression(condition),
            .innerRml = "",
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
        newEntries.erase(id);
    }
}

void Conditional::RegisteredEntry::resolve(Graphics::RmlInterface::RmlElementIdentifier const& id, Graphics::RmlInterface& interface){
    if (newEntry) {
        newEntry = false;
        innerRml = element->GetInnerRML();
    }
    auto const ctxAndScope = interface.getRmlElementContextAndScope(id);
    if (!ctxAndScope) return;
    if (!Math::isZero(std::stod(condition.eval(ctxAndScope->ctxScope)))) {
        element->SetInnerRML(innerRml);
    }
    else {
        element->SetInnerRML("");
    }
}

} // namespace Nebulite::Module::RmlUi
