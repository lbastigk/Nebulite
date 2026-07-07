#ifndef NEBULITE_MODULE_RMLUI_EVENTBRIDGE_HPP
#define NEBULITE_MODULE_RMLUI_EVENTBRIDGE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <optional>

// External
#include <SDL3/SDL_events.h>

// Nebulite
#include "Nebulite/Interaction/AttributeCommand.hpp"
#include "Nebulite/Module/Base/RmlUiModule.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
// Forward declarations

namespace Rml {
class Element;
class ElementDocument;
} // namespace Rml

namespace Nebulite::Graphics {
class RmlInterface;
} // namespace Nebulite::Graphics

namespace Nebulite::Utility::IO {
class Capture;
} // namespace Nebulite::Utility::IO

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
            // Processing trigger during element deletion used to be buggy.
            // If we ever notice an issue, store the DeletedElement and apply on next cycle
            static void processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, Rml::Element* element);
        };

        struct OnEnter : Interaction::AttributeCommand<"onEnter"> {
            static void processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, SDL_Event const& event, int keyModifiers, Rml::Element* focusElement);
        };

        struct OnClick : Interaction::AttributeCommand<"onClick"> {
            static  void processTrigger(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, SDL_Event const& event, int keyModifiers, Rml::Element* focusElement);
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

        template<typename...>
        struct TypeList {};

        // Ensure all Attributes are present. Later on we might improve this using C++26 reflection
        using AttributeList = TypeList<
            OnDestroy,
            OnEnter,
            OnClick
        >;

        template<typename... Ts>
        struct ForEach;

        template<typename... Ts>
        struct ForEach<TypeList<Ts...>> {
            static bool hasSupportedAttribute(Rml::Element* element) {
                return (Ts::hasSupportedAttribute(element) || ...);
            }
        };

        static bool hasSupportedAttribute(Rml::Element* element) {
            return ForEach<AttributeList>::hasSupportedAttribute(element);
        }
    };

private:
    /**
     * @brief Struct for storing all relevant information for a triggered Rml event, as well as the actions to perform.
     * @details Templated to allow for any type of Trigger. The full Document Attribute to check is build from that type
     */
    template<typename TriggerType>
    struct BridgeEntry {
        explicit BridgeEntry(Rml::Element* element);

        std::optional<Graphics::RmlInterface::RmlElementIdentifier> elementIdentifier = std::nullopt;

        Rml::ElementDocument* owner = nullptr;

        Interaction::Actions<TriggerType> actions;

        void apply(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, Rml::Element* element) const ;
    };
};

template<typename TriggerType>
EventBridge::BridgeEntry<TriggerType>::BridgeEntry(Rml::Element* element) : actions(element) {
    assert(element);
    if (Graphics::RmlInterface::RmlElementIdentifier::hasElementIdentifier(element)) {
        elementIdentifier = Graphics::RmlInterface::RmlElementIdentifier(element);
    }
    owner = element ? element->GetOwnerDocument() : nullptr;
}

template<typename TriggerType>
void EventBridge::BridgeEntry<TriggerType>::apply(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, Rml::Element* element) const {
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
        // For some reason, Entries are executed twice if we delete a domain with an open document.
        // This catches the second call.
        return;
    }
    actions.apply(manager, capture, ctxAndScope.value(), element, owner);
}

} // namespace Nebulite::Module::RmlUi
#endif // NEBULITE_MODULE_RMLUI_EVENTBRIDGE_HPP
