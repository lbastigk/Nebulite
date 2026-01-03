/**
 * @file ScopedKey.hpp
 * @brief Defines the ScopedKey and OwnedScopedKey classes for managing scoped keys within JSON documents.
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
#include <type_traits>

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class ScopedKey;
class OwnedScopedKey;
class JsonScopeBase;
} // namespace Nebulite::Data

//------------------------------------------
// Self-owning scoped key
namespace Nebulite::Data {
/**
 * @brief An owning version of scopedKey that holds its own string buffer.
 * @details This class is useful for constructing scoped keys at runtime,
 *          where the key string needs to be built dynamically.
 *          It owns the string buffer, ensuring that the data remains valid
 *          for the lifetime of the OwnedScopedKey instance.
 */
class OwnedScopedKey {
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
    OwnedScopedKey() = default;

    // construct from a view + suffix (runtime)
    OwnedScopedKey(ScopedKey const& base, std::string_view suffix);

    // direct construction
    OwnedScopedKey(std::string s, std::optional<std::string_view> const& scope = std::nullopt);

    // produce a scopedKey view that points into this owned buffer.
    // caller must keep the OwnedScopedKey alive while using the returned view.
    [[nodiscard]] ScopedKey view() const & noexcept ;

    // convenience: implicit conversion to scopedKey view (only valid while *this is alive)
    operator ScopedKey() const & noexcept ;

    // Add operator for appending suffixes
    [[nodiscard]] OwnedScopedKey operator+(std::string_view const& suffix) const ;
};
} // namespace Nebulite::Data


//------------------------------------------
// Compile-time friendly scoped key
namespace Nebulite::Data {
/**
 * @brief String wrapper to represent keys within a JsonScopeBase.
 * @details This class allows for easy conversion of string literals
 *          into fully scoped keys within the JsonScopeBase.
 *          This reduces accidental key misusage, as conversion to a usable type
 *          std::string requires an explicit action.
 *          It also provides safety checks to ensure that keys are used within their intended scopes.
 *          We can use this to generate static scoped keys in DomainModules, ensuring that
 *          they are always used in the correct scope.
 */
class ScopedKey {
    [[nodiscard]] std::string full(JsonScopeBase const& scope) const;

    /**
     * @brief Optional scope prefix for this key.
     * @details The required scope prefix that this key must be used within.
     *          If set, any JsonScopeBase using this key must have a scope
     *          that matches or is a sub-scope of this prefix.
     *          If not set, the key is assumed to be at the root scope.
     */
    std::optional<std::string_view> givenScope = std::nullopt;

    /**
     * @brief The key string within the scope.
     */
    std::string_view key;

    // private non-constexpr constructor used by OwnedScopedKey
    // Any key shared publicly should be constructed with a required scope to avoid accidental misuse
    constexpr ScopedKey(std::optional<std::string_view> const& requiredScope, std::string_view const& keyInScope) noexcept
        : givenScope(requiredScope), key(keyInScope) {}

    // allow the owning type to construct views pointing into its buffer
    friend class OwnedScopedKey;

public:
    friend class JsonScopeBase;
    friend class JsonScope;

    // No scope given, expected at root of JsonScopeBase
    template<typename T, typename = std::enable_if_t<std::is_constructible_v<std::string_view, T>>>
    constexpr ScopedKey(T const& keyInScope) noexcept
        : key(std::string_view(keyInScope)) {}

    // Scope given, is checked at usage time
    template<typename T, typename U>
    requires std::convertible_to<T, std::string_view> && std::convertible_to<U, std::string_view>
    constexpr ScopedKey(U const& requiredScope, T const& keyInScope) noexcept
        : givenScope(std::string_view(requiredScope)), key(std::string_view(keyInScope)) {}

    // Add Operator for appending suffixes that produces an OwnedScopedKey
    [[nodiscard]] OwnedScopedKey operator+(std::string_view const& suffix) const ;
};
} // namespace Nebulite::Data


#endif // NEBULITE_DATA_DOCUMENT_SCOPED_KEY_HPP
