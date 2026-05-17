#ifndef MODULE_RMLUI_CONDITIONAL_HPP
#define MODULE_RMLUI_CONDITIONAL_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {

class Conditional final : public Base::RmlUiModule {
public:
    explicit Conditional(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void OnElementCreate(Rml::Element* element) override ;

    void OnElementDestroy(Rml::Element* element) override ;

    struct Attribute {
        static auto constexpr conditional = "if";

        static bool hasSupportedAttribute(Rml::Element const* element) {
            return element->HasAttribute(conditional);
        }
    };

private:
    struct RegisteredEntry {
        Interaction::Logic::Expression condition;
        std::string innerRml;
        Rml::Element* element;
        bool newEntry = true; // new means that we need to get the inner rml on next update

        void resolve(Graphics::RmlInterface::RmlElementIdentifier const& id, Graphics::RmlInterface& interface);
    };

    // TODO: this structure does not work for nested ifs! New entries might be registered while out ifs are resolved
    absl::flat_hash_map<Graphics::RmlInterface::RmlElementIdentifier, RegisteredEntry> registeredEntries;

    absl::flat_hash_map<Graphics::RmlInterface::RmlElementIdentifier, RegisteredEntry> newEntries;
};
} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_CONDITIONAL_HPP
