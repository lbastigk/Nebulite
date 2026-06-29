#ifndef DATA_OPTIONALFIXEDSTRING_HPP
#define DATA_OPTIONALFIXEDSTRING_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstddef>
#include <string_view>

//------------------------------------------
namespace Nebulite::Data {
/**
 * @brief A template-parameter-friendly string, with nullopt-like capability
 * @tparam N Size of the string
 * @tparam forceOutsideDefinition Useful if you want to force the definition of the string outside the struct.
 *                                The idea is that, sometimes, we don't know the exact value at compile time,
 *                                so outer processes shall deduct the value based on what they know.
 */
template <std::size_t N, bool forceOutsideDefinition = false>
struct OptionalFixedString {
    std::array<char, N == 0 ? 1 : N> value{};

    consteval OptionalFixedString(char const (&str)[N == 0 ? 1 : N]) { // NOLINT
        static_assert(N > 0, "Use the default constructor for empty strings");
        static_assert(!forceOutsideDefinition, "Cannot initialize string from literal when forceOutsideDefinition is true");
        for (std::size_t i = 0; i < N; ++i) value[i] = str[i];
    }

    consteval OptionalFixedString() {
        static_assert(N == 0, "Default constructor can only be used for empty strings");
    }

    static constexpr std::size_t length() requires(N > 0 && !forceOutsideDefinition) {
        return N-1;
    }

    // Returns the last character of the string, or '\0' if the string is empty
    [[nodiscard]] constexpr char back() const requires(N > 0 && !forceOutsideDefinition) {
        if constexpr (N == 1) { // Empty string, return NULL instead
            return '\0';
        } else {
            return value[N - 2];
        }
    }

    [[nodiscard]] constexpr std::string_view view() const requires(N > 0 && !forceOutsideDefinition) {
        return {value.data(), N - 1};
    }

    static constexpr bool hasValue() {
        return N > 0 && !forceOutsideDefinition;
    }

    [[nodiscard]] static constexpr bool hasOutsideDefinition() {
        return forceOutsideDefinition;
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
