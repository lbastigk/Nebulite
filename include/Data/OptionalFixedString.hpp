#ifndef DATA_OPTIONALFIXEDSTRING_HPP
#define DATA_OPTIONALFIXEDSTRING_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <string_view>

//------------------------------------------
namespace Nebulite::Data {
/**
 * @brief A template-parameter-friendly string, with nullopt-like capability
 * @tparam N Size of the string
 */
template <std::size_t N>
struct OptionalFixedString {
    std::array<char, N == 0 ? 1 : N> value{};

    // NOLINTNEXTLINE
    consteval OptionalFixedString(char const (&str)[N == 0 ? 1 : N]) {
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
        static_assert(N > 0, "No string given");
        return N-1;
    }

    // Returns the last character of the string, or '\0' if the string is empty
    [[nodiscard]] constexpr char back() const {
        static_assert(N > 0, "No string given");
        if constexpr (N == 1) { // Empty string, return NULL instead
            return '\0';
        } else {
            return value[N - 2];
        }
    }

    [[nodiscard]] constexpr std::string_view view() const {
        static_assert(N > 0, "No string given");
        return {value.data(), N - 1};
    }
};

// CTAD from string_view is difficult ...
// So we use char here
template <std::size_t N>
// NOLINTNEXTLINE
OptionalFixedString(char const(&)[N]) -> OptionalFixedString<N>;

template <std::size_t N>
OptionalFixedString(std::array<char, N>) -> OptionalFixedString<N>;

OptionalFixedString() -> OptionalFixedString<0>;

} // namespace Nebulite::Data
#endif // DATA_OPTIONALFIXEDSTRING_HPP
