#ifndef INTERACTION_ATTRIBUTECOMMAND_HPP
#define INTERACTION_ATTRIBUTECOMMAND_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <utility>

// Nebulite
#include "Data/OptionalFixedString.hpp"

//------------------------------------------
namespace Nebulite::Interaction {

struct SpecialAction {
    enum class Type : uint8_t {
        blurElement,
        deleteDocument
    };

    static auto constexpr supported = {
        std::make_pair("blurElement", Type::blurElement),
        std::make_pair("deleteDocument", Type::deleteDocument)
    };

    static std::optional<Type> get(std::string_view const& str){
        for (auto const& [name, action] : supported) {
            if (str == name) {
                return action;
            }
        }
        return std::nullopt;
    }
};

class ActionsImpl {
protected:
    /**
     * @brief Applies a special action
     * @param ruleset The ruleset to apply. Either a link or a static ruleset.
     * @param cap The capture for logging
     * @param ctxAndScope The context and scope for executing the parsed string
     */
    static void applyRuleset(std::string_view const& ruleset, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope);

    /**
     * @brief Applies a special action
     * @param stringToParse The string to parse and execute
     * @param cap The capture for logging
     * @param ctxAndScope The context and scope for executing the parsed string
     */
    static void parseString(std::string_view const& stringToParse, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope);

    /**
     * @brief Applies a special action
     * @param action The action to apply
     * @param manager The RmlInterface for context management
     * @param element The active element
     * @param document The owner document
     */
    static void applySpecialAction(SpecialAction::Type const& action, Graphics::RmlInterface& manager, Rml::Element* element, Rml::ElementDocument* document);
};

/**
 * @brief Available Actions performable on a Trigger
 */
template<typename TriggerType>
class Actions : ActionsImpl {
public:
    explicit Actions(Rml::Element* element) {
        if (auto const* var = element->GetAttribute(TriggerType::Specialization::ruleset.toString()); var) {
            rulesetLink = var->template Get<Rml::String>();
        }
        if (auto const* val = element->GetAttribute(TriggerType::Specialization::parse.toString()); val) {
            stringToParse = val->template Get<Rml::String>();
        }
        if (auto const* val = element->GetAttribute(TriggerType::Specialization::special.toString()); val) {
            specialAction = SpecialAction::get(val->template Get<Rml::String>());
        }
    }

    void apply(Graphics::RmlInterface& manager, Utility::IO::Capture& cap, Graphics::RmlInterface::ContextAndScope& ctxAndScope, Rml::Element* element, Rml::ElementDocument* document) const {
        if (rulesetLink) applyRuleset(rulesetLink.value(), cap, ctxAndScope);
        if (stringToParse) parseString(stringToParse.value(), cap, ctxAndScope);
        if (specialAction) applySpecialAction(specialAction.value(), manager, element, document);
    }

private:
    std::optional<std::string> rulesetLink = std::nullopt;
    std::optional<std::string> stringToParse = std::nullopt;
    std::optional<SpecialAction::Type> specialAction = std::nullopt; // TODO: support multiple special actions using comma separation
};

template <Data::OptionalFixedString trigger>
struct FullCommand {
    // NOLINTNEXTLINE
    constexpr FullCommand(std::string_view special) : specialization(special) {}

    std::string_view specialization;

    [[nodiscard]] std::string toString() const {
        return std::string(trigger.view()) + "#" + std::string(specialization);
    }
};

template <Data::OptionalFixedString trigger>
class AttributeCommand {
    static consteval FullCommand<trigger> addCommand(std::string_view const& specialization) {
        return {specialization};
    }
public:
    // Specializations
    static auto constexpr specializationCount = 3;

    struct Specialization {
        static auto constexpr ruleset = addCommand("invokeRuleset");
        static auto constexpr parse = addCommand("parse");
        static auto constexpr special = addCommand("special");
    };

    static bool hasSupportedAttribute(Rml::Element* element) {
        return element->GetAttribute(Specialization::ruleset.toString())
            || element->GetAttribute(Specialization::parse.toString())
            || element->GetAttribute(Specialization::special.toString());
    }
};

} // namespace Nebulite::Interaction
#endif // INTERACTION_ATTRIBUTECOMMAND_HPP
