//------------------------------------------
// Includes

// Standard Library
#include <regex>

// Nebulite
#include "Module/RmlUi/DataReference.hpp"
#include "Module/RmlUi/Reflection.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

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

    // Reflect on each update
    if (element->GetAttribute(reflectionAttribute)) {
        auto const expression = std::string(element->GetAttribute(reflectionAttribute)->Get<Rml::String>());
        reflections.emplace(
            element,
            ReflectionEntry{
                .reflectionListExpression = Interaction::Logic::Expression(expression),
                .reflectionList = Data::JSON(),
                .rmlValue = element->GetInnerRML(),
                .markedForDeletion = false,
                .allocatedIds = {}
            }
        );
    }
    // Reflect once (Important for interactive UI-Elements, as data-reflect would invalidate any fields, causing them to unfocus on each update
    else if (element->GetAttribute(reflectionOnceAttribute)) {
        auto const expression = std::string(element->GetAttribute(reflectionOnceAttribute)->Get<Rml::String>());
        reflectOnce.emplace_back(
            element,
            ReflectionEntry{
                .reflectionListExpression = Interaction::Logic::Expression(expression),
                .reflectionList = Data::JSON(),
                .rmlValue = element->GetInnerRML(),
                .markedForDeletion = false,
                .allocatedIds = {}
            }
        );
    }
    // TODO: Reflect-on-change?
}

void Reflection::OnElementDestroy(Rml::Element* element) {
    if (!element) return;
    if (auto const it = reflections.find(element); it != reflections.end()) {
        it->second.markedForDeletion = true;
    }
}

void Reflection::removeDeletedElements(){
    std::vector<Rml::Element*> elementsToRemove;
    for (auto& [element, entry] : reflections) {
        if (entry.markedForDeletion) {
            elementsToRemove.emplace_back(element);
        }
    }
    for (auto const& elementToRemove : elementsToRemove) {
        reflections.erase(elementToRemove);
    }
}

//----------------------------------------------

void Reflection::reflect(){
    for (auto& [element, entry] : reflections) {
        reflectElement(element, entry);
    }

    for (auto& [element, entry] : reflectOnce) {
        reflectElement(element, entry);
    }
    reflectOnce.clear();
}

void Reflection::reflectElement(Rml::Element* element, ReflectionEntry& entry) const {
    if (!element) return;
    if (entry.markedForDeletion) return;

    // Get owner context, keep nearly everything the same but nest contextScope self
    auto const ownerContextAndScope = interface.getRmlDocumentContextAndScope(element->GetOwnerDocument());
    if (!ownerContextAndScope) {
        return;
    }
    auto const& ownerContext = ownerContextAndScope.value().ctx;
    auto const& ownerContextScope = ownerContextAndScope.value().ctxScope;

    // Evaluate expression, result must be an array
    entry.reflectionList = entry.reflectionListExpression.evalAsJson(ownerContextScope);
    if (auto const type = entry.reflectionList.memberType(""); type != Data::KeyType::array) {
        capture.warning.println("Reflection expression did not evaluate to an array. Skipping reflection. Result: " + entry.reflectionList.serialize());
        return;
    }

    // Combine inner RMLs and replace the referenceIdentifierAttribute with a unique id
    if (entry.rmlValue.empty()) {
        entry.rmlValue = element->GetInnerRML();
    }
    size_t const size = entry.reflectionList.memberSize("");
    std::string newRml;
    for (size_t i = 0; i < size; ++i) {
        newRml += entry.rmlValue;
    }
    element->SetInnerRML(newRml);

    // Check children size
    if (auto const childrenCount = static_cast<size_t>(element->GetNumChildren()); childrenCount % size == 0) {
        // For each element, overwrite context mapping
        for (size_t i = 0; i < childrenCount; ++i) {
            auto const jsonIndex = i * size / childrenCount;
            std::string const childKey = "[" + std::to_string(jsonIndex) + "]";
            auto& newScope = entry.reflectionList.shareManagedScopeBase(childKey);

            // Keep nearly all context and contextScope the same, but modify scope of self
            Interaction::ContextScope const childContextScope{
                {
                    .self = newScope,
                    .other = ownerContextScope.other,
                    .global = ownerContextScope.global,
                }
            };
            auto const child = element->GetChild(static_cast<int>(i));
            if (entry.allocatedIds.size() > i) {
                auto childId = Graphics::RmlInterface::RmlElementIdentifier(entry.allocatedIds[i]);
                interface.setRmlElementContextAndScope(childId, {ownerContext, childContextScope});
            }
            else {
                Graphics::RmlInterface::RmlElementIdentifier::removeElementIdentifier(child);
                Graphics::RmlInterface::RmlElementIdentifier childId(child);
                entry.allocatedIds.push_back(childId.getId());
                interface.setRmlElementContextAndScope(childId, {ownerContext, childContextScope});
            }
            Graphics::RmlInterface::RmlElementIdentifier::forceElementIdentifier(child, entry.allocatedIds[i]);
        }
    }
    else {
        capture.warning.println("Rml Children count does not match reflection count. Expected a multiple of: ", size, ", Actual: ", childrenCount, ". Skipping reflection, Something went seriously wrong...");
    }
}

} // namespace Nebulite::Module::RmlUi
