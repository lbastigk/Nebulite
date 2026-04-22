//------------------------------------------
// Includes

// Standard Library
#include <regex>

// Nebulite
#include "Module/RmlUi/Reflection.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

Reflection::Reflection(Utility::Capture& c, Core::Renderer& r) : RmlUiModule(c,r) {
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

void Reflection::OnInitialise() {

}

void Reflection::OnShutdown() {

}

// NOLINTNEXTLINE
void Reflection::OnDocumentOpen(Rml::Context* /*context*/, const Rml::String& /*document_path*/) {

}

void Reflection::OnDocumentLoad(Rml::ElementDocument* /*document*/) {

}

void Reflection::OnDocumentUnload(Rml::ElementDocument* document) {
    reflections.erase(document);
}

void Reflection::OnContextCreate(Rml::Context* /*context*/) {

}

void Reflection::OnContextDestroy(Rml::Context* /*context*/) {

}

void Reflection::OnElementCreate(Rml::Element* element) {
    if (!element) return;

    // Reflect on each update
    if (element->GetAttribute("data-reflect")) {
        auto const expression = std::string(element->GetAttribute("data-reflect")->Get<Rml::String>());
        reflections[element->GetOwnerDocument()].emplace(
            element,
            ReflectionEntry{
                .entries = Interaction::Logic::Expression(expression),
                .rmlValue = element->GetInnerRML(),
                .jsonResult = Data::JSON(),
                .markedForDeletion = false
            }
        );
    }
    // Reflect once (Important for interactive UI-Elements, as data-reflect would invalidate any fields, causing them to unfocus on each update
    else if (element->GetAttribute("data-reflect-once")) {
        auto const expression = std::string(element->GetAttribute("data-reflect-once")->Get<Rml::String>());
        reflectOnce.emplace_back(
            element,
            ReflectionEntry{
                .entries = Interaction::Logic::Expression(expression),
                .rmlValue = element->GetInnerRML(),
                .jsonResult = Data::JSON(),
                .markedForDeletion = false
            }
        );
    }
    // TODO: Reflect-on-change?
}

void Reflection::OnElementDestroy(Rml::Element* element) {
    if (!element) return;
    if (auto const it = reflections[element->GetOwnerDocument()].find(element); it != reflections[element->GetOwnerDocument()].end()) {
        it->second.markedForDeletion = true;
    }
}

void Reflection::removeDeletedElements(){
    for (auto& elements : std::views::values(reflections)) {
        std::vector<Rml::Element*> elementsToRemove;
        for (auto& [element, entry] : elements) {
            if (entry.markedForDeletion) {
                elementsToRemove.emplace_back(element);
            }
        }
        for (auto const& elementToRemove : elementsToRemove) {
            elements.erase(elementToRemove);
        }
    }
}

//----------------------------------------------

std::string Reflection::modifyDataIdentifier(const std::string& input, const size_t& index) {
    static const std::regex re(R"escape(data-identifier="([^"]*)")escape");

    std::string result;
    auto searchStart(input.cbegin());
    std::smatch match;

    while (std::regex_search(searchStart, input.cend(), match, re)) {
        result.append(match.prefix());

        result += "data-identifier=\"";
        result += match[1].str();
        result += "_REFLECT_INDEX_" + std::to_string(index);
        result += "\"";

        searchStart = match.suffix().first;
    }

    result.append(searchStart, input.cend());
    return result;
}

void Reflection::reflect(){
    for (auto& elements : std::views::values(reflections)) {
        for (auto& [element, entry] : elements) {
            reflectElement(element, entry);
        }
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
    auto const ownerContextAndScope = renderer.getRmlDocumentContextAndScope(element->GetOwnerDocument());
    if (!ownerContextAndScope) {
        return;
    }
    auto const& ownerContext = ownerContextAndScope.value().ctx;
    auto const& ownerContextScope = ownerContextAndScope.value().ctxScope;

    // Evaluate expression, result must be an array
    entry.jsonResult = entry.entries.evalAsJson(ownerContext);
    if (auto const type = entry.jsonResult.memberType(""); type != Data::KeyType::array) {
        capture.warning.println("Reflection expression did not evaluate to an array. Skipping reflection. Result: " + entry.jsonResult.serialize());
        return;
    }

    // Combine inner RMLs and replace the data-identifier with a unique id
    if (entry.rmlValue.empty()) {
        entry.rmlValue = element->GetInnerRML();
    }
    size_t const size = entry.jsonResult.memberSize("");
    std::string newRml;
    for (size_t i = 0; i < size; ++i) {
        newRml += modifyDataIdentifier(entry.rmlValue, i);
    }
    element->SetInnerRML(newRml);

    // Check children size
    if (auto const childrenCount = static_cast<size_t>(element->GetNumChildren()); childrenCount % size != 0) {
        capture.warning.println("Rml Children count does not match reflection count. Expected a multiple of: ", size, ", Actual: ", childrenCount, ". Skipping reflection, Something went seriously wrong...");
    }
    else {
        // For each element, overwrite context mapping
        for (size_t i = 0; i < childrenCount; ++i) {
            auto const& child = element->GetChild(static_cast<int>(i));
            if (!child) {
                capture.warning.println("Failed to get child at index ", i, " for element ", element->GetTagName());
                continue;
            }
            auto const jsonIndex = i * size / childrenCount;
            std::string const childKey = "[" + std::to_string(jsonIndex) + "]";
            auto& newScope = entry.jsonResult.shareManagedScopeBase(childKey);

            // Keep nearly all context and contextScope the same, but modify scope of self
            Interaction::ContextScope const childContextScope{
                .self = newScope,
                .other = ownerContextScope.other,
                .global = ownerContextScope.global,
            };
            Graphics::RmlInterface::RmlElementIdentifier childId(element, i, child);
            renderer.setRmlElementContextAndScope(childId, {ownerContext, childContextScope});
        }
    }
}

} // namespace Nebulite::Module::RmlUi
