//------------------------------------------
// Includes

// Standard library
#include <numeric>

// External

// Nebulite
#include "Module/RmlUi/Reflection.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

Reflection::Reflection(Utility::Capture& c, Core::Renderer& r) : RmlUiModule(c,r) {
    evaluationRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
        [this] {
            Interaction::ContextScope const ctx{
                .self = global,
                .other = global,
                .global = global
            };
            removeDeletedElements();
            reflect();
        },
        10, // Update every 10ms
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

void Reflection::OnDocumentLoad(Rml::ElementDocument* document) {

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
    if (element->GetAttribute("data-reflect")) {
        auto const expression = std::string(element->GetAttribute("data-reflect")->Get<Rml::String>());
        reflections[element->GetOwnerDocument()].emplace(
            element,
            ReflectionEntry{
                .entries = Interaction::Logic::Expression(expression),
                .context = Interaction::ContextScope{
                    .self = global,
                    .other = global,
                    .global = global
                },
                .rmlValue = element->GetInnerRML(),
                .markedForDeletion = false
            }
        );
    }
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

void Reflection::reflect(){
    for (auto& elements : std::views::values(reflections)) {
        for (auto& [element, entry] : elements) {
            if (!element) continue;
            if (entry.markedForDeletion) continue;
            auto const jsonResult = entry.entries.evalAsJson(entry.context);
            if (auto const type = jsonResult.memberType(""); type != Data::KeyType::array) {
                capture.warning.println("Reflection expression did not evaluate to an array. Skipping reflection. Result: " + jsonResult.serialize());
                continue;
            }
            auto repeat = [](size_t const count, std::string const& str) {
                std::string result;
                for (auto _ : std::views::iota(size_t{0}, count)) {
                    result += str;
                }
                return result;
            };
            if (entry.rmlValue.empty()) {
                entry.rmlValue = element->GetInnerRML();
            }

            size_t const size = jsonResult.memberSize("");
            std::string const newRml = repeat(size, entry.rmlValue);
            element->SetInnerRML(newRml);

            // Check children size
            if (size_t const childrenCount = element->GetNumChildren(); childrenCount != size) {
                capture.warning.println("Rml Children count does not match reflection count. Expected: ", size, ", Actual: ", childrenCount);
            }
            else {
                // Overwrite context for each element
            }
        }
    }
}


} // namespace Nebulite::Module::RmlUi
