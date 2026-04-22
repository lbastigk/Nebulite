//------------------------------------------
// Includes

// Nebulite
#include "Module/RmlUi/DataInput.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

#include <complex>

//------------------------------------------

namespace Nebulite::Module::RmlUi {


DataInput::DataInput(Utility::Capture& c, Core::Renderer& r) : RmlUiModule(c,r) {
    evaluationRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
        [this] {
            updateDataValues();
        },
        // If 1000.0/fps is higher than this value, the ui starts glitching due to the reset rml still being written while rendering.
        // So we update Expressions instantly with each new render pass.
        // Getting rid of the TimedRoutine is also an option, but we leave it here atm.
        10,
        Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    );
}

void DataInput::update() {
    evaluationRoutine->update();
}

void DataInput::postRenderUpdate() {

}

void DataInput::OnInitialise() {

}

void DataInput::OnShutdown() {

}

// NOLINTNEXTLINE
void DataInput::OnDocumentOpen(Rml::Context* /*context*/, const Rml::String& /*document_path*/) {

}

void DataInput::OnDocumentLoad(Rml::ElementDocument* document) {
    documents.emplace_back(document);
}

void DataInput::OnDocumentUnload(Rml::ElementDocument* document) {
    std::erase(documents, document);
}

void DataInput::OnContextCreate(Rml::Context* /*context*/) {

}

void DataInput::OnContextDestroy(Rml::Context* /*context*/) {

}

void DataInput::OnElementCreate(Rml::Element* element) {
    if (!element) return;

    // TODO: Update data-if, so that we can actually hide/show the element at runtime!
    //       At the moment, the data-if is only evaluated at document creat, not during an update

    // Check if the element has a data-value
    auto const dataAttributes = {
        "data-value",
        "data-if"
    };
    for (auto const& attribute : dataAttributes) {
        auto const rmlValue = element->GetAttribute(attribute);
        if (!rmlValue) continue;
        if (!element->HasAttribute("data-identifier")) {
            capture.warning.println("A unique identifier is required for data inputs to work. Please provide 'data-identifier'.");
            continue;
        }

        // TODO: use attribute data-context. Fallback to global if not available
        // Still, we need a document to owner map, so we can dynamically set context. Should be enough to use the callerScope as self/other

        if (rmlValue->GetType() == Rml::Variant::STRING) {
            // Normalize value
            auto const attributeValue = std::string(rmlValue->Get<Rml::String>());

            // Backup value if needed
            std::string const backupAttributeKey = attribute + std::string("_backup");
            if (!element->GetAttribute(backupAttributeKey)) {
                // Normalize attribute and store backup
                element->SetAttribute(backupAttributeKey, attributeValue);
            }
            if (!element->GetAttribute(attribute)) {
                capture.warning.println("Failed to create backup attribute for data input: ", attributeValue);
                continue;
            }

            auto const unnormalizedId = element->GetAttribute("data-identifier");
            auto const unnormalizedKey = element->GetAttribute(backupAttributeKey);
            if (!unnormalizedId || !unnormalizedKey) {
                capture.warning.println("Failed to normalize data input.");
                continue;
            }

            std::string normalized = "ID__" + normalize(unnormalizedId->Get<Rml::String>()) + "__VALUE__" + normalize(unnormalizedKey->Get<Rml::String>());
            element->SetAttribute(attribute, normalized);
            if (registeredStrings.contains(normalized)) {
                continue;
            }

            // Create entry
            Data::ScopedKey const key{unnormalizedKey->Get<Rml::String>()};
            auto const value = global.get<std::string>(key).value_or("");
            auto entry = std::make_unique<RegisteredEntry>();
            entry->normalizedValue = normalized;
            entry->attribute = attribute;
            entry->key = key;
            entry->element = element;
            entry->currentRmlValue = value;
            entry->previousRmlValue = value;
            entry->previousDocumentValue = value;
            entry->isNewEntry = true;

            // Add Entry
            renderer.getDataModelConstructor().Bind(normalized, &entry->currentRmlValue);
            registeredStrings.emplace(normalized, std::move(entry));
            //capture.log.println("Registered data input with key: ", normalized);
        }
    }
}

void DataInput::OnElementDestroy(Rml::Element* /*element*/) {

}

//----------------------------------------------

void DataInput::updateDataValues() {
    for (auto const& entry : registeredStrings | std::views::values) {
        // Determine data flow...
        auto& currentRml = entry->currentRmlValue;
        auto const& previousRml = entry->previousRmlValue;
        auto currentDocument = global.get<std::string>(entry->key).value_or("");
        auto const& previousDocument = entry->previousDocumentValue;

        // Check if entry is new
        if (entry->isNewEntry) {
            entry->isNewEntry = false;
            entry->element->SetAttribute(entry->attribute, entry->normalizedValue);
            entry->element->SetAttribute("value", currentDocument);
            entry->element->GetOwnerDocument()->UpdateDocument();
            continue;
        }

        // 1.) rml -> document
        if (currentRml != previousRml) {
            global.set<std::string>(entry->key, currentRml);
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

std::string DataInput::normalize(std::string const& key) {
    auto view = key
        | std::views::transform([](char const& c) -> std::string {
            if (std::isalnum(c)) return std::string(1, c);
            if (c == '_')  return std::string("_");
            return "__" + std::to_string(c) + "__";
        })
        | std::views::join;

    return std::ranges::to<std::string>(view);
}

} // namespace Nebulite::Module::RmlUi
