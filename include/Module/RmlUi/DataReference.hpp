#ifndef NEBULITE_MODULE_RMLUI_DATA_INPUT_HPP
#define NEBULITE_MODULE_RMLUI_DATA_INPUT_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include <RmlUi/Core.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class DataReference final : public Base::RmlUiModule {
public:
    explicit DataReference(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void OnElementCreate(Rml::Element* element) override ;

    void OnElementDestroy(Rml::Element* element) override ;

    static auto constexpr dataValueAttribute = "data-value";
    static auto constexpr dataIfAttribute = "data-if";

private:
    absl::flat_hash_map<
        Rml::Element*,
        Rml::String
    > rmlStrings;

    struct RegisteredEntry {
        Interaction::ContextDeriver::TargetType targetType = Interaction::ContextDeriver::TargetType::none;
        Data::ScopedKey key;
        std::string normalizedValue;
        Rml::Element* element = nullptr;
        Rml::String attribute;
        Rml::String previousRmlValue;
        std::string previousDocumentValue;
        bool isNewEntry = true;
        std::optional<std::string> innerRml; // Only set if element has data-if-attribute
    };

    absl::flat_hash_map<Graphics::RmlInterface::RmlElementIdentifier, std::unique_ptr<RegisteredEntry>> registeredEntries;

    absl::flat_hash_map<std::string, std::unique_ptr<Rml::String>> registeredStrings;

    std::unique_ptr<Utility::Coordination::TimedRoutine> evaluationRoutine;

    void updateDataValues();

    void registerDataValue(Rml::Element* element) ;

    void updateRegisteredValues(Graphics::RmlInterface::RmlElementIdentifier const& id, Rml::Element* element);

    void synchronizeEntry(std::unique_ptr<RegisteredEntry> const& entry, Rml::Element* element, Data::JsonScope& target);

    static std::string normalize(std::string const& key);
};
} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_DATA_INPUT_HPP
