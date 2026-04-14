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

    // We cheat and set the context to full
    renderer.setRmlDocumentContextScope(document, {global, global, global});
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
    elementsToAdd.emplace_back(element);

    //auto const identifier = Graphics::RmlInterface::RmlElementIdentifier(parent, index, element);
    //auto const contextScope = renderer.getRmlElementContextScope(identifier);
    //if (!contextScope.has_value()) return;

    // TODO: Determine Document context:
    //auto const context = renderer.getRmlDocumentContext(document);
    //if (!context) return;
    Interaction::Context const context = {Global::instance(), Global::instance(), Global::instance()};

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

        auto const identifier = element->GetAttribute("data-identifier");
        if (!identifier) {
            capture.warning.println("A unique identifier is required for data inputs to work. Please provide 'data-identifier'.");
            continue;
        }

        // TODO: use attribute data-context. Fallback to global if not available
        // Still, we need a document to owner map, so we can dynamically set context. Should be enough to use the callerScope as self/other

        if (rmlValue->GetType() == Rml::Variant::STRING) {
            // Normalize value
            auto const attributeValue = std::string(rmlValue->Get<Rml::String>());
            std::string normalized;
            if (element->GetAttribute("data-was-normalized")) {
                normalized = attributeValue;
            }
            else {
                normalized = normalizeJsonKey("ID__" + identifier->Get<Rml::String>() + "__KEY__" + attributeValue);
                element->SetAttribute(attribute, normalized);
                element->SetAttribute("data-was-normalized", "true");
            }

            // Create entry
            Data::ScopedKey const key{attributeValue};
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

            if (!registeredStrings.contains(normalized)) {
                renderer.getDataModelConstructor().Bind(normalized, &entry->currentRmlValue);
                registeredStrings.emplace(normalized, std::move(entry));
                //capture.log.println("Registered data input with key: ", normalized);
            }
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

        // TODO: This almost works, but having multiple inputs of the same data-value seems to fail sometimes?
        // e.g., editing the first entry only changes the first two: Likely because they share the same pointer?
        /*
        <p>
            <input type="text" data-value="rml.input.animal"/>
        </p>
        <p data-reflect-once="{global.time|listMembersAndValues}">
            <p>
                <input type="text" data-value="rml.input.animal"/>
            </p>
        </p>
         */

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

std::string DataInput::normalizeJsonKey(std::string const& key) {
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
