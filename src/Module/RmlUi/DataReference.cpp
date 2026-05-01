//------------------------------------------
// Includes

// Nebulite
#include "Module/RmlUi/DataReference.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

DataReference::DataReference(Utility::IO::Capture& c, Graphics::RmlInterface& i) : RmlUiModule(c,i) {
    evaluationRoutine = std::make_unique<Utility::Coordination::TimedRoutine>(
        [this] {
            updateDataValues();
        },
        10, // ms
        Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    );
}

void DataReference::update() {
    evaluationRoutine->update();
}

void DataReference::OnElementCreate(Rml::Element* element) {
    // Normalize bound data value
    registerDataValue(element);
}

void DataReference::OnElementDestroy(Rml::Element* element) {
    Graphics::RmlInterface::RmlElementIdentifier const id(element);
    registeredEntries.erase(id);
}

//----------------------------------------------

void DataReference::registerDataValue(Rml::Element* element) {
    for (auto const& attribute : {dataValueAttribute, dataIfAttribute}) {
        std::string backupAttribute = "__backup__" + std::string(attribute);
        auto const rmlValue = element->GetAttribute(attribute);
        auto const rmlBackupValue = element->GetAttribute(backupAttribute);
        if (!rmlValue) continue;
        if (rmlValue->GetType() == Rml::Variant::STRING) {
            // Get either from backup or directly
            auto attributeValue = std::string(rmlValue->Get<Rml::String>());
            if (!rmlBackupValue) {
                element->SetAttribute(backupAttribute, attributeValue);
            }
            else {
                attributeValue = rmlBackupValue->Get<Rml::String>();
            }

            // Normalize value + unique element id
            auto id = Graphics::RmlInterface::RmlElementIdentifier(element);
            std::string const normalized = "ID__" + std::to_string(id.getId()) + "__VALUE__" + normalize(attributeValue);
            element->SetAttribute(attribute, normalized);

            // Create entry + value
            auto entry = std::make_unique<RegisteredEntry>();
            entry->targetType = Interaction::ContextDeriver::getTypeFromString(attributeValue);
            entry->key = Data::ScopedKey(Interaction::ContextDeriver::stripContext(attributeValue));
            entry->normalizedValue = normalized;
            entry->element = element;
            entry->attribute = attribute;
            entry->previousRmlValue = "";
            entry->previousDocumentValue = "";
            entry->isNewEntry = true;
            entry->innerRml = "";
            auto value = std::make_unique<Rml::String>();
            *value = "";

            // Remove data-if attribute
            if (std::string(attribute) == dataIfAttribute) {
                entry->innerRml = "";
                element->RemoveAttribute(attribute);
            }

            // Add Entry
            registeredEntries.emplace(id, std::move(entry));
            if (registeredStrings.find(normalized) == registeredStrings.end()) {
                interface.dataModelConstructor.Bind(normalized, value.get());
                registeredStrings.emplace(normalized, std::move(value));
            }
        }
    }
}

void DataReference::updateDataValues() {
    for (auto const& document : interface.getOpenedDocuments()) {
        Graphics::RmlInterface::updateElement(document, [&](Rml::Element* element, Rml::Element* /*parent*/) {
            Graphics::RmlInterface::RmlElementIdentifier const id(element);
            updateRegisteredValues(id, element);
        });
    }
}

void DataReference::updateRegisteredValues(Graphics::RmlInterface::RmlElementIdentifier const& id, Rml::Element* element){
    if (!element) return;

    auto const idContext = interface.getRmlElementContextAndScope(id);
    auto const docContext = interface.getRmlDocumentContextAndScope(element->GetOwnerDocument());
    if (!idContext && !docContext) return;

    auto& ctxScope = idContext ? idContext.value().ctxScope : docContext.value().ctxScope;
    if (ctxScope.hasDummyScope()) {
        capture.warning.println("Failed to update data reference, a context member has a dummy scope!");
        registeredEntries.erase(id);
        return;
    }

    // Update all registered entries
    if (auto const it = registeredEntries.find(id); it != registeredEntries.end()){
        auto const& entry = it->second;

        auto const targetCheck = ctxScope.getTargetFromType(entry->targetType);
        if (!targetCheck) {
            capture.warning.println("Failed to update data reference, target is not available in context!");
            return;
        }
        auto& target = targetCheck.value().get();

        // Check if entry is new, set necessary attributes from context
        if (entry->isNewEntry) {
            entry->isNewEntry = false;
            entry->element->SetAttribute("value", target.get<std::string>(entry->key).value_or(""));
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
        auto const currentDocument = target.get<std::string>(entry->key).value_or("");
        auto const& previousDocument = entry->previousDocumentValue;

        // 1.) rml -> document
        if (currentRml != previousRml) {
            target.set<std::string>(entry->key, currentRml);
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

        // 3.) data-if attribute
        if (entry->innerRml.has_value() && entry->innerRml->empty()) {
            entry->innerRml = element->GetInnerRML();
            if (!entry->innerRml->empty()) {
                element->SetInnerRML("");
            }
        }
        if (entry->innerRml.has_value() && !entry->innerRml->empty()) {
            if (bool const show = target.get<bool>(entry->key).value_or(false); show && element->GetInnerRML().empty()) {
                element->SetInnerRML(entry->innerRml.value());
                element->GetOwnerDocument()->UpdateDocument();
            }
            else if (!show && !element->GetInnerRML().empty()) {
                element->SetInnerRML("");
                element->GetOwnerDocument()->UpdateDocument();
            }
        }

    }
}

std::string DataReference::normalize(std::string const& key) {
    auto view = key
        | std::views::transform([](char const& c) -> std::string {
            // NOLINTNEXTLINE
            if (std::isalnum(c)) return std::string(1, c);
            if (c == '_')  return "_";
            return "__" + std::to_string(c) + "__"; // Replace non-alphanumeric characters with their ASCII value
        })
        | std::views::join;

    return std::ranges::to<std::string>(view);
}

} // namespace Nebulite::Module::RmlUi
