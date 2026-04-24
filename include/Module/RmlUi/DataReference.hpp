#ifndef NEBULITE_MODULE_RMLUI_DATA_INPUT_HPP
#define NEBULITE_MODULE_RMLUI_DATA_INPUT_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_map.h"
#include <RmlUi/Core.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Utility/IO/Capture.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Module/Base/RmlUiModule.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class DataReference final : public Base::RmlUiModule {
public:

    explicit DataReference(Utility::IO::Capture& c, Core::Renderer& r);

    void update() override ;

    void postRenderUpdate() override;

    void OnInitialise() override ;

    void OnShutdown() override ;

    void OnDocumentOpen(Rml::Context* context, const Rml::String& document_path) override ;

    void OnDocumentLoad(Rml::ElementDocument* document) override ;

    void OnDocumentUnload(Rml::ElementDocument* document) override ;

    void OnContextCreate(Rml::Context* context) override ;

    void OnContextDestroy(Rml::Context* context) override ;

    void OnElementCreate(Rml::Element* element) override ;

    void OnElementDestroy(Rml::Element* element) override ;

private:

    // TODO: Add ability to dynamically retrieve data value from the scope of the element/parent element
    //       Issue: elements inside a data-reflect aren't scoped, so data-value=rml.input.animal is always relative to the documents scope
    //       Instead, we should perhaps use an element scope? Or is a separate keyword like data-value-scoped necessary?

    // TODO: Add context instead of always retrieving from global

    std::vector<Rml::ElementDocument*> documents;

    absl::flat_hash_map<
        Rml::Element*,
        Rml::String
    > rmlStrings;

    //--------------------------------

    struct RegisteredEntry {
        Data::ScopedKey key;
        std::string normalizedValue;
        Rml::Element* element = nullptr;
        Rml::String previousRmlValue;
        std::string previousDocumentValue;
        bool isNewEntry = true;
        Rml::String attribute;
    };

    std::vector<std::unique_ptr<RegisteredEntry>> registeredButWithoutId;

    absl::flat_hash_map<Graphics::RmlInterface::RmlElementIdentifier, std::unique_ptr<RegisteredEntry>> registeredEntries;

    absl::flat_hash_map<std::string, std::unique_ptr<Rml::String>> registeredStrings;

    std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    void updateDataValues();

    void normalizeDataValue(Rml::Element* element) ;

    void registerNewValues(Graphics::RmlInterface::RmlElementIdentifier const& id, Rml::Element const* element);

    void updateRegisteredValues(Graphics::RmlInterface::RmlElementIdentifier const& id, Rml::Element const* element);

    static std::string normalize(std::string const& key);

};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_DATA_INPUT_HPP
