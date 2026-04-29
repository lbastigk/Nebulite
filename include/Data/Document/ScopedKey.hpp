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
 *          The checks happen at runtime when the key is used to access the JsonScope,
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

namespace Nebulite::Data {
class ScopedKey;
class ScopedKeyView;
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
// Helper struct for compile-time optional fixed strings as template parameters

namespace Nebulite::Data {

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

OptionalFixedString() -> OptionalFixedString<0>;

//------------------------------------------
// Self-owning scoped key

/**
 * @class ScopedKey
 * @brief An owning version of ScopedKeyView that holds its own string buffer.
 * @details This class is useful for constructing scoped keys at runtime,
 *          where the key string needs to be built dynamically.
 *          It owns the string buffer, ensuring that the data remains valid
 *          for the lifetime of the ScopedKey instance.
 *          So what is the point of the scoped keying? Some keys are meant to be used in specific scopes, and to better detect accidental misuse of keys,
 *          we return access errors when a key is used in the wrong scope. This is unlike accessing members that don't exist or are not returnable as the requested type.
 *          The difference here is:
 *          - an error return when the key is used in the right scope is like asking for a book in a library and being told
 *            "Sorry, we don't have that book" (the key doesn't exist or is not the right type)
 *          - an error return when the key is used in the wrong scope is like asking for a hotdog in a library.
 *            You clearly are at the wrong place to ask for that!
 */
class ScopedKey {
    /**
     * @brief Optional scope prefix for this key.
     * @details The required scope prefix that this key must be used within.
     *          If set, any JsonScope using this key must have a scope
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

    // construct from a view + suffix
    ScopedKey(ScopedKeyView const& base, std::string_view const& suffix);

    // direct construction
    explicit ScopedKey(std::optional<std::string_view> const& scope, std::string suffix);
    explicit ScopedKey(std::string suffix);

    // produce a scopedKey view that points into this owned buffer.
    // caller must keep the ScopedKeyView alive while using the returned view.
    [[nodiscard]] ScopedKeyView view() const & noexcept ;

    // Add operator for appending suffixes
    [[nodiscard]] ScopedKey operator+(std::string_view const& suffix) const ;

    /**
     * @brief A constant representing the absence of a scope.
     */
    static constexpr auto noScope = OptionalFixedString();

    /**
     * @brief Add another scopedKey to the current Key
     */
    [[nodiscard]] ScopedKey nestKey(ScopedKey const& other) const ;
    [[nodiscard]] ScopedKey nestKey(ScopedKeyView const& other) const ;
};

//------------------------------------------
// Compile-time friendly scoped key

/**
 * @class ScopedKeyView
 * @brief Non-Owning String wrapper to represent keys within a JsonScope.
 * @details This class allows for easy conversion of string literals
 *          into fully scoped keys within the JsonScope.
 *          This reduces accidental key misusage, as conversion to a usable type
 *          std::string requires an explicit action.
 *          It also provides safety checks to ensure that keys are used within their intended scopes.
 *          We can use this to generate static scoped keys in DomainModules, ensuring that
 *          they are always used in the correct scope.
 *          See also: ScopedKey, which is an owning version of this class that holds its own string buffer.
 */
class ScopedKeyView {
    /**
     * @brief Combines the givenScope and the prefix to a full key
     * @return the full key
     */
    [[nodiscard]] std::string buildKey() const ;

    /**
     * @brief Optional scope prefix for this key.
     * @details The required scope prefix that this key must be used within.
     *          If set, any JsonScope using this key must have a scope
     *          that matches or is a sub-scope of this prefix.
     *          If not set, the key is assumed to be at the root scope.
     */
    std::optional<std::string_view> givenScope = std::nullopt;

    /**
     * @brief The key string within the scope.
     */
    std::string_view key;

    // allow the owning type to construct views pointing into its buffer
    friend class ScopedKey;

public:
    /**
     * @brief Produce the full key string including scope prefix.
     * @param scope The current scope of the JsonScope.
     * @return The full key string with scope prefix.
     * @throws std::invalid_argument if the key is used outside its required scope.
     */
    [[nodiscard]] std::string full(JsonScope const& scope) const;

    // Adding suffix to produce a new ScopedKeyView
    [[nodiscard]] ScopedKey operator+(std::string_view const& suffix) const ;

    // Any key shared publicly should be constructed with a required scope to avoid accidental misuse
    constexpr ScopedKeyView(std::optional<std::string_view> const& requiredScope, std::string_view const& keyInScope) noexcept
        : givenScope(requiredScope), key(keyInScope) {}

    explicit constexpr ScopedKeyView(std::string_view const& keyInScope) : key(keyInScope) {}

    /**
     * @brief Create a ScopedKeyView from a compile-time fixed string with an optional scope prefix.
     * @tparam RequiredScope A compile-time fixed string representing the required scope prefix for this key.
     *                       If empty, the keys scope is arbitrary and assumed to be at the root of the JsonScope.
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

    [[nodiscard]] std::string toString() const {
        if (givenScope.has_value()) {
            return std::string(givenScope.value()) + std::string(key);
        }
        return std::string(key);
    }

    [[nodiscard]] ScopedKey toScopedKey() const {
        if (givenScope.has_value()) {
            return ScopedKey(givenScope, std::string(givenScope.value()) + std::string(key));
        }
        return ScopedKey(std::nullopt, std::string(key));
    }

    /**
     * @brief Add another scopedKey to the current Key
     */
    [[nodiscard]] ScopedKey nestKey(ScopedKey const& other) const ;
    [[nodiscard]] ScopedKey nestKey(ScopedKeyView const& other) const ;

    /**
     * @brief Combines two keys, removing trailing dots between them if necessary.
     * @param key1 The first key
     * @param key2 The second key to append to the first key
     * @return The combined key string
     */
    static std::string combineKeys(std::string_view const& key1, std::string_view const& key2);
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_SCOPED_KEY_HPP
