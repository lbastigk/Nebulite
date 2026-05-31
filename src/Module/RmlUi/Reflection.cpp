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
#include "Data/Document/JSON.hpp"
#include "Data/Document/KeyType.hpp"
#include "Graphics/RmlInterface.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Module/RmlUi/Reflection.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

Reflection::Reflection(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {
    evaluationRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
        [this] {
            removeDeletedElements();
            reflect();
        },
        10, // ms
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

    // Evaluate expression, result must be an array
    *reflectionList = entry->reflectionListExpression.evalAsJson(scope);
    return *reflectionList;
}

void Reflection::setReflectionScopes(
        Data::JSON& reflectionList,
        size_t const& listSize,
        std::unique_ptr<ReflectionEntry> const& entry,
        Rml::Element const* element,
        Interaction::Context const& context,
        Interaction::ContextScope const& scope
    ) const {
    auto const childrenCount = static_cast<size_t>(element->GetNumChildren());
    for (size_t i = 0; i < childrenCount; ++i) {
        auto const jsonIndex = i * listSize / childrenCount;
        std::string const childKey = Data::ScopedKey().addIndex(jsonIndex).toString();
        auto& newScope = reflectionList.shareManagedScopeBase(childKey);

        // Keep nearly all context and contextScope the same, but modify scope of self
        Interaction::ContextScope const childContextScope{
            {
                .self = newScope,
                .other = scope.other,
                .global = scope.global,
            }
        };
        auto* const child = element->GetChild(static_cast<int>(i));
        if (entry->allocatedIds.size() > i) {
            auto childId = Graphics::RmlInterface::RmlElementIdentifier(entry->allocatedIds[i]);
            interface.setRmlElementContextAndScope(childId, {.ctx=context, .ctxScope=childContextScope});
        }
        else {
            Graphics::RmlInterface::RmlElementIdentifier::removeElementIdentifier(child);
            Graphics::RmlInterface::RmlElementIdentifier const childId(child);
            entry->allocatedIds.push_back(childId.getId());
            interface.setRmlElementContextAndScope(childId, {.ctx=context, .ctxScope=childContextScope});
        }
        Graphics::RmlInterface::RmlElementIdentifier::forceElementIdentifier(child, entry->allocatedIds[i]);
    }
}

void Reflection::reflectElement(Rml::Element* element, std::unique_ptr<ReflectionEntry> const& entry) {
    if (!element) return;
    if (!entry) return;
    if (entry->markedForDeletion) return;

    // Get context and scope of this reflection
    auto const contextAndScope = [&](Rml::Element const* e) -> std::optional<Graphics::RmlInterface::ContextAndScope> {
        if (Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(e)) {
            // Nested reflections are incredibly difficult to realize and would require a complete overhaul of the current system
            // Perhaps later on we could use short-lived rml documents to generate reflection results and insert them in the main document,
            // keeping track of each JSON reflection result
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
    auto const& context = contextAndScope.value().ctx;
    auto const& scope = contextAndScope.value().ctxScope;

    // Get reflection result, must be an array to iterate over its members
    auto& reflectionList = evaluateReflectionList(entry, element, scope);
    if (auto const type = reflectionList.memberType(""); type != Data::KeyType::array) {
        capture.warning.println("Reflection expression '", entry->reflectionListExpression.getFullExpression() ,"' did not evaluate to an array. Skipping reflection.");
        capture.warning.println("Result: " + reflectionList.serialize());
        capture.warning.println("Self scope provided: ", scope.self.serialize() );
        std::abort();
    }

    // Combine inner RMLs and replace the referenceIdentifierAttribute with a unique id
    if (entry->rmlValue.empty()) {
        entry->rmlValue = element->GetInnerRML();
    }
    size_t const size = reflectionList.memberSize("");
    std::string newRml;
    for (size_t i = 0; i < size; ++i) {
        newRml += entry->rmlValue;
    }
    element->SetInnerRML(newRml);

    // Check children size
    if (static_cast<size_t>(element->GetNumChildren()) % size == 0) {
        // For each element, overwrite context mapping
        setReflectionScopes(reflectionList, size, entry, element, context, scope);
    }
    else {
        capture.warning.println("Rml Children count does not match reflection count. Expected a multiple of: ", size, ", Actual: ", element->GetNumChildren(), ". Skipping reflection, Something went seriously wrong...");
    }
}

} // namespace Nebulite::Module::RmlUi
