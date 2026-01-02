/**
 * @file JsonScope.hpp
 * @brief This file contains the definition of the JsonScope and JsonScobeBase class, which provides a scoped interface
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
 * @class Nebulite::Data::JsonScopeBase
 * @brief The JsonScopeBase class provides a scoped interface for accessing and modifying JSON documents.
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
class JsonScopeBase {
protected:
    std::shared_ptr<JSON> baseDocument;

private:

    std::string scopePrefix;

    //------------------------------------------
    // Ordered double pointers system

    /**
     * @brief Mapped ordered double pointers for expression references.
     * @todo A proper refactor so that each MappedOrderedDoublePointers has a JsonScopeBase as root!
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

    void swap(JsonScopeBase& o) noexcept ;

    // Necessary helper for shareScope
    [[nodiscard]] JsonScope& shareManagedScope(std::string const& prefix) const {
        return shareScope(scopedKey(prefix));
    }

public:
    //------------------------------------------
    // Constructors

    // Constructing a JsonScopeBase from a JSON document and a prefix
    JsonScopeBase(JSON& doc, std::string const& prefix);

    // Constructing a JsonScopeBase from another JsonScopeBase and a sub-prefix
    JsonScopeBase(JsonScopeBase const& other, std::string const& prefix);

    // Default constructor, we create a self-owned empty JSON document
    explicit JsonScopeBase();

    //------------------------------------------
    // Special member functions

    JsonScopeBase(JsonScopeBase const& other);
    JsonScopeBase(JsonScopeBase&& other) noexcept;
    JsonScopeBase& operator=(JsonScopeBase const& other);
    JsonScopeBase& operator=(JsonScopeBase&& other) noexcept;

    virtual ~JsonScopeBase();

    //------------------------------------------
    // Get the prefix of this scope

    /**
     * @brief Gets the scope prefix with trailing dot.
     * @details If the prefix is empty, returns an empty string.
     * @return The scope prefix as a const reference to std::string.
     */
    [[nodiscard]] std::string const& getScopePrefix() const noexcept {
        return scopePrefix;
    }

    //------------------------------------------
    // Helper struct for scoped keys

    /**
     * @brief A helper struct to represent keys within the JsonScopeBase.
     * @details This struct allows for easy conversion of string literals
     *          into fully scoped keys within the JsonScopeBase.
     *          This reduces accidental key misusage, as conversion to a usable type
     *          std::string requires an explicit action.
     *          It also provides safety checks to ensure that keys are used within their intended scopes.
     *          We can use this to generate static scoped keys in DomainModules, ensuring that
     *          they are always used in the correct scope.
     */
    class scopedKey {
        /**
         * @brief Generates the full scoped key using the provided JsonScope
         * @details Checks if the optional given scope from the key matches the allowed scope
         *          from the provided JsonScope. If they do not match, an exception is thrown.
         *          If no given scope is set, the JsonScopes scopePrefix is used directly.
         * @param scope The JsonScopeBase to use for generating the full key and checking scope validity.
         * @return The fully scoped key as a std::string.
         */
        [[nodiscard]] std::string full(JsonScopeBase const& scope) const ;

        // The actual key within the scope
        std::string_view key;

        // Optional given scope. The JsonScopeBase must have access to this scope when using the key.
        std::optional<std::string_view> givenScope = std::nullopt;

    public:
        // JsonScopeBase should be the only class able to convert scopedKey to full key
        friend class JsonScopeBase;

        // Accept any T that is constructible into std::string
        // We disable linting as the implicit conversion is intended here
        // TODO: Activate later on once the key usage is fully integrated
        //       Provide methods to add strings together to form full keys
        template<typename T, typename = std::enable_if_t<std::is_constructible_v<std::string_view, T>>>
        // NOLINTNEXTLINE
        scopedKey(T const& keyInScope)
            : key(std::string_view(keyInScope)) {}

        // To be used when we want to ensure that the key is used in a specific scope
        template<typename T, typename = std::enable_if_t<std::is_constructible_v<std::string_view, T>>>
        scopedKey(JsonScopeBase const& scope, T const& keyInScope)
            : key(std::string_view(keyInScope)), givenScope(scope.getScopePrefix()) {}

        // To be used for constexpr
        template<typename T, typename U>
        requires std::convertible_to<T, std::string_view> && std::convertible_to<U, std::string_view>
        constexpr scopedKey(U const& scope, T const& keyInScope) noexcept
            : key(std::string_view(keyInScope)), givenScope(std::string_view(scope)) {}
    };

    //------------------------------------------
    // Sharing a scope

    // Proper scope sharing with nested unscoped key generation
    [[nodiscard]] JsonScope& shareScope(scopedKey const& key) const {
        return baseDocument->shareManagedScope(key.full(*this));
    }

    //------------------------------------------
    // Getter

    template<typename T>
    T get(scopedKey const& key, T const& defaultValue = T()) const {
        return baseDocument->get<T>(key.full(*this), defaultValue);
    }

    [[nodiscard]] std::optional<RjDirectAccess::simpleValue> getVariant(scopedKey const& key) const {
        return baseDocument->getVariant(key.full(*this));
    }

    [[nodiscard]] JSON getSubDoc(scopedKey const& key) const {
        return baseDocument->getSubDoc(key.full(*this));
    }

    [[nodiscard]] double* getStableDoublePointer(scopedKey const& key) const {
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

    void setSubDoc(scopedKey const& key, JsonScopeBase const& subDoc) const {
        // Slightly more complicated: If we wish to set the sub-document from another JsonScopeBase,
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

    [[nodiscard]] std::scoped_lock<std::recursive_mutex> lock() const {
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

    [[nodiscard]] JSON::KeyType memberType(scopedKey const& key) const {
        return baseDocument->memberType(key.full(*this));
    }

    [[nodiscard]] size_t memberSize(scopedKey const& key) const {
        return baseDocument->memberSize(key.full(*this));
    }

    void removeKey(scopedKey const& key) const {
        baseDocument->removeKey(key.full(*this));
    }

    //------------------------------------------
    // Serialize/Deserialize

    [[nodiscard]] std::string serialize(scopedKey const& key = scopedKey("")) const {
        return baseDocument->serialize(key.full(*this));
    }

    virtual void deserialize(std::string const& serialOrLink);
};

/**
 * @brief Domain-ized JsonScope class
 * @details Inherits from Interaction::Execution::Domain to integrate with the Nebulite interaction system.
 *          Also inherits from JsonScopeBase to provide scoped JSON document access.
 */
class JsonScope final : public Interaction::Execution::Domain<JsonScope>, public JsonScopeBase {
public:
    //------------------------------------------
    // Constructors

    // Constructing a JsonScopeBase from a JSON document and a prefix
    JsonScope(JSON& doc, std::string const& prefix, std::string const& name = "Unnamed JsonScope");

    // Constructing a JsonScopeBase from another JsonScopeBase and a sub-prefix
    JsonScope(JsonScope const& other, std::string const& prefix, std::string const& name = "Unnamed JsonScope");

    // Default constructor, we create a self-owned empty JSON document
    explicit JsonScope(std::string const& name = "Unnamed JsonScope");

    //------------------------------------------
    // Special member functions

    JsonScope(JsonScope const& other);
    JsonScope(JsonScope&& other) noexcept;
    JsonScope& operator=(JsonScope const& other);
    JsonScope& operator=(JsonScope&& other) noexcept;

    ~JsonScope() override = default;

    //------------------------------------------
    // Domain related stuff

    Constants::Error update() override {
        updateModules();
        return Constants::ErrorTable::NONE();
    }

    //------------------------------------------
    // Overwrite deserialization to add token parsing

    void deserialize(std::string const& serialOrLink) override ;
};

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
