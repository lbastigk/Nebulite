//------------------------------------------
// Includes

// External
#include <cstddef>
#include <cstdint> // NOLINT

// External
#include <RmlUi/Core/Element.h>

// Nebulite: RmlUi-Modules
#include "Nebulite/Graphics/RmlUi/ElementIdentifier.hpp"

//------------------------------------------
namespace Nebulite::Graphics::RmlUi {

size_t& ElementIdentifier::count() {
    static std::size_t rollingIdentifier = 0;
    return rollingIdentifier;
}

size_t ElementIdentifier::idRoll() {
    return count()++;
}

size_t ElementIdentifier::getCount() {
    return count();
}

void ElementIdentifier::forceElementIdentifier(Rml::Element* element, ElementIdentifier const& identifier) {
    element->SetAttribute(identifierAttribute, identifier.id);
}

void ElementIdentifier::removeElementIdentifier(Rml::Element* element) {
    element->RemoveAttribute(identifierAttribute);
}

bool ElementIdentifier::hasElementIdentifier(Rml::Element const* element){
    return element->HasAttribute(identifierAttribute);
}

ElementIdentifier::ElementIdentifier(Rml::Element* e){
    // See if element has attribute
    if (e->GetAttribute(identifierAttribute)) {
        id = e->GetAttribute(identifierAttribute)->Get<size_t>();
    }
    else {
        id = idRoll();
        e->SetAttribute(identifierAttribute, id);
    }
}

ElementIdentifier ElementIdentifier::newIdentifier() {
    return ElementIdentifier{idRoll()};
}

} // namespace Nebulite::Graphics::RmlUi
