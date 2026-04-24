//------------------------------------------
// Includes

// Nebulite
#include "Module/RmlUi/DataInput.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

#include <complex>

//------------------------------------------

namespace {
auto constexpr dataAttributes = {
    "data-value",
    "data-if"       // TODO: data-if does not work ... Value is seen as "1", but it's somehow not updated correctly. Perhaps missing a document/element update?
                    //       previously this worked, as the data set was directly pulled from the scope. But now we initialize to an empty string, so it's false at start
};
} // namespace

namespace Nebulite::Module::RmlUi {


DataInput::DataInput(Utility::IO::Capture& c, Core::Renderer& r) : RmlUiModule(c,r) {
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
    // Normalize bound data value
    normalizeDataValue(element);
}

void DataInput::OnElementDestroy(Rml::Element* /*element*/) {

}

//----------------------------------------------

void DataInput::normalizeDataValue(Rml::Element* element) {
    for (auto const& attribute : dataAttributes) {
        auto const rmlValue = element->GetAttribute(attribute);
        if (!rmlValue) continue;
        if (!element->HasAttribute("data-identifier")) {
            capture.warning.println("A unique identifier is required for data inputs to work. Please provide 'data-identifier'.");
            continue;
        }
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

            // Create entry
            Data::ScopedKey const key{unnormalizedKey->Get<Rml::String>()};
            auto entry = std::make_unique<RegisteredEntry>();
            entry->normalizedValue = normalized;
            entry->attribute = attribute;
            entry->key = key;
            entry->element = element;
            entry->previousRmlValue = "";
            entry->previousDocumentValue = "";
            entry->isNewEntry = true;

            auto value = std::make_unique<Rml::String>();
            *value = "";

            // Add Entry
            renderer.getDataModelConstructor().Bind(normalized, value.get());
            registeredButWithoutId.emplace_back(std::move(entry));
            registeredStrings.emplace(normalized, std::move(value));
        }
    }
}

void DataInput::updateDataValues() {
    // TODO: Update data-if, so that we can actually hide/show the element at runtime!
    //       At the moment, the data-if is only evaluated at document creat, not during an update
    for (auto const& document : documents) {
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element const* element, Rml::Element* parent, size_t const& index) {
            Graphics::RmlInterface::RmlElementIdentifier const id(parent, index, element);
            registerNewValues(id, element);
            updateRegisteredValues(id, element);
        });
    }
}

void DataInput::registerNewValues(Graphics::RmlInterface::RmlElementIdentifier const& id, Rml::Element const* element){
    std::vector<std::unique_ptr<RegisteredEntry>> toAdd;
    if (auto const it = registeredEntries.find(id); it == registeredEntries.end()) {
        for (auto& noId : registeredButWithoutId) {
            if (noId->element == element) {
                // Push out of vector and into map
                toAdd.emplace_back(std::move(noId));
                registeredButWithoutId.erase(std::ranges::find(registeredButWithoutId, noId));
                break;
            }
        }
    }
    for (auto& entry : toAdd) {
        registeredEntries[id] = std::move(entry);
    }
}

void DataInput::updateRegisteredValues(Graphics::RmlInterface::RmlElementIdentifier const& id, Rml::Element const* element){
    if (!element) return;
    if (auto const it = registeredEntries.find(id); it != registeredEntries.end()){
        auto const idContext = renderer.getRmlElementContextAndScope(id);
        auto const docContext = renderer.getRmlDocumentContextAndScope(element->GetOwnerDocument());
        if (!idContext && !docContext) return;

        auto const& entry = it->second;
        auto& slf = idContext ? idContext.value().ctxScope.self : docContext.value().ctxScope.self;

        // Check if entry is new
        if (entry->isNewEntry) {
            entry->isNewEntry = false;
            entry->element->SetAttribute(entry->attribute, entry->normalizedValue);
            entry->element->SetAttribute("value", slf.get<std::string>(entry->key).value_or(""));
            entry->element->GetOwnerDocument()->UpdateDocument();
            return;
        }

        // Determine data flow...
        auto const registered = registeredStrings.find(entry->normalizedValue);
        if (registered == registeredStrings.end()) {
            return;
        }

        auto& currentRml = *registered->second;
        auto const& previousRml = entry->previousRmlValue;
        auto const currentDocument = slf.get<std::string>(entry->key).value_or("");
        auto const& previousDocument = entry->previousDocumentValue;

        // 1.) rml -> document
        if (currentRml != previousRml) {
            slf.set<std::string>(entry->key, currentRml);
            entry->previousRmlValue = currentRml;
            entry->previousDocumentValue = currentRml;
        }
        // 2.) document -> rml
        else if (currentDocument != previousDocument) {
            entry->element->SetAttribute("value", currentDocument);
            currentRml = currentDocument;
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
