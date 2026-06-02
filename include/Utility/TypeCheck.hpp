#ifndef UTILITY_TYPECHECK_HPP
#define UTILITY_TYPECHECK_HPP

//------------------------------------------
// Includes

// Standard library
#include <expected>
#include <optional>
#include <type_traits>

//------------------------------------------
namespace Nebulite::Utility::TypeCheck {

template<typename>
struct is_std_optional : std::false_type {};

template<typename U>
struct is_std_optional<std::optional<U>> : std::true_type {};

template<typename T>
inline constexpr bool is_std_optional_v = is_std_optional<std::remove_cvref_t<T>>::value;

template<typename>
struct is_std_expected : std::false_type {};

template<typename T, typename E>
struct is_std_expected<std::expected<T, E>> : std::true_type {};

template<typename T>
inline constexpr bool is_std_expected_v = is_std_expected<std::remove_cvref_t<T>>::value;

} // namespace Nebulite::Utility::TypeCheck
#endif // UTILITY_TYPECHECK_HPP
