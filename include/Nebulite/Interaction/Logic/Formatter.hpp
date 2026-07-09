#ifndef NEBULITE_INTERACTION_LOGIC_FORMATTER_HPP
#define NEBULITE_INTERACTION_LOGIC_FORMATTER_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

//------------------------------------------
namespace Nebulite::Interaction::Logic {
/**
 * @struct Nebulite::Interaction::Logic::Formatter
 * @brief Represents formatting options for the component.
 */
struct Formatter {
    std::optional<std::uint8_t> alignment = std::nullopt;
    std::optional<std::uint8_t> precision = std::nullopt;

    /**
     * @enum Nebulite::Interaction::Logic::Formatter::CastType
     * @brief Represents the type of cast to apply to an expression component.
     */
    enum class CastType : std::uint8_t {
        none, // No cast -> using pure string
        to_int, // Cast to integer
        to_double // Cast to double
    } cast = CastType::none; // Default to none

    bool leadingZero = false; // If true, pad with leading zeros

    /**
     * @brief Parses the formatter string
     * @param formatter The formatter string to parse.
     * @return a formatter, or nullopt
     */
    static Formatter readFormatter(std::string_view formatter);

    [[nodiscard]] std::string format(double value) const ;
};
} // namespace Nebulite::Interaction::Logic
#endif // NEBULITE_INTERACTION_LOGIC_FORMATTER_HPP
