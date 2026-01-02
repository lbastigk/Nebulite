/**
 * @file JsonScope.hpp
 * @brief This file contains the definition of the JsonScope class, which provides a scoped interface
 *        for accessing and modifying JSON documents within the Nebulite engine.
 */

#ifndef NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
#define NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/JSON.hpp"

//------------------------------------------
namespace Nebulite::Data {

/**
 * @class Nebulite::Data::JsonScope
 * @brief The JsonScope class provides a scoped interface for accessing and modifying JSON documents.
 * @details It allows for temporary modifications to a JSON document within a specific scope,
 *          that is a key-prefixed section of the document.
 * @todo Proper overload order required. Prioritize string_view?
 * @todo Needs a proper implementation of domain-related stuff
 *       1.) Ensure domains have a JsonScope instead of direct JSON access
 *       2.) Ensure that the JSON domainModules are either part of the scope or part of both JSON and scope
 *       For now, JsonScope is not a Nebulite domain.
 *       A good idea would be to have JSON not be a domain, but JsonScope be one.
 *       This drastically simplifies things, such as construction of JSON documents,
 *       and having a nice wrapper for scoped access, that itself is a domain.
 */
class JsonScope {
    std::variant<std::shared_ptr<JSON>, std::reference_wrapper<JsonScope>> baseDocument;
    std::string scopePrefix;

    //------------------------------------------
    // Helper for std::visit

    template<typename F>
    decltype(auto) visitBase(F&& f) {
            return std::visit([&](auto&& alt) -> decltype(auto) {
                using Alt = std::decay_t<decltype(alt)>;
                if constexpr (std::is_same_v<Alt, std::shared_ptr<JSON>>) {
                    return std::invoke(std::forward<F>(f), *alt);        // JSON&
                } else {
                    return std::invoke(std::forward<F>(f), alt.get());   // JsonScope&
                }
            }, baseDocument);
        }

    template<typename F>
    decltype(auto) visitBase(F&& f) const {
        return std::visit([&](auto const& alt) -> decltype(auto) {
            using Alt = std::decay_t<decltype(alt)>;
            if constexpr (std::is_same_v<Alt, std::shared_ptr<JSON>>) {
                return std::invoke(std::forward<F>(f), *alt);        // JSON&
            } else {
                return std::invoke(std::forward<F>(f), alt.get());   // JsonScope&
            }
        }, baseDocument);
    }

    //------------------------------------------
    // Ordered double pointers system

    // TODO: To be implemented: Each JsonScope should have its own ordered double pointers system,
    //       so that expressions and rulesets working within a scope receive the correct pointers.

    /**
     * @brief Mapped ordered double pointers for expression references.
     */
    MappedOrderedDoublePointers expressionRefs[ORDERED_DOUBLE_POINTERS_MAPS];

    /**
     * @brief Super quick double cache based on unique IDs, no hash lookup.
     *        Used for the first few entries. It's recommended to reserve
     *        low value uids for frequently used keys.
     * @todo Add a reserve-mechanism in globalspace for ids, so they are low value.
     */
    std::array<double*, JSON::uidQuickCacheSize> uidDoubleCache{nullptr};

    //------------------------------------------
    // Valid prefix check and generation

    std::string generatePrefix(std::string const& givenPrefix) const {
        std::string fullPrefix = scopePrefix + givenPrefix;
        if (!fullPrefix.empty() && !fullPrefix.ends_with(".")) fullPrefix += ".";
        return fullPrefix;
    }

    std::string generateFullKey(std::string const& key) const {
        std::string full;
        full.reserve(scopePrefix.size() + key.size());
        full = scopePrefix;
        full.append(key);
        return full;
    }

    std::string generateFullKey(std::string_view const& key) const {
        std::string full;
        full.reserve(scopePrefix.size() + key.size());
        full = scopePrefix;
        full.append(key);
        return full;
    }

    std::string generateFullKey(char const* key) const {
        std::string full;
        full.reserve(scopePrefix.size() + std::strlen(key));
        full = scopePrefix;
        full.append(key);
        return full;
    }

public:
    //------------------------------------------
    // Constructors

    // Constructing a JsonScope from a JSON document and a prefix
    JsonScope(JSON& doc, std::string const& prefix)
        // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
        : baseDocument(std::shared_ptr<JSON>(&doc, [](JSON*){})), scopePrefix(generatePrefix(prefix)) {}

    // Constructing a JsonScope from another JsonScope and a sub-prefix
    JsonScope(JsonScope& other, std::string const& prefix)
        : baseDocument(std::ref(other)), scopePrefix(prefix) {
        if (!scopePrefix.empty() && !scopePrefix.ends_with(".")) scopePrefix += ".";
    }

    // Default constructor, we create a self-owned empty JSON document
    JsonScope()
        : baseDocument(std::make_shared<JSON>()), scopePrefix("") {
    }

    //------------------------------------------
    // Danger territory: access to base document

    [[deprecated("Accessing the base document breaks the scope abstraction. This should only be used temporarly if certain JSON features aren't available in JsonScope. Use with caution.")]]
    JSON& getBaseDocument() {
        return visitBase([](auto& alt) -> JSON& {
            using Alt = std::decay_t<decltype(alt)>;
            if constexpr (std::is_same_v<Alt, JSON>) {
                return alt;
            } else {
                return alt.getBaseDocument();
            }
        });
    }

    //------------------------------------------
    // Sharing a scope

    JsonScope shareScope(std::string const& subPrefix) {
        return JsonScope(*this, generateFullKey(subPrefix));
    }

    //------------------------------------------
    // Getter

    template<typename T>
    T get(std::string const& key, T const& defaultValue) const {
        return visitBase([&](auto& alt) -> T {
            return alt.template get<T>(generateFullKey(key), defaultValue);
        });
    }
    template<typename T>
    T get(std::string_view const& key, T const& defaultValue) const {
        return visitBase([&](auto& alt) -> T {
            return alt.template get<T>(generateFullKey(key), defaultValue);
        });
    }
    template<typename T>
    T get(char const* key, T const& defaultValue) const {
        return visitBase([&](auto& alt) -> T {
            return alt.template get<T>(generateFullKey(key), defaultValue);
        });
    }

    std::optional<RjDirectAccess::simpleValue> getVariant(std::string const& key) const {
        return visitBase([&](auto& alt) -> std::optional<RjDirectAccess::simpleValue> {
            return alt.getVariant(generateFullKey(key));
        });
    }
    std::optional<RjDirectAccess::simpleValue> getVariant(std::string_view const& key) const {
        return visitBase([&](auto& alt) -> std::optional<RjDirectAccess::simpleValue> {
            return alt.getVariant(generateFullKey(key));
        });
    }
    std::optional<RjDirectAccess::simpleValue> getVariant(char const* key) const {
        return visitBase([&](auto& alt) -> std::optional<RjDirectAccess::simpleValue> {
            return alt.getVariant(generateFullKey(key));
        });
    }

    JSON getSubDoc(std::string const& key) const {
        return visitBase([&](auto& alt) -> JSON {
            return alt.getSubDoc(generateFullKey(key));
        });
    }
    JSON getSubDoc(std::string_view const& key) const {
        return visitBase([&](auto& alt) -> JSON {
            return alt.getSubDoc(generateFullKey(key));
        });
    }
    JSON getSubDoc(char const* key) const {
        return visitBase([&](auto& alt) -> JSON {
            return alt.getSubDoc(generateFullKey(key));
        });
    }

    double* getStableDoublePointer(std::string const& key) {
        return visitBase([&](auto& alt) -> double* {
            return alt.getStableDoublePointer(generateFullKey(key));
        });
    }
    double* getStableDoublePointer(std::string_view const& key) {
        return visitBase([&](auto& alt) -> double* {
            return alt.getStableDoublePointer(generateFullKey(key));
        });
    }
    double* getStableDoublePointer(char const* key) {
        return visitBase([&](auto& alt) -> double* {
            return alt.getStableDoublePointer(generateFullKey(key));
        });
    }

    //------------------------------------------
    // Setter

    template<typename T>
    void set(std::string const& key, T const& value) {
        visitBase([&](auto& alt) -> void {
            alt.template set<T>(generateFullKey(key), value);
        });
    }
    template<typename T>
    void set(std::string_view const& key, T const& value) {
        visitBase([&](auto& alt) -> void {
            alt.template set<T>(generateFullKey(key), value);
        });
    }
    template<typename T>
    void set(char const* key, T const& value) {
        visitBase([&](auto& alt) -> void {
            alt.template set<T>(generateFullKey(key), value);
        });
    }

    void setVariant(std::string const& key, RjDirectAccess::simpleValue const& value) {
        visitBase([&](auto& alt) -> void {
            alt.setVariant(generateFullKey(key), value);
        });
    }
    void setVariant(std::string_view const& key, RjDirectAccess::simpleValue const& value) {
        visitBase([&](auto& alt) -> void {
            alt.setVariant(generateFullKey(key), value);
        });
    }
    void setVariant(char const* key, RjDirectAccess::simpleValue const& value) {
        visitBase([&](auto& alt) -> void {
            alt.setVariant(generateFullKey(key), value);
        });
    }

    void setSubDoc(std::string const& key, JSON& subDoc) {
        visitBase([&](auto& alt) -> void {
            alt.setSubDoc(generateFullKey(key), subDoc);
        });
    }
    void setSubDoc(std::string_view const& key, JSON& subDoc) {
        visitBase([&](auto& alt) -> void {
            alt.setSubDoc(generateFullKey(key), subDoc);
        });
    }
    void setSubDoc(char const* key, JSON& subDoc) {
        visitBase([&](auto& alt) -> void {
            alt.setSubDoc(generateFullKey(key), subDoc);
        });
    }

    void setEmptyArray(std::string const& key) {
        visitBase([&](auto& alt) -> void {
            alt.setEmptyArray(generateFullKey(key));
        });
    }
    void setEmptyArray(std::string_view const& key) {
        visitBase([&](auto& alt) -> void {
            alt.setEmptyArray(generateFullKey(key));
        });
    }
    void setEmptyArray(char const* key) {
        visitBase([&](auto& alt) -> void {
            alt.setEmptyArray(generateFullKey(key));
        });
    }

    //------------------------------------------
    // Special sets for threadsafe maths operations

    void set_add(std::string_view const& key, double const& val) {
        visitBase([&](auto& alt) -> void {
            alt.set_add(generateFullKey(key), val);
        });
    }

    void set_multiply(std::string_view const& key, double const& val) {
        visitBase([&](auto& alt) -> void {
            alt.set_multiply(generateFullKey(key), val);
        });
    }

    void set_concat(std::string_view const& key, std::string const& valStr) {
        visitBase([&](auto& alt) -> void {
            alt.set_concat(generateFullKey(key), valStr);
        });
    }

    //------------------------------------------
    // Locking

    std::scoped_lock<std::recursive_mutex> lock() {
        return visitBase([&](auto& alt) -> std::scoped_lock<std::recursive_mutex> {
            return alt.lock();
        });
    }

    //------------------------------------------
    // Getters: Unique id based retrieval

    // TODO: Probably best to have an own map for scopes?
    //       otherwise key clashes may happen, as MappedOrderedDoublePointers
    //       itself manages the keys internally.
    //       MappedOrderedDoublePointers* probably needs its own prefixing system!!!
    //       maybe best to pass on construction, more research needed.

    // WARNING: This is not yet implemented!
    [[deprecated("JsonScope::getOrderedCacheListMap is not yet implemented!")]]
    MappedOrderedDoublePointers* getOrderedCacheListMap() {
        return nullptr; // To be implemented
    }

    //------------------------------------------
    // Key Types, Sizes

    JSON::KeyType memberType(std::string const& key) {
        return visitBase([&](auto& alt) -> JSON::KeyType {
            return alt.memberType(generateFullKey(key));
        });
    }
    JSON::KeyType memberType(std::string_view key) {
        return visitBase([&](auto& alt) -> JSON::KeyType {
            return alt.memberType(generateFullKey(key));
        });
    }
    JSON::KeyType memberType(char const* key) {
        return visitBase([&](auto& alt) -> JSON::KeyType {
            return alt.memberType(generateFullKey(key));
        });
    }

    size_t memberSize(std::string const& key) {
        return visitBase([&](auto& alt) -> size_t {
            return alt.memberSize(generateFullKey(key));
        });
    }
    size_t memberSize(std::string_view key) {
        return visitBase([&](auto& alt) -> size_t {
            return alt.memberSize(generateFullKey(key));
        });
    }
    size_t memberSize(char const* key) {
        return visitBase([&](auto& alt) -> size_t {
            return alt.memberSize(generateFullKey(key));
        });
    }

    void removeKey(std::string_view key) {
        visitBase([&](auto& alt) -> void {
            alt.removeKey(generateFullKey(key).c_str());
        });
    }
    void removeKey(std::string const& key) {
        visitBase([&](auto& alt) -> void {
            alt.removeKey(generateFullKey(key).c_str());
        });
    }

    void removeKey(char const* key) {
        visitBase([&](auto& alt) -> void {
            alt.removeKey(generateFullKey(key).c_str());
        });
    }

    //------------------------------------------
    // Serialize/Deserialize

    std::string serialize(std::string const& key = "") {
        return visitBase([&](auto& alt) -> std::string {
            return alt.serialize(generateFullKey(key));
        });
    }
    void deserialize(std::string const& serialOrLink) {
        JSON tmp;
        tmp.deserialize(serialOrLink);
        auto scopePrefixWithoutDot = scopePrefix;
        if (!scopePrefixWithoutDot.empty() && scopePrefixWithoutDot.ends_with(".")) {
            scopePrefixWithoutDot = scopePrefixWithoutDot.substr(0, scopePrefixWithoutDot.size() - 1);
        }
        visitBase([&](auto& alt) -> void {
            alt.setSubDoc(scopePrefixWithoutDot, tmp);
        });
    }
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
