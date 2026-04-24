//------------------------------------------
// Includes

// Nebulite
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ExpressionManager::ExpressionManager(Utility::IO::Capture& c, Core::Renderer& r) : RmlUiModule(c,r) {
    evaluationRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
        [this] {
            updateExpressions();
            expressionsWereEvaluated = true;
        },
        // If 1000.0/fps is higher than this value, the ui starts glitching due to the reset rml still being written while rendering.
        // So we update Expressions instantly with each new render pass.
        // Getting rid of the TimedRoutine is also an option, but we leave it here atm.
        0,
        Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    );
}

void ExpressionManager::update() {
    evaluationRoutine->update();
}

void ExpressionManager::postRenderUpdate() {
    if (expressionsWereEvaluated) {
        resetExpressions();
        expressionsWereEvaluated = false;
    }
}

void ExpressionManager::OnInitialise() {

}

void ExpressionManager::OnShutdown() {

}

// NOLINTNEXTLINE
void ExpressionManager::OnDocumentOpen(Rml::Context* /*context*/, const Rml::String& /*document_path*/) {

}

void ExpressionManager::OnDocumentLoad(Rml::ElementDocument* document) {
    documents.emplace_back(document);
}

void ExpressionManager::OnDocumentUnload(Rml::ElementDocument* document) {
    std::erase(documents, document);
}

void ExpressionManager::OnContextCreate(Rml::Context* /*context*/) {

}

void ExpressionManager::OnContextDestroy(Rml::Context* /*context*/) {

}

void ExpressionManager::OnElementCreate(Rml::Element* /*element*/) {

}

void ExpressionManager::OnElementDestroy(Rml::Element* /*element*/) {

}

//----------------------------------------------

void ExpressionManager::updateExpressions(){
    for (auto const& document : documents) {
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* parent, size_t const& index) {
            if (element->GetAttribute("data-eval") || element->GetAttribute("data-if")) {
                // On element creation, the inner rml is not set. So we create an empty ElementEntry that is populated later on.
                Rml::String innerRml = element->GetInnerRML();
                rmlStrings[element] = innerRml;
                if (auto const it = expressions.find(innerRml); it == expressions.end()) {
                    expressions.emplace(innerRml, Interaction::Logic::Expression(innerRml));
                }

                Graphics::RmlInterface::RmlElementIdentifier const elementId(parent, index, element);
                if (auto const context = renderer.getRmlElementContextAndScope(elementId); context.has_value()) {
                    if (auto const it = expressions.find(innerRml); it != expressions.end()) {
                        std::string const& evaluated = it->second.eval(context.value().ctxScope);
                        element->SetInnerRML(evaluated);
                    }
                }
            }
        });
    }
}

void ExpressionManager::resetExpressions(){
    for (auto const& document : documents) {
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* /*parent*/, size_t const& /*index*/) {
            if (element->GetAttribute("data-eval") || element->GetAttribute("data-if")) {
                // Reset
                element->SetInnerRML(rmlStrings[element]);
            }
        });
    }
}

} // namespace Nebulite::Module::RmlUi
