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
/**
 * @brief Adds the conditional "if"-attribute to elements
 * @details Compared to data-if, this offers a more lightweight and direct integration into nebulites data system.
 */
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
        Rml::Style::Display originalDisplay;
        Interaction::Logic::Expression condition;
        Rml::Element* element;
        bool newEntry = true; // new means that we need to get the inner rml on next update

        void resolve(Graphics::RmlInterface::RmlElementIdentifier const& id, Graphics::RmlInterface& interface);

        void hideElement() const ;

        void showElement() const ;
    };

    absl::flat_hash_map<Graphics::RmlInterface::RmlElementIdentifier, RegisteredEntry> registeredEntries;
};
} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_CONDITIONAL_HPP
