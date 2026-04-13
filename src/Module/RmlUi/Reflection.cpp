//------------------------------------------
// Includes

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
        1000, // ms
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

//----------------------------------------------

/*

TODO... does not work correctly at the moment. On every update, the result switches between two:

<p data-reflect="{global.time|listMembersAndValues}">
    <p data-eval="true">
        Member is: {self.key} with value: {self.value}.
    </p>
</p>

[FRAME 1]
Member is: {self.key} with value: {self.value}.
Member is: {self.key} with value: {self.value}.
Member is: {self.key} with value: {self.value}.
Member is: {self.key} with value: {self.value}.
Member is: dt with value: 0.001.

[FRAME 2]
Member is: {self.key} with value: {self.value}.
Member is: dt_ms with value: 1.
Member is: frameCount with value: 6422.
Member is: runtime with value: {object}.
Member is: t with value: 6.342.
Member is: t_ms with value: 6432.

capture output:

--------------------------
0x2a5e6d8 -> [0] = dt
0x2a5e540 -> [1] = dt_ms
0x2a5e3a8 -> [2] = frameCount
0x2a5e210 -> [3] = runtime
0x2a5e078 -> [4] = t
0x2a5dee0 -> [5] = t_ms
--------------------------
0x2a5dee0 -> [0] = dt
0x2a5e078 -> [1] = dt_ms
0x2a5e210 -> [2] = frameCount
0x2a5e3a8 -> [3] = runtime
0x2a5e540 -> [4] = t
0x2a5e6d8 -> [5] = t_ms

 */

void Reflection::reflect(){
    for (auto& elements : std::views::values(reflections)) {
        for (auto& [element, entry] : elements) {
            if (!element) continue;
            if (entry.markedForDeletion) continue;
            entry.jsonResult = entry.entries.evalAsJson(entry.context);
            if (auto const type = entry.jsonResult.memberType(""); type != Data::KeyType::array) {
                capture.warning.println("Reflection expression did not evaluate to an array. Skipping reflection. Result: " + entry.jsonResult.serialize());
                continue;
            }
            if (entry.rmlValue.empty()) {
                entry.rmlValue = element->GetInnerRML();
            }

            size_t const size = entry.jsonResult.memberSize("");
            std::string const newRml = Utility::StringHandler::repeat(entry.rmlValue, size);
            element->SetInnerRML(newRml);
            if (entry.markedForDeletion) continue;

            // Check children size
            if (auto const childrenCount = static_cast<size_t>(element->GetNumChildren()); childrenCount != size) {
                capture.warning.println("Rml Children count does not match reflection count. Expected: ", size, ", Actual: ", childrenCount);
            }
            else {
                // Overwrite context for each element
                capture.log.println("--------------------------");
                for (size_t i = 0; i < size; ++i) {
                    auto const& child = element->GetChild(static_cast<int>(i));
                    if (!child) {
                        capture.warning.println("Failed to get child at index ", i, " for element ", element->GetTagName());
                        continue;
                    }
                    std::string const childKey = "[" + std::to_string(i) + "]";
                    auto& newScope = entry.jsonResult.shareManagedScopeBase(childKey);
                    Interaction::ContextScope childContext{
                        .self = newScope,
                        .other = entry.context.other,
                        .global = entry.context.global,
                    };
                    renderer.setRmlElementContextScope(child, childContext);

                    capture.log.println(child, " -> ",childKey, " = ", entry.jsonResult.get<std::string>(childKey+".key").value_or("???"));
                }
            }
        }
    }
}

} // namespace Nebulite::Module::RmlUi
