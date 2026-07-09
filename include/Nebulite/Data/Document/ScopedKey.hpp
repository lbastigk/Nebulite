#ifndef NEBULITE_DATA_DOCUMENT_SCOPEDKEY_HPP
#define NEBULITE_DATA_DOCUMENT_SCOPEDKEY_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class ScopedKeyView;
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
// Helper struct for compile-time optional fixed strings as template parameters

namespace Nebulite::Data {

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

    // Add operator for appending suffixes
    [[nodiscard]] ScopedKey operator+(std::string_view suffix) const ;

public:
    ScopedKey() = default;

    // construct from a view + suffix
    ScopedKey(ScopedKeyView const& base, std::string_view suffix);

    // direct construction
    explicit ScopedKey(std::optional<std::string_view> const& scope, std::string suffix);
    explicit ScopedKey(std::string_view suffix);

    [[nodiscard]] std::string toString() const ;

    // produce a scopedKey view that points into this owned buffer.
    // caller must keep the ScopedKeyView alive while using the returned view.
    [[nodiscard]] ScopedKeyView view() const & noexcept ;

    /**
     * @brief Add another scopedKey to the current Key
     */
    [[nodiscard]] ScopedKey nestKey(ScopedKey const& other) const ;
    [[nodiscard]] ScopedKey nestKey(ScopedKeyView const& other) const ;

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
#endif // NEBULITE_DATA_DOCUMENT_SCOPEDKEY_HPP
