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

            // Todo: reflect on each entry
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
    if (!document) return;
    compileDocument(document, document, 0);
}

void Reflection::OnDocumentUnload(Rml::ElementDocument* document) {
    expressions.erase(document);
}

void Reflection::OnContextCreate(Rml::Context* /*context*/) {

}

void Reflection::OnContextDestroy(Rml::Context* /*context*/) {

}

void Reflection::OnElementCreate(Rml::Element* element) {

}

void Reflection::OnElementDestroy(Rml::Element* /*element*/) {

}


void Reflection::compileDocument(Rml::ElementDocument* root, Rml::Element* element, size_t const& depth) {
    if (!element) return;
    if (element->GetAttribute("data-reflect")) {
        // TODO: Add attributes such as data-reflect that loop through each member of a given scope to, for example, dynamically generate lists
        // Idea: duplicate any inner rml for each member. The only issue is how we pass the correct context to each element. We need a way to store the context for each element.
        // The only way to also get the member key is to copy the values on each update?
        // e.g.: Reflect on global.obj with members a,b,c
        // <p data-reflect="global.obj">
        //   <p data-eval="true">
        //     Member is: {self.key} with value: {self.value}
        //   </p>
        // </p>
        // Would expand on every update
        auto expression = std::string(element->GetAttribute("data-reflect")->Get<Rml::String>());
        capture.warning.println("Data attribute data-reflect is not yet supported. Tried reflect on expression: ", expression);
    }
    else { // Do not evaluate any inner elements, as this could cause issues with pointers: parent element gets re-evaluated, potentially breaking element pointers of children.
        auto const count = static_cast<size_t>(element->GetNumChildren());
        std::vector<int> indices(count);
        std::ranges::iota(indices, 0);
        std::ranges::for_each(indices.begin(), indices.end(),[&](int const& idx){
            compileDocument(root, element->GetChild(idx), depth + 1);
        });
    }
}

} // namespace Nebulite::Module::RmlUi
