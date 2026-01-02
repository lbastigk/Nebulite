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
#include "Interaction/Execution/Domain.hpp"
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
NEBULITE_DOMAIN(JsonScope) {
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

    //------------------------------------------
    // Valid prefix check and generation

    /**
     * @brief Generates a proper prefix given a user-provided prefix.
     * @details Ensures that the prefix ends with a dot if it's not empty.
     * @param givenPrefix The user-provided prefix.
     * @return The properly formatted prefix.
     */
    std::string generatePrefix(std::string const& givenPrefix) const {
        std::string fullPrefix = givenPrefix;
        if (!fullPrefix.empty() && !fullPrefix.ends_with(".")) fullPrefix += ".";
        return fullPrefix;
    }

public:
    //------------------------------------------
    // Constructors

    // Constructing a JsonScope from a JSON document and a prefix
    JsonScope(JSON& doc, std::string const& prefix, std::string const& name = "Unnamed JsonScope")
        : Domain(name, *this, getBaseDocument()),   // TODO: change to *this once JsonScope is used in domains
        // create a non-owning shared_ptr to the provided JSON (no delete on destruction)
        baseDocument(std::shared_ptr<JSON>(&doc, [](JSON*){})), scopePrefix(generatePrefix(prefix)) {}

    // Constructing a JsonScope from another JsonScope and a sub-prefix
    JsonScope(JsonScope& other, std::string const& prefix, std::string const& name = "Unnamed JsonScope")
        : Domain(name, *this, getBaseDocument()),   // TODO: change to *this once JsonScope is used in domains
          baseDocument(std::ref(other)), scopePrefix(generatePrefix(prefix)) {}

    // Default constructor, we create a self-owned empty JSON document
    JsonScope(std::string const& name = "Unnamed JsonScope")
        : Domain(name, *this, getBaseDocument()),   // TODO: change to *this once JsonScope is used in domains
          baseDocument(std::make_shared<JSON>()), scopePrefix("") {}

    //------------------------------------------
    // Special member functions

    JsonScope(JsonScope const&) = delete;
    JsonScope& operator=(JsonScope const&) = delete;
    JsonScope(JsonScope&&) noexcept;
    JsonScope& operator=(JsonScope&&) noexcept;

    ~JsonScope() override;

    //------------------------------------------
    // Idea: Using a custom type to turn a string literal into a scoped key

    /**
     * @brief A helper struct to represent unscoped keys.
     * @details This struct allows for easy conversion of string literals
     *          into fully scoped keys within the JsonScope.
     *          This reduces accidental key misusage, as conversion to a usable type
     *          std::string requires an explicit action.
     */
    struct unscopedKey {
        unscopedKey(std::string_view const& k) : key(k) {}
        unscopedKey(const char* k) : key(k) {}
        unscopedKey(std::string const& k) : key(k) {}

        friend class JsonScope;
    private:
        /**
         * @brief Generates the full scoped key using the provided JsonScope.
         * @param scope The JsonScope to use for generating the full key.
         * @return The fully scoped key as a std::string.
         */
        std::string full(JsonScope const* scope) const {
            std::string full;
            full.reserve(scope->scopePrefix.size() + key.size());
            full = scope->scopePrefix;
            full.append(key);
            return full;
        }
        std::string_view key;
    };

    //------------------------------------------
    // Domain related stuff

    Constants::Error update() override {
        // TODO: Remove domain from JSON, make JsonScope a domain instead
        //       for now we just return NONE
        return Constants::ErrorTable::NONE();
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

    JsonScope shareScope(unscopedKey const& key) {
        return JsonScope(*this, key.full(this));
    }

    //------------------------------------------
    // Getter

    template<typename T>
    T get(unscopedKey const& key, T const& defaultValue) const {
        return visitBase([&](auto& alt) -> T {
            return alt.template get<T>(key.full(this), defaultValue);
        });
    }

    std::optional<RjDirectAccess::simpleValue> getVariant(unscopedKey const& key) const {
        return visitBase([&](auto& alt) -> std::optional<RjDirectAccess::simpleValue> {
            return alt.getVariant(key.full(this));
        });
    }

    JSON getSubDoc(unscopedKey const& key) const {
        return visitBase([&](auto& alt) -> JSON {
            return alt.getSubDoc(key.full(this));
        });
    }

    double* getStableDoublePointer(unscopedKey const& key) {
        return visitBase([&](auto& alt) -> double* {
            return alt.getStableDoublePointer(key.full(this));
        });
    }

    //------------------------------------------
    // Setter

    template<typename T>
    void set(unscopedKey const& key, T const& value) {
        visitBase([&](auto& alt) -> void {
            alt.template set<T>(key.full(this), value);
        });
    }

    void setVariant(unscopedKey const& key, RjDirectAccess::simpleValue const& value) {
        visitBase([&](auto& alt) -> void {
            alt.setVariant(key.full(this), value);
        });
    }

    void setSubDoc(unscopedKey const& key, JSON& subDoc) {
        visitBase([&](auto& alt) -> void {
            alt.setSubDoc(key.full(this), subDoc);
        });
    }

    void setEmptyArray(unscopedKey const& key) {
        visitBase([&](auto& alt) -> void {
            alt.setEmptyArray(key.full(this));
        });
    }

    //------------------------------------------
    // Special sets for threadsafe maths operations

    void set_add(unscopedKey const& key, double const& val) {
        visitBase([&](auto& alt) -> void {
            alt.set_add(key.full(this), val);
        });
    }

    void set_multiply(unscopedKey const& key, double const& val) {
        visitBase([&](auto& alt) -> void {
            alt.set_multiply(key.full(this), val);
        });
    }

    void set_concat(unscopedKey const& key, std::string const& valStr) {
        visitBase([&](auto& alt) -> void {
            alt.set_concat(key.full(this), valStr);
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

    JSON::KeyType memberType(unscopedKey const& key) {
        return visitBase([&](auto& alt) -> JSON::KeyType {
            return alt.memberType(key.full(this));
        });
    }

    size_t memberSize(unscopedKey const& key) {
        return visitBase([&](auto& alt) -> size_t {
            return alt.memberSize(key.full(this));
        });
    }

    void removeKey(unscopedKey const& key) {
        visitBase([&](auto& alt) -> void {
            alt.removeKey(key.full(this));
        });
    }

    //------------------------------------------
    // Serialize/Deserialize

    std::string serialize(unscopedKey const& key = "") {
        return visitBase([&](auto& alt) -> std::string {
            return alt.serialize(key.full(this));
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
        reinitModules();
    }
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
