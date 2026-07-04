#ifndef MODULE_RMLUI_CONDITIONAL_HPP
#define MODULE_RMLUI_CONDITIONAL_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core/StyleTypes.h>
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Interaction/Logic/Expression.hpp"
#include "Module/Base/RmlUiModule.hpp"

//------------------------------------------
// Forward declarations

namespace Rml {
class Element;
} // namespace Rml

namespace Nebulite::Graphics {
class RmlInterface;
} // namespace Nebulite::Graphics

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

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
        Rml::Element* element;
        Rml::Style::Display originalDisplay; // Original display style of the element
        Interaction::Logic::Expression condition; // Condition to determine if the element should be shown or hidden

        void resolve(Graphics::RmlInterface& interface) const ;

        void hideElement() const ;

        void showElement() const ;
    };

    absl::flat_hash_map<Rml::Element*, RegisteredEntry> registeredEntries;
};
} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_CONDITIONAL_HPP
