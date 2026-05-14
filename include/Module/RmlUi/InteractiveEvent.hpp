#ifndef MODULE_RMLUI_INTERACTIVEEVENT_HPP
#define MODULE_RMLUI_INTERACTIVEEVENT_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Interaction/AttributeCommand.hpp"
#include "Module/Base/RmlUiModule.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::RmlUi {
class InteractiveEvent final : public Base::RmlUiModule {
public:
    explicit InteractiveEvent(Utility::IO::Capture& c, Graphics::RmlInterface& i);

    void update() override ;

    void processRmlUiEvent(SDL_Event const& event, int keyModifiers, Rml::Element* focusElement) override ;

    void OnElementDestroy(Rml::Element* element) override ;

    struct Attribute {
        struct OnDestroy : Interaction::AttributeCommand<Interaction::AttributeCommandTrigger::onDestroy> {
            static auto constexpr ruleset = addCommand("invoke-ruleset");
            static auto constexpr parse = addCommand("parse");
            static auto constexpr special = addCommand("special");

            static bool hasSupportedAttribute(Rml::Element* element) {
                return element->GetAttribute(ruleset.toString())
                    || element->GetAttribute(parse.toString())
                    || element->GetAttribute(special.toString());
            }
        };

        struct OnEnter : Interaction::AttributeCommand<Interaction::AttributeCommandTrigger::onEnter> {
            static auto constexpr ruleset = addCommand("invoke-ruleset");
            static auto constexpr parse = addCommand("parse");
            static auto constexpr special = addCommand("special");

            static bool hasSupportedAttribute(Rml::Element* element) {
                return element->GetAttribute(ruleset.toString())
                    || element->GetAttribute(parse.toString())
                    || element->GetAttribute(special.toString());
            }

            static void processEvent(Graphics::RmlInterface& manager, Utility::IO::Capture& capture, SDL_Event const& event, int keyModifiers, Rml::Element* focusElement);
        };

        static bool hasSupportedAttribute(Rml::Element* element) {
            return OnDestroy::hasSupportedAttribute(element)
                || OnEnter::hasSupportedAttribute(element);
        }
    };

private:

    enum class SpecialAction : uint8_t {
        deleteDocument,
        blurElement
    };

    static auto constexpr supported = {
        std::make_pair("deleteDocument", SpecialAction::deleteDocument),
        std::make_pair("blurElement", SpecialAction::blurElement)
    };

    static std::optional<SpecialAction> parseSpecialAction(std::string_view const& str);

    struct Actions {
        std::optional<std::string> rulesetLink = std::nullopt;
        std::optional<std::string> stringToParse = std::nullopt;
        std::optional<SpecialAction> specialAction = std::nullopt;

        static void applyRuleset(std::optional<std::string> const& rulesetLink, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope);

        static void parseString(std::optional<std::string> const& stringToParse, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope);

        static void applySpecialAction(std::optional<SpecialAction> const& action, Graphics::RmlInterface& manager, Rml::Element* element, Rml::ElementDocument* document);
    };

    struct DeletedElement {
        std::optional<Graphics::RmlInterface::RmlElementIdentifier> elementIdentifier = std::nullopt;

        Rml::ElementDocument* owner = nullptr;

        Actions actions;

        void apply(Utility::IO::Capture& capture, Graphics::RmlInterface& interface) const ;
    };

    std::vector<DeletedElement> interactiveEventsToApply;
};
} // namespace Nebulite::Module::RmlUi

#endif // MODULE_RMLUI_INTERACTIVEEVENT_HPP
