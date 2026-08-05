#ifndef NEBULITE_UTILITY_TYPECHECK_HPP
#define NEBULITE_UTILITY_TYPECHECK_HPP

//------------------------------------------
// Includes

// Standard library
#include <expected>
#include <optional>
#include <type_traits>

//------------------------------------------
namespace Nebulite::Utility::TypeCheck {

template <typename T>
struct IsStaticMemberFunction : std::bool_constant<std::is_function_v<std::remove_pointer_t<T>>> {};

template <typename T>
inline constexpr bool isStaticMemberFunction = IsStaticMemberFunction<T>::value;

template<typename>
struct IsOptional : std::false_type {};

template<typename U>
struct IsOptional<std::optional<U>> : std::true_type {};

template<typename T>
inline constexpr bool isOptional = IsOptional<std::remove_cvref_t<T>>::value;

template<typename>
struct IsExpected : std::false_type {};

template<typename T, typename E>
struct IsExpected<std::expected<T, E>> : std::true_type {};

template<typename T>
inline constexpr bool isExpected = IsExpected<std::remove_cvref_t<T>>::value;

} // namespace Nebulite::Utility::TypeCheck
#endif // NEBULITE_UTILITY_TYPECHECK_HPP
