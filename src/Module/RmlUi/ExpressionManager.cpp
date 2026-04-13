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
            removeDeletedElements();
            updateExpressions();
            updateDataValues();
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

void ExpressionManager::OnDocumentLoad(Rml::ElementDocument* /*document*/) {

}

void ExpressionManager::OnDocumentUnload(Rml::ElementDocument* document) {
    expressions.erase(document);
}

void ExpressionManager::OnContextCreate(Rml::Context* /*context*/) {

}

void ExpressionManager::OnContextDestroy(Rml::Context* /*context*/) {

}

void ExpressionManager::OnElementCreate(Rml::Element* element) {
    if (!element) return;
    if (element->GetAttribute("data-eval")) {
        // On element creation, the inner rml is not set. So we create an empty ElementEntry that is populated later on.
        expressions[element->GetOwnerDocument()].emplace(element, ElementEntry());
    }

    // TODO: For some reason this just causes the data-if field to be empty. Please Investigate!

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

void ExpressionManager::OnElementDestroy(Rml::Element* element) {
    for (auto& elementMap: std::views::values(expressions)) {
        if (auto const it = elementMap.find(element); it != elementMap.end()) {
            it->second.markedForDeletion = true;
        }
    }
}

//----------------------------------------------

void ExpressionManager::removeDeletedElements(){
    for (auto& elements : std::views::values(expressions)) {
        for (auto& [element, entry] : elements) {
            if (entry.markedForDeletion) {
                elements.erase(element);
            }
        }
    }
}

void ExpressionManager::updateExpressions(){
    Interaction::ContextScope const ctx{
        .self = global,
        .other = global,
        .global = global
    };

    for (auto& elements : std::views::values(expressions)) {
        for (auto& [element, entry] : elements) {
            if (!element) continue;
            if (entry.markedForDeletion) continue;
            if (!entry.expression.has_value()) {
                entry.expression.emplace(element->GetInnerRML());
            }
            auto const result = entry.expression.value().eval(ctx);
            element->SetInnerRML(result);
        }
    }
}

void ExpressionManager::updateDataValues() {
    for (auto const& [keyStr, entry] : registeredStrings) {
        auto key = Data::ScopedKey{keyStr};
        // Determine data flow
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
            entry->currentRmlValue = currentDocument;
            entry->previousRmlValue = currentDocument;
            entry->previousDocumentValue = currentDocument;
        }
    }
}

} // namespace Nebulite::Module::RmlUi
