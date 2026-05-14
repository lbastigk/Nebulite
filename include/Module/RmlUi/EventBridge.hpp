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
     * @brief Struct for storing all relevant information for a triggered Rml event, as well as the actions to perform.
     * @details Templated to allow for any type of Trigger. The full Document Attribute to check is build from that type
     */
    template<typename>
    struct BridgeEntry {
        explicit BridgeEntry(Rml::Element* element);

        std::optional<Graphics::RmlInterface::RmlElementIdentifier> elementIdentifier = std::nullopt;

        Rml::ElementDocument* owner = nullptr;

        Actions actions;

        void apply(Graphics::RmlInterface& manager, Utility::IO::Capture& capture) const ;
    };
};

template<typename TriggerType>
EventBridge::BridgeEntry<TriggerType>::BridgeEntry(Rml::Element* element){
    assert(element);
    if (Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(element)) {
        elementIdentifier = Graphics::RmlInterface::RmlElementIdentifier(element);
    }
    owner = element ? element->GetOwnerDocument() : nullptr;

    // Process
    static_assert(Interaction::AttributeCommand<"">::specializationCount == 3, "If you added a new Rml attribute command specialization, make sure to add it to the trigger processing!");
    if (auto const* var = element->GetAttribute(TriggerType::ruleset.toString()); var) {
        actions.rulesetLink = var->template Get<Rml::String>();
    }
    if (auto const* val = element->GetAttribute(TriggerType::parse.toString()); val) {
        actions.stringToParse = val->template Get<Rml::String>();
    }
    if (auto const* val = element->GetAttribute(TriggerType::special.toString()); val) {
        actions.specialAction = Interaction::SpecialAction::get(val->template Get<Rml::String>());
    }
}

template<typename TriggerType>
void EventBridge::BridgeEntry<TriggerType>::apply(Graphics::RmlInterface& manager, Utility::IO::Capture& capture) const {
    auto ctxAndScope = [&] -> std::optional<Graphics::RmlInterface::ContextAndScope> {
        if (elementIdentifier.has_value()) {
            return manager.getRmlElementContextAndScope(elementIdentifier.value());
        }
        if (owner) {
            return manager.getRmlDocumentContextAndScope(owner);
        }
        return std::nullopt;
    }();
    if (!ctxAndScope) {
        // For some reason, Entries are executed twice if we delete a domain with an open document. This catches the second call.
        return;
    }

    Actions::applyRuleset(actions.rulesetLink, capture, ctxAndScope.value());
    Actions::parseString(actions.stringToParse, capture, ctxAndScope.value());
    Actions::applySpecialAction(actions.specialAction, manager, capture, nullptr, owner);
}

} // namespace Nebulite::Module::RmlUi
#endif // MODULE_RMLUI_EVENTBRIDGE_HPP
