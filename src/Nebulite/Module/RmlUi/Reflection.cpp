//------------------------------------------
// Includes

// Standard Library
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// External
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Element.h>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Graphics/RmlInterface.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Module/Base/RmlUiModule.hpp"
#include "Nebulite/Module/RmlUi/Reflection.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Coordination/TimedRoutine.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

Reflection::Reflection(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {
    evaluationRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
        [this] {
            removeDeletedElements();
            reflect();
        },
        static_cast<size_t>(1000.0 / 144.0), // ms
        Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    );
}

void Reflection::update() {
    evaluationRoutine->update();
}

void Reflection::OnElementCreate(Rml::Element* element) {
    if (!element) return;

    auto addToList = [&](std::string const& attribute, std::vector<std::pair<Rml::Element*, std::unique_ptr<ReflectionEntry>>>& list) {
        if (element->GetAttribute(attribute)) {
            auto const expression = std::string(element->GetAttribute(attribute)->Get<Rml::String>());
            if (expression.empty()) {
                capture.warning.println("Element has empty reflection expression, skipping reflection.");
                return;
            }
            list.emplace_back(
                element,
                std::make_unique<ReflectionEntry>(expression, element)
            );
        }
    };

    addToList(Attribute::reflect, toAdd.reflections);
    addToList(Attribute::reflectOne, toAdd.reflectOnce);
}

void Reflection::OnElementDestroy(Rml::Element* element) {
    if (!element) return;
    if (auto const it = reflections.find(element); it != reflections.end()) {
        if (it->second) it->second->markedForDeletion = true;
    }
}

void Reflection::removeDeletedElements(){
    std::vector<Rml::Element*> elementsToRemove;
    for (auto& [element, entry] : reflections) {
        if (!entry || entry->markedForDeletion) {
            elementsToRemove.emplace_back(element);
        }
    }
    for (auto const& elementToRemove : elementsToRemove) {
        reflections.erase(elementToRemove);
    }
}

//----------------------------------------------

void Reflection::reflect(){
    // Add new reflections
    for (auto& [element, entry] : toAdd.reflections) {
        reflections.emplace(element, std::move(entry));
    }
    toAdd.reflections.clear();
    for (auto& [element, entry] : toAdd.reflectOnce) {
        reflectOnce.emplace_back(element, std::move(entry));
    }
    toAdd.reflectOnce.clear();

    // Reflect
    for (auto& [element, entry] : reflections) {
        reflectElement(element, entry);
    }
    for (auto& [element, entry] : reflectOnce) {
         reflectElement(element, entry);
    }
    reflectOnce.clear();
}

Data::JSON& Reflection::evaluateReflectionList(std::unique_ptr<ReflectionEntry> const& entry, Rml::Element* element, Interaction::ContextScope const& scope){
    auto* const elementDocument = element->GetOwnerDocument();
    if (reflectionResults[elementDocument].find(element) == reflectionResults[elementDocument].end()) {
        reflectionResults[elementDocument][element] = std::make_unique<Data::JSON>();
    }
    auto const& reflectionList = reflectionResults[elementDocument][element];
    *reflectionList = entry->reflectionListExpression.evalAsJson(scope);
    return *reflectionList;
}

void Reflection::setIdentifiers(Rml::Element* element, Graphics::RmlInterface::RmlElementIdentifier const& id) {
    Graphics::RmlInterface::RmlElementIdentifier::forceElementIdentifier(element, id);
    for (int j = 0; j < element->GetNumChildren(); ++j) {
        setIdentifiers(element->GetChild(j), id);
    }
}

void Reflection::setReflectionScopes(Data::JSON& reflectionList, std::unique_ptr<ReflectionEntry> const& entry, Rml::Element const* element, Graphics::RmlInterface::ContextAndScope const& contextAndScope) const {
    auto const childrenCount = static_cast<size_t>(element->GetNumChildren());
    std::size_t idsIndex = 0;

    while (entry->allocatedIds.size() < childrenCount) {
        entry->allocatedIds.emplace_back(Graphics::RmlInterface::RmlElementIdentifier::newIdentifier());
    }

    std::size_t jsonIndex = 0;
    for (std::size_t i = 0; i < childrenCount; ++i) {
        auto* const child = element->GetChild(static_cast<int>(i));
        if (auto const* value = child->GetAttribute("reflectionIndex"); value) {
            jsonIndex = static_cast<size_t>(value->Get<int>());
            continue; // The div that holds the reflection index does not need context/scope.
        }

        std::string const childKey = Data::ScopedKey().addIndex(jsonIndex).toString();
        auto& newScope = reflectionList.shareManagedScope(childKey);

        // Keep nearly all context and contextScope the same, but modify scope of self
        Interaction::ContextScope const childContextScope{
            {
                .self = newScope,
                .other = contextAndScope.ctxScope.other,
                .global = contextAndScope.ctxScope.global,
            }
        };

        auto const& allocatedId = entry->allocatedIds[idsIndex];
        setIdentifiers(child, allocatedId);
        auto const identifier = Graphics::RmlInterface::RmlElementIdentifier(allocatedId);
        interface.setRmlElementContextAndScope(identifier, {.ctx=contextAndScope.ctx, .ctxScope=childContextScope});
        idsIndex++;
    }
}

void Reflection::reflectElement(Rml::Element* element, std::unique_ptr<ReflectionEntry> const& entry) {
    if (!element) return;
    if (!entry) return;
    if (entry->markedForDeletion) return;

    // Get context and scope of this reflection
    auto const contextAndScope = [&](Rml::Element const* e) -> std::optional<Graphics::RmlInterface::ContextAndScope> {
        if (Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(e)) {
            // If a reflection element already has an element identifier, it's likely that this is the result of a nested reflection.
            // At the moment, this is not supported.
            throw std::logic_error("Nested reflection is not supported. Use JSON-Transformations on the outer reflection to mimic the inner one.");
        }
        if (auto const ownerContextAndScope = interface.getRmlDocumentContextAndScope(e->GetOwnerDocument()); ownerContextAndScope) {
            return ownerContextAndScope;
        }
        return std::nullopt;
    }(element);
    if (!contextAndScope) {
        return;
    }

    // Get reflection result, must be an array to iterate over its members
    auto& reflectionList = evaluateReflectionList(entry, element, contextAndScope.value().ctxScope);
    if (auto const type = reflectionList.memberType(""); type != Data::KeyType::array) {
        capture.warning.println("Reflection expression '", entry->reflectionListExpression.getFullExpression() ,"' did not evaluate to an array. Skipping reflection.");
        capture.warning.println("Result: " + reflectionList.serialize());
        capture.warning.println("Self scope provided: ", contextAndScope.value().ctxScope.self.serialize() );
        std::abort();
    }

    // Combine inner RMLs and replace the referenceIdentifierAttribute with a unique id
    if (entry->rmlValue.empty()) {
        entry->rmlValue = element->GetInnerRML();
    }
    std::size_t const size = reflectionList.memberSize("");
    std::string newRml;
    for (std::size_t i = 0; i < size; ++i) {
        // Helps keeping track of the current index. Earlier versions relied on the fact that repeating the inner rml
        // means we can just increase the index every n elements, where n is the amount of elements per reflection entry.
        // But this approach limits our reflection abilities.
        // Later on we might wish to do nested reflections where the amount of elements per entry isn't guaranteed to be the same.
        // The dummy-element-approach addresses this problem
        newRml += "<div reflectionIndex=\"" + std::to_string(i) + "\"></div>";
        newRml += entry->rmlValue;
    }
    element->SetInnerRML(newRml);
    if (size == 0) return;
    setReflectionScopes(reflectionList, entry, element, contextAndScope.value());
}

} // namespace Nebulite::Module::RmlUi
