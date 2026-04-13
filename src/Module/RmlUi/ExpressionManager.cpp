
//------------------------------------------
// Includes

// Standard library
#include <numeric>

// External

// Nebulite
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ExpressionManager::ExpressionManager(Utility::Capture& c) : RmlUiModule(c) {
    evaluationRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
        [this] {
            Interaction::ContextScope const ctx{
                .self = Global::shareScope(accessToken),
                .other = Global::shareScope(accessToken),
                .global = Global::shareScope(accessToken)
            };

            for (auto const& elements : std::views::values(expressions)) {
                for (auto& [element, expression] : elements) {
                    if (!element) continue;
                    auto const result = expression.eval(ctx);
                    element->SetInnerRML(result);
                }
            }
        },
        10, // Update every 10ms
        Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    );
}

void ExpressionManager::update() {
    evaluationRoutine->update();
}

void ExpressionManager::OnInitialise() {

}

void ExpressionManager::OnShutdown() {

}

// NOLINTNEXTLINE
void ExpressionManager::OnDocumentOpen(Rml::Context* /*context*/, const Rml::String& /*document_path*/) {

}

void ExpressionManager::OnDocumentLoad(Rml::ElementDocument* document) {
    if (!document) return;
    compileDocument(document, document, 0);
}

void ExpressionManager::OnDocumentUnload(Rml::ElementDocument* document) {
    expressions.erase(document);
}

void ExpressionManager::OnContextCreate(Rml::Context* /*context*/) {

}

void ExpressionManager::OnContextDestroy(Rml::Context* /*context*/) {

}

void ExpressionManager::OnElementCreate(Rml::Element* element) {
    // Check if the element has a data-value
    if (element) {
        auto const dataAttributes = {
            "data-value",
            "data-if"
        };
        for (auto const& attribute : dataAttributes) {
            auto const value = element->GetAttribute(attribute);
            if (!value) continue;

            if (value->GetType() == Rml::Variant::STRING) {
                auto const dataValue = std::string(value->Get<Rml::String>());
                capture.warning.println("Data attribute ", attribute," is not yet supported. Tried to bind value: ", dataValue);
            }
        }
    }
}

void ExpressionManager::OnElementDestroy(Rml::Element* element) {
    (void)element;
    // TODO: doesn't work: We cannot remove elements on destroy, as we might be looping through the maps!
    //       Instead, mark deleted and delete on update
    /*
    for (auto elementMap: expressions | std::views::values) {
        if (elementMap.find(element) != elementMap.end()) {
            elementMap.erase(elementMap.find(element));
        }
    }
    */
}


void ExpressionManager::compileDocument(Rml::ElementDocument* root, Rml::Element* element, size_t const& depth) {
    if (!element) return;
    if (element->GetAttribute("data-eval")) {
        // Nebulite Expression can handle text, so we don't need to sanitize the inner RML in any way.
        expressions[root].emplace(element, Interaction::Logic::Expression(element->GetInnerRML()));
    }
    else if (element->GetAttribute("data-reflect")) {
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
        auto key = std::string(element->GetAttribute("data-reflect")->Get<Rml::String>());
        capture.warning.println("Data attribute data-reflect is not yet supported. Tried reflect on key: ", key);
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
