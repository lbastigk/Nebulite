#ifndef NEBULITE_MODULE_RMLUI_DATA_INPUT_HPP
#define NEBULITE_MODULE_RMLUI_DATA_INPUT_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include <absl/container/flat_hash_map.h>
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

    explicit DataReference(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void OnElementCreate(Rml::Element* element) override ;

    void OnElementDestroy(Rml::Element* element) override ;

private:
    absl::flat_hash_map<
        Rml::Element*,
        Rml::String
    > rmlStrings;

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
