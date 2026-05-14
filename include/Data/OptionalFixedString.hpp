#ifndef DATA_OPTIONALFIXEDSTRING_HPP
#define DATA_OPTIONALFIXEDSTRING_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <string>
#include <string_view>

//------------------------------------------
namespace Nebulite::Data {
/**
 * @brief A template-parameter-friendly string, with nullopt-like capability
 * @tparam N Size of the string
 */
template <std::size_t N>
struct OptionalFixedString {
    char value[N == 0 ? 1 : N]{};
    //std::array<char, N == 0 ? 1 : N> value{}; // TODO: use this instead

    // NOLINTNEXTLINE
    consteval OptionalFixedString(const char (&str)[N == 0 ? 1 : N]) {
        static_assert(N > 0, "Use the default constructor for empty strings");
        for (std::size_t i = 0; i < N; ++i) value[i] = str[i];
    }

    consteval OptionalFixedString() {
        static_assert(N == 0, "Default constructor can only be used for empty strings");
    }

    static constexpr bool hasValue() {
        return N > 0;
    }

    static constexpr size_t length() {
        return N-1;
    }

    [[nodiscard]] constexpr char back() const {
        if constexpr (N == 0 || N == 1) { // Nullopt or empty string
            return '\0';
        } else {
            return value[N - 2];
        }
    }

    [[nodiscard]] constexpr std::string_view view() const {
        if constexpr (N > 0) return {value, N - 1};
        else return {};
    }
};

template <std::size_t N>
OptionalFixedString(const char (&)[N]) -> OptionalFixedString<N>;

template <std::size_t N>
OptionalFixedString(std::array<char, N>) -> OptionalFixedString<N>;

OptionalFixedString() -> OptionalFixedString<0>;

} // namespace Nebulite::Data
#endif // DATA_OPTIONALFIXEDSTRING_HPP
