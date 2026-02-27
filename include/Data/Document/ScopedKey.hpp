/**
 * @file ScopedKey.hpp
 * @brief Defines the ScopedKey and ScopedKeyView classes for managing scoped keys within JSON documents.
 * @details The purpose is to ensure subclasses only access keys within their intended scopes.
 *          Furthermore, it allows for root-scoped keys that are always taken at given scope root.
 *          This allows us to have structured JSON documents representing Multiple Domain Classes:
 *          RenderObject with its subclass Texture, where Texture keys are always under "texture." scope.
 *          We can then, in both classes, use a root-scoped key for "name"
 *          that either refers to "name" at root (RenderObject) or "texture.name" (Texture), depending on the current scope.
 *          We can also have scoped keys that fail if used outside their intended scope, e.g. a key
 *          defined with scope "renderer." cannot be used in a JsonScope with scope "physics.",
 *          as that would be an accidental misuse of the key.
 *          The checks happen at runtime when the key is used to access the JsonScopeBase,
 *          throwing an exception if the scopes do not match.
 *          This allows for greater separation of concerns and reduces accidental key misusage.
 */

#ifndef NEBULITE_DATA_DOCUMENT_SCOPED_KEY_HPP
#define NEBULITE_DATA_DOCUMENT_SCOPED_KEY_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <string_view>
#include <optional>

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class JsonScope;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class ScopedKey;
class ScopedKeyView;
class JsonScopeBase;
} // namespace Nebulite::Data

//------------------------------------------
// Helper struct for compile-time optional fixed strings as template parameters

template <std::size_t N>
struct OptionalFixedString {
    char value[N == 0 ? 1 : N]{};

    // NOLINTNEXTLINE
    consteval OptionalFixedString(const char (&str)[N == 0 ? 1 : N]) {
        static_assert(N > 0, "Use the default constructor for empty strings");
        for (std::size_t i = 0; i < N; ++i) value[i] = str[i];
    }

    consteval OptionalFixedString() {
        static_assert(N == 0, "Default constructor can only be used for empty strings");
    }

    static constexpr bool has_scope = N > 0;
    constexpr std::string_view view() {
        if constexpr (N > 0) return {value, N - 1};
        else return {};
    }
};

template <std::size_t N>
OptionalFixedString(const char (&)[N]) -> OptionalFixedString<N>;

//------------------------------------------
// Self-owning scoped key
namespace Nebulite::Data {
/**
 * @class ScopedKey
 * @brief An owning version of ScopedKeyView that holds its own string buffer.
 * @details This class is useful for constructing scoped keys at runtime,
 *          where the key string needs to be built dynamically.
 *          It owns the string buffer, ensuring that the data remains valid
 *          for the lifetime of the ScopedKey instance.
 */
class ScopedKey {
    /**
     * @brief Optional scope prefix for this key.
     * @details The required scope prefix that this key must be used within.
     *          If set, any JsonScopeBase using this key must have a scope
     *          that matches or is a sub-scope of this prefix.
     *          If not set, the key is assumed to be at the root scope.
     */
    std::optional<std::string_view> givenScope = std::nullopt;

    /**
     * @brief The owned string buffer containing the full key.
     */
    std::string owned;

public:
    ScopedKey() = default;

    // construct from a view + suffix (runtime)
    ScopedKey(ScopedKeyView const& base, std::string_view const& suffix);

    // direct construction
    explicit ScopedKey(std::string s, std::optional<std::string_view> const& scope = std::nullopt);

    // produce a scopedKey view that points into this owned buffer.
    // caller must keep the ScopedKeyView alive while using the returned view.
    [[nodiscard]] ScopedKeyView view() const & noexcept ;

    // convenience: implicit conversion to scopedKey view (only valid while *this is alive)
    //operator ScopedKeyView() const & noexcept ;

    // Add operator for appending suffixes
    [[nodiscard]] ScopedKey operator+(std::string_view const& suffix) const ;
};
} // namespace Nebulite::Data


//------------------------------------------
// Compile-time friendly scoped key
namespace Nebulite::Data {
/**
 * @class ScopedKeyView
 * @brief Non-Owning String wrapper to represent keys within a JsonScopeBase.
 * @details This class allows for easy conversion of string literals
 *          into fully scoped keys within the JsonScopeBase.
 *          This reduces accidental key misusage, as conversion to a usable type
 *          std::string requires an explicit action.
 *          It also provides safety checks to ensure that keys are used within their intended scopes.
 *          We can use this to generate static scoped keys in DomainModules, ensuring that
 *          they are always used in the correct scope.
 */
class ScopedKeyView {
    /**
     * @brief Produce the full key string including scope prefix.
     * @param scope The current scope of the JsonScopeBase.
     * @return The full key string with scope prefix.
     * @throws std::invalid_argument if the key is used outside its required scope.
     */
    [[nodiscard]] std::string full(JsonScopeBase const& scope) const;

    /**
     * @brief Optional scope prefix for this key.
     * @details The required scope prefix that this key must be used within.
     *          If set, any JsonScopeBase using this key must have a scope
     *          that matches or is a sub-scope of this prefix.
     *          If not set, the key is assumed to be at the root scope.
     */
    // NOLINTNEXTLINE
    std::optional<std::string_view> givenScope = std::nullopt;
    // Ignored due to warnings with the compile-time construction: Usage of non-initialized class field 'givenScope' when called from function 'create<RequiredScope, T>'

    /**
     * @brief The key string within the scope.
     */
    // NOLINTNEXTLINE
    std::string_view key;
    // Ignored due to warnings with the compile-time construction: Usage of non-initialized class field 'key' when called from function 'create<RequiredScope, T>'

    // Any key shared publicly should be constructed with a required scope to avoid accidental misuse
    constexpr ScopedKeyView(std::optional<std::string_view> const& requiredScope, std::string_view const& keyInScope) noexcept
        : givenScope(requiredScope), key(keyInScope) {}

    // allow the owning type to construct views pointing into its buffer
    friend class ScopedKey;

public:
    friend class JsonScopeBase;
    friend class Core::JsonScope;

    // Adding suffix to produce a new ScopedKeyView
    [[nodiscard]] ScopedKey operator+(std::string_view const& suffix) const ;

    // No scope given, expected at root of JsonScopeBase
    // Making this explicit would help avoid accidental misuse of keys,
    // But this would require a lot of code changes in existing codebases.
    // Instead, we assume every string literal used is assumed to be without given scope.
    //template<typename T, typename = std::enable_if_t<std::is_constructible_v<std::string_view, T>>>
    //constexpr ScopedKeyView(T const& keyInScope) noexcept
    //    : key(std::string_view(keyInScope)) {}

    explicit constexpr ScopedKeyView(std::string_view const& keyInScope)
        : key(keyInScope) {}

    /**
     * @brief Create a ScopedKeyView from a compile-time fixed string with an optional scope prefix.
     * @tparam RequiredScope A compile-time fixed string representing the required scope prefix for this key.
     *                       If empty, the keys scope is arbitrary and assumed to be at the root of the JsonScopeBase.
     * @tparam T A type that can be converted to std::string_view, representing the key string within the scope.
     * @param keyInScope The key string within the scope, which can be a string literal or any type convertible to std::string_view.
     * @return A ScopedKeyView instance with the appropriate scope and key string.
     */
    template <OptionalFixedString RequiredScope, typename T>
    static consteval ScopedKeyView createFromOptionalFixedString(T const& keyInScope) noexcept {
        if constexpr (!RequiredScope.has_scope) {
            // no scope provided
            return {std::nullopt, std::string_view(keyInScope)};
        } else {
            // compute length at compile time
            constexpr const char *s = RequiredScope.value;
            constexpr std::size_t len = [] (const char *p) constexpr {
                std::size_t i = 0;
                while (p[i] != '\0') ++i;
                return i;
            }(s);

            // static assert to ensure scope is valid
            static_assert(len == 0 || s[len - 1] == '.', "ScopedKey: The provided scope must be empty or end with a dot ('.')");

            return {std::optional(std::string_view(s, len)), std::string_view(keyInScope)};
        }
    }

    std::string toString() const {
        if (givenScope.has_value()) {
            return std::string(givenScope.value()) + std::string(key);
        }
        return std::string(key);
    }

    ScopedKey toScopedKey() const {
        if (givenScope.has_value()) {
            return ScopedKey(std::string(givenScope.value()) + std::string(key), givenScope);
        }
        return ScopedKey(std::string(key), std::nullopt);
    }
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_SCOPED_KEY_HPP
