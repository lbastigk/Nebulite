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

template <Data::OptionalFixedString trigger>
struct FullCommand {
    // NOLINTNEXTLINE
    constexpr FullCommand(std::string_view special) : specialization(special) {}

    std::string_view specialization;

    [[nodiscard]] std::string toString() const {
        return std::string(trigger.view()) + "-" + std::string(specialization);
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

template <Data::OptionalFixedString trigger>
struct AttributeCommand {

    static constexpr FullCommand<trigger> addCommand(std::string_view const& specialization) {
        return {specialization};
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
