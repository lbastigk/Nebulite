#ifndef MODULE_RMLUI_EVENTBRIDGE_HPP
#define MODULE_RMLUI_EVENTBRIDGE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/AttributeCommand.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {
class EventBridge final : public Base::RmlUiModule {
public:
    explicit EventBridge(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void processRmlUiEvent(SDL_Event const& event, int keyModifiers, Rml::Element* focusElement) override ;

    void OnElementDestroy(Rml::Element* element) override ;

    struct Attribute {
        struct OnDestroy : Interaction::AttributeCommand<"onDestroy"> {
            // Processing trigger during element deletion used to be buggy. If we ever notice an issue, store the DeletedElement and apply on next cycle
            static void processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, Rml::Element* element);
        };

        struct OnEnter : Interaction::AttributeCommand<"onEnter"> {
            static void processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, SDL_Event const& event, int keyModifiers, Rml::Element* focusElement);
        };

        // TODO: New Triggers:
        // - onHover
        // - onHoverEnter
        // - onHoverLeave
        // - onClick
        // - onDoubleClick
        // - onPress
        // - onRelease
        // - onFocus
        // - onFocusEnter
        // - onFocusLeave

        static bool hasSupportedAttribute(Rml::Element* element) {
            return OnDestroy::hasSupportedAttribute(element)
                || OnEnter::hasSupportedAttribute(element);
        }
    };

private:
    /**
     * @brief Available actions from Rml attribute commands
     */
    struct Actions {
        static_assert(Interaction::AttributeCommand<"">::specializationCount == 3, "If you added a new Rml attribute command specialization, make sure to add it to the Actions struct as well!");

        std::optional<std::string> rulesetLink = std::nullopt;
        std::optional<std::string> stringToParse = std::nullopt;
        std::optional<Interaction::SpecialAction::Type> specialAction = std::nullopt;

        static void applyRuleset(std::optional<std::string> const& rulesetLink, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope);

        static void parseString(std::optional<std::string> const& stringToParse, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope);

        static void applySpecialAction(std::optional<Interaction::SpecialAction::Type> const& action, Graphics::RmlInterface& manager, Utility::IO::Capture& capture, Rml::Element* element, Rml::ElementDocument* document);
    };

    /**
     * @brief A deleted element and its actions. Storing the entry for one cycle is required
     */
    struct DeletedElement {
        std::optional<Graphics::RmlInterface::RmlElementIdentifier> elementIdentifier = std::nullopt;

        Rml::ElementDocument* owner = nullptr;

        Actions actions;

        void apply(Graphics::RmlInterface& manager, Utility::IO::Capture& capture) const ;
    };
};
} // namespace Nebulite::Module::RmlUi

#endif // MODULE_RMLUI_EVENTBRIDGE_HPP
