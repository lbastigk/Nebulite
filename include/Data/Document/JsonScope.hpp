/**
 * @file JsonScope.hpp
 * @brief This file contains the definition of the JsonScope class, which provides a scoped interface
 *        for accessing and modifying JSON documents within the Nebulite engine.
 */

#ifndef NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
#define NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP

//------------------------------------------
// Includes

// Standard library
#include <thread>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Data/Document/JSON.hpp"       // TODO: Better to remove and forward-declare, but this requires a .tpp file for JsonScope methods that use JSON, where we can include JSON.hpp
#include "Data/OrderedDoublePointers.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
namespace Nebulite::Data {
/**
 * @class Nebulite::Data::JsonScope
 * @brief The JsonScope class provides a scoped interface for accessing and modifying JSON documents.
 * @details It allows for modifications to a JSON document within a specific scope,
 *          that is a key-prefixed section of the document. This is useful for modular data management,
 *          where different parts of a JSON document can be managed independently.
 *          Holds little data itself, mostly acts as a scoped view over an existing JSON document or another JsonScope.
 * @todo Move to namespace Nebulite::Core where all other Domain-related classes are located.
 * @todo Use scoped keys in DomainModules to reduce accidental key misusage:
 *       JsonScope::scopedKey key = {"subkey_in_scope", domain->getDoc().getScopePrefix()};
 *       We can then use this key outside of the DomainModule without worrying about scope issues.
 *       This would require changing all DomainModules to use JsonScope::scopedKey instead of std::string_view for hardcoded keys.
 */
NEBULITE_DOMAIN(JsonScope) {
    std::shared_ptr<JSON> baseDocument;
    std::string scopePrefix;

    //------------------------------------------
    // Ordered double pointers system

    /**
     * @brief Mapped ordered double pointers for expression references.
     * @todo A proper refactor so that each MappedOrderedDoublePointers has a JsonScope as root!
     *       This ensures that no accidental wrong key accesses happens.
     *       Increases complexity of construction a bit, but is worth it.
     *       -> Basically: for(m : MappedOrderedDoublePointers) m = MappedOrderedDoublePointers(this);
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
    static std::string generatePrefix(std::string const& givenPrefix) {
        std::string fullPrefix = givenPrefix;
        if (!fullPrefix.empty() && !fullPrefix.ends_with(".")) fullPrefix += ".";
        return fullPrefix;
    }

    void swap(JsonScope& o) noexcept ;

    // Necessary helper for shareScope
    JsonScope& shareManagedScope(std::string const& prefix) const {
        return shareScope(scopedKey(prefix));
    }

public:
    //------------------------------------------
    // Constructors

    // Constructing a JsonScope from a JSON document and a prefix
    JsonScope(JSON& doc, std::string const& prefix, std::string const& name = "Unnamed JsonScope");

    // Constructing a JsonScope from another JsonScope and a sub-prefix
    JsonScope(JsonScope const& other, std::string const& prefix, std::string const& name = "Unnamed JsonScope");

    // Default constructor, we create a self-owned empty JSON document
    explicit JsonScope(std::string const& name = "Unnamed JsonScope");

    //------------------------------------------
    // Special member functions

    // TODO: Implement all copy/move semantics properly

    JsonScope(JsonScope const& other);
    JsonScope(JsonScope&& other) noexcept;
    JsonScope& operator=(JsonScope const& other);
    JsonScope& operator=(JsonScope&& other) noexcept;

    ~JsonScope() override;

    //------------------------------------------
    // Get the prefix of this scope

    /**
     * @brief Gets the scope prefix with trailing dot.
     * @details If the prefix is empty, returns an empty string.
     * @return The scope prefix as a const reference to std::string.
     */
    std::string const& getScopePrefix() const noexcept {
        return scopePrefix;
    }

    //------------------------------------------
    // Helper struct for scoped keys

    /**
     * @brief A helper struct to represent keys within the JsonScope.
     * @details This struct allows for easy conversion of string literals
     *          into fully scoped keys within the JsonScope.
     *          This reduces accidental key misusage, as conversion to a usable type
     *          std::string requires an explicit action.
     *          It also provides safety checks to ensure that keys are used within their intended scopes.
     *          We can use this to generate static scoped keys in DomainModules, ensuring that
     *          they are always used in the correct scope.
     */
    struct scopedKey {
        // JsonScope should be the only class able to convert scopedKey to full key
        friend class JsonScope;

        // Accept any T that is constructible into std::string
        // We disable linting as the implicit conversion is intended here
        template<typename T, typename = std::enable_if_t<std::is_constructible_v<std::string, T>>>
        // NOLINTNEXTLINE
        scopedKey(T const& k)
            : key(std::string(k)), givenScope(std::nullopt) {}

        // To be used when we want to ensure that the key is used in a specific scope
        template<typename T, typename = std::enable_if_t<std::is_constructible_v<std::string, T>>>
        scopedKey(T const& k, JsonScope const& scope)
            : key(std::string(k)), givenScope(scope.getScopePrefix()) {}

    private:
        /**
         * @brief Generates the full scoped key using the provided JsonScope
         * @details Checks if the optional given scope from the key matches the allowed scope
         *          from the provided JsonScope. If they do not match, an exception is thrown.
         *          If no given scope is set, the JsonScopes scopePrefix is used directly.
         * @param scope The JsonScope to use for generating the full key.
         * @return The fully scoped key as a std::string.
         */
        std::string full(JsonScope const& scope) const {
            // The scope that this JsonScope is allowed to use
            std::string const& allowedScope = scope.scopePrefix;

            // See if we require a specific scope
            bool const requiresScope = givenScope.has_value();
            std::string fullKey;
            if (requiresScope) {
                // Ensure that the given scope lies within the allowed scope
                // E.g. givenScope = "module1.submodule." allowedScope = "module1." -> valid
                //      givenScope = "module2."           allowedScope = "module1." -> invalid, we are only allowed to use module1.*
                std::string const& given = *givenScope;
                if (!given.starts_with(allowedScope)) {
                    std::string const msg =
                        "ScopedKey scope mismatch: key '" + std::string(key) +
                        "' was created with the given scope prefix '" + given +
                        "' but was used in JsonScope with prefix '" + allowedScope;
                    throw std::invalid_argument(msg);
                }

                // Now we can safely use the given scope, as it lies within the allowed scope
                fullKey.reserve(given.size() + key.size());
                fullKey = given;
                fullKey.append(key);
                return fullKey;
            }
            else {
                fullKey.reserve(allowedScope.size() + key.size());
                fullKey = scope.scopePrefix;
                fullKey.append(key);
                return fullKey;
            }
        }

        // The actual key within the scope
        std::string key;

        // Optional given scope. The JsonScope must have access to this scope when using the key.
        std::optional<std::string> givenScope;
    };

    //------------------------------------------
    // Domain related stuff

    Constants::Error update() override {
        updateModules();
        return Constants::ErrorTable::NONE();
    }

    //------------------------------------------
    // Sharing a scope

    // Proper scope sharing with nested unscoped key generation
    JsonScope& shareScope(scopedKey const& key) const {
        return baseDocument->shareManagedScope(key.full(*this));
    }

    //------------------------------------------
    // Getter

    template<typename T>
    T get(scopedKey const& key, T const& defaultValue = T()) const {
        return baseDocument->get<T>(key.full(*this), defaultValue);
    }

    std::optional<RjDirectAccess::simpleValue> getVariant(scopedKey const& key) const {
        return baseDocument->getVariant(key.full(*this));
    }

    JSON getSubDoc(scopedKey const& key) const {
        return baseDocument->getSubDoc(key.full(*this));
    }

    double* getStableDoublePointer(scopedKey const& key) const {
        return baseDocument->getStableDoublePointer(key.full(*this));
    }

    //------------------------------------------
    // Setter

    template<typename T>
    void set(scopedKey const& key, T const& value) {
        baseDocument->set<T>(key.full(*this), value);
    }

    void setVariant(scopedKey const& key, RjDirectAccess::simpleValue const& value) const {
        baseDocument->setVariant(key.full(*this), value);
    }

    void setSubDoc(scopedKey const& key, JSON& subDoc) const {
        baseDocument->setSubDoc(key.full(*this), subDoc);
    }

    void setSubDoc(scopedKey const& key, JsonScope const& subDoc) const {
        // Slightly more complicated: If we wish to set the sub-document from another JsonScope,
        // we need to extract the underlying JSON document from it in the correct scope.
        JSON subDocScope = subDoc.getSubDoc(scopedKey(""));
        baseDocument->setSubDoc(key.full(*this), subDocScope);
    }

    void setEmptyArray(scopedKey const& key) const {
        baseDocument->setEmptyArray(key.full(*this));
    }

    //------------------------------------------
    // Special sets for threadsafe maths operations

    void set_add(scopedKey const& key, double const& val) const {
        baseDocument->set_add(key.full(*this), val);
    }

    void set_multiply(scopedKey const& key, double const& val) const {
        baseDocument->set_multiply(key.full(*this), val);
    }

    void set_concat(scopedKey const& key, std::string const& valStr) const {
        baseDocument->set_concat(key.full(*this), valStr);
    }

    //------------------------------------------
    // Locking

    std::scoped_lock<std::recursive_mutex> lock() const {
        return baseDocument->lock();
    }

    //------------------------------------------
    // Getters: Unique id based retrieval

    MappedOrderedDoublePointers* getOrderedCacheListMap() {
#if ORDERED_DOUBLE_POINTERS_MAPS == 1
        return &expressionRefs[0];
#else
        // Each thread gets a unique starting position based on thread ID
        thread_local const size_t idx = std::hash<std::thread::id>{}(std::this_thread::get_id()) % ORDERED_DOUBLE_POINTERS_MAPS;
        return &expressionRefs[idx];
#endif
    }

    //------------------------------------------
    // Key Types, Sizes

    JSON::KeyType memberType(scopedKey const& key) const {
        return baseDocument->memberType(key.full(*this));
    }

    size_t memberSize(scopedKey const& key) const {
        return baseDocument->memberSize(key.full(*this));
    }

    void removeKey(scopedKey const& key) const {
        baseDocument->removeKey(key.full(*this));
    }

    //------------------------------------------
    // Serialize/Deserialize

    std::string serialize(scopedKey const& key = scopedKey("")) const {
        return baseDocument->serialize(key.full(*this));
    }

    void deserialize(std::string const& serialOrLink);
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
