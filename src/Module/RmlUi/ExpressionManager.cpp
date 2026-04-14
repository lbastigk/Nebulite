//------------------------------------------
// Includes

// Nebulite
#include "Module/RmlUi/ExpressionManager.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

ExpressionManager::ExpressionManager(Utility::Capture& c, Core::Renderer& r) : RmlUiModule(c,r) {
    evaluationRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
        [this] {
            updateDataValues();
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

void ExpressionManager::OnElementCreate(Rml::Element* element) {
    if (!element) return;

    // Check if the element has a data-value
    auto const dataAttributes = {
        "data-value",
        "data-if"
    };
    for (auto const& attribute : dataAttributes) {
        auto const rmlValue = element->GetAttribute(attribute);
        if (!rmlValue) continue;

        // TODO: use attribute data-context. Fallback to global if not available
        // Still, we need a document to owner map, so we can dynamically set context. Should be enough to use the callerScope as self/other

        if (rmlValue->GetType() == Rml::Variant::STRING) {
            auto const keyStr = std::string(rmlValue->Get<Rml::String>());
            if (auto it = registeredStrings.find(keyStr); it == registeredStrings.end()) {
                // Create entry
                Data::ScopedKey const key{keyStr};
                auto const value = global.get<std::string>(key).value_or("");
                auto entry = std::make_unique<RegisteredEntry>();
                entry->element = element;
                entry->currentRmlValue = value;
                entry->previousRmlValue = value;
                entry->previousDocumentValue = value;

                // Register entry
                renderer.getDataModelConstructor().Bind(keyStr, &entry->currentRmlValue);
                registeredStrings.emplace(keyStr, std::move(entry));
            }
        }
    }
}

void ExpressionManager::OnElementDestroy(Rml::Element* /*element*/) {

}

//----------------------------------------------

void ExpressionManager::updateExpressions(){
    for (auto const& document : documents) {
        updateElement(document, [&](Rml::Element* element, Rml::Element* parent, size_t const& index) {
            if (element->GetAttribute("data-eval") || element->GetAttribute("data-if")) {
                // On element creation, the inner rml is not set. So we create an empty ElementEntry that is populated later on.
                Rml::String innerRml = element->GetInnerRML();
                rmlStrings[element] = innerRml;
                if (auto const it = expressions.find(innerRml); it == expressions.end()) {
                    expressions.emplace(innerRml, Interaction::Logic::Expression(innerRml));
                }

                Core::Renderer::RmlInterface::RmlElementIdentifier const elementId(parent, index, element);
                if (auto const context = renderer.getRmlElementContextScope(elementId); context.has_value()) {
                    if (auto const it = expressions.find(innerRml); it != expressions.end()) {
                        std::string const& evaluated = it->second.eval(context.value());
                        element->SetInnerRML(evaluated);
                    }
                }
            }
        });
    }
}

void ExpressionManager::resetExpressions(){
    for (auto const& document : documents) {
        updateElement(document, [&](Rml::Element* element, Rml::Element* /*parent*/, size_t const& /*index*/) {
            if (element->GetAttribute("data-eval") || element->GetAttribute("data-if")) {
                // Reset
                element->SetInnerRML(rmlStrings[element]);
            }
        });
    }
}

void ExpressionManager::updateDataValues() {
    for (auto const& [keyStr, entry] : registeredStrings) {
        auto key = Data::ScopedKey{keyStr};

        // Determine data flow...
        auto& currentRml = entry->currentRmlValue;
        auto const& previousRml = entry->previousRmlValue;
        auto currentDocument = global.get<std::string>(key).value_or("");
        auto const& previousDocument = entry->previousDocumentValue;

        // 1.) rml -> document
        if (currentRml != previousRml) {
            global.set<std::string>(key, currentRml);
            entry->currentRmlValue = currentRml;
            entry->previousRmlValue = currentRml;
            entry->previousDocumentValue = currentRml;
        }
        // 2.) document -> rml
        else if (currentDocument != previousDocument) {
            entry->element->SetAttribute("value", currentDocument);
            entry->currentRmlValue = currentDocument;
            entry->previousRmlValue = currentDocument;
            entry->previousDocumentValue = currentDocument;
            entry->element->GetOwnerDocument()->UpdateDocument();
        }
    }
}

} // namespace Nebulite::Module::RmlUi
