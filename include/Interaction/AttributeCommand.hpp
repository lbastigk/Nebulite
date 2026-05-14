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

template<AttributeCommandTrigger trigger>
struct AttributeCommand {
    static constexpr FullCommand addCommand(std::string_view const& specialization) {
        return {trigger, specialization};
    }
};

} // namespace Nebulite::Interaction
#endif // INTERACTION_ATTRIBUTECOMMAND_HPP
