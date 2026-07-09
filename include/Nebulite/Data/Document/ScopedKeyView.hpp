#ifndef NEBULITE_DATA_DOCUMENT_SCOPEDKEYVIEW_HPP
#define NEBULITE_DATA_DOCUMENT_SCOPEDKEYVIEW_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Data/OptionalFixedString.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class ScopedKey;
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
// Helper struct for compile-time optional fixed strings as template parameters

namespace Nebulite::Data {
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
    std::optional<std::string_view> givenScope = std::nullopt; // NOLINT

    /**
     * @brief The key string within the scope.
     */
    std::string_view key; // NOLINT

    // allow the owning type to construct views pointing into its buffer
    friend class ScopedKey;

    // Adding suffix to produce a new ScopedKeyView
    [[nodiscard]] ScopedKey operator+(std::string_view suffix) const ;

public:
    /**
     * @brief Produce the full key string including scope prefix.
     * @param scope The current scope of the JsonScope.
     * @return The full key string with scope prefix.
     * @throws std::invalid_argument if the key is used outside its required scope.
     */
    [[nodiscard]] std::string full(JsonScope const& scope) const;

    // Any key shared publicly should be constructed with a required scope to avoid accidental misuse
    constexpr ScopedKeyView(std::optional<std::string_view> const& requiredScope, std::string_view const keyInScope) noexcept
        : givenScope(requiredScope), key(keyInScope) {}

    explicit constexpr ScopedKeyView(std::string_view const keyInScope) : key(keyInScope) {}

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
        if constexpr (!RequiredScope.hasValue()) {
            // no scope provided
            return {std::nullopt, std::string_view(keyInScope)};
        } else {
            // compute length at compile time
            constexpr std::size_t len = RequiredScope.length();

            // static assert to ensure scope is valid
            static_assert(len == 0 || RequiredScope.back() == '.', "ScopedKey: The provided scope must be empty or end with a dot ('.')");

            return {std::optional(RequiredScope.view()), std::string_view(keyInScope)};
        }
    }

    [[nodiscard]] std::string toString() const {
        return buildKey();
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
    static std::string combineKeys(std::string_view key1, std::string_view key2);

    /**
     * @brief Adds a specified index to the key
     */
    [[nodiscard]] ScopedKey addIndex(std::size_t index) const ;

    /**
     * @brief Adds a specified member to the key
     */
    [[nodiscard]] ScopedKey addMember(std::string_view member) const ;
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_SCOPEDKEYVIEW_HPP
