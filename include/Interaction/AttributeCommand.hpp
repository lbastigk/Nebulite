#ifndef INTERACTION_ATTRIBUTECOMMAND_HPP
#define INTERACTION_ATTRIBUTECOMMAND_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <utility>

//------------------------------------------
namespace Nebulite::Interaction {

enum class AttributeCommandTrigger : uint8_t{
    onDestroy, onEnter
};

struct FullCommand {
    // NOLINTNEXTLINE
    constexpr FullCommand(AttributeCommandTrigger trigger, std::string_view specialization) : command(trigger, specialization) {}

    std::pair<AttributeCommandTrigger, std::string_view> command;

    [[nodiscard]] std::string toString() const {
        auto stringify = [](AttributeCommandTrigger const& tr) -> std::string {
            switch (tr) {
            case AttributeCommandTrigger::onDestroy:
                return "on-destroy";
            case AttributeCommandTrigger::onEnter:
                return "on-enter";
            default:
                std::unreachable();
            }
        };
        auto const& [tr, specialization] = command;
        return stringify(tr) + "-" + std::string(specialization);
    }
};

struct SpecialAction {
    enum class Type : uint8_t {
        deleteDocument,
        blurElement
    };

    static auto constexpr supported = {
        std::make_pair("deleteDocument", Type::deleteDocument),
        std::make_pair("blurElement", Type::blurElement)
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

template<AttributeCommandTrigger trigger>
struct AttributeCommand {
    static constexpr FullCommand addCommand(std::string_view const& specialization) {
        return {trigger, specialization};
    }

    static auto constexpr ruleset = addCommand("invoke-ruleset");
    static auto constexpr parse = addCommand("parse");
    static auto constexpr special = addCommand("special");

    static bool hasSupportedAttribute(Rml::Element* element) {
        return element->GetAttribute(ruleset.toString())
            || element->GetAttribute(parse.toString())
            || element->GetAttribute(special.toString());
    }
};

} // namespace Nebulite::Interaction
#endif // INTERACTION_ATTRIBUTECOMMAND_HPP
