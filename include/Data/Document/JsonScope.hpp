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
 */
NEBULITE_DOMAIN(JsonScope) {
    std::variant<std::shared_ptr<JSON>, std::reference_wrapper<JsonScope>> baseDocument;
    std::string scopePrefix;

    //------------------------------------------
    // Helper for std::visit

    template<typename F>
    decltype(auto) visitBase(F&& f) {
        return std::visit([&]<typename Alt>(Alt&& alt) -> decltype(auto) {
            using AltT = std::decay_t<Alt>;
            if constexpr (std::is_same_v<AltT, std::shared_ptr<JSON>>) {
                return std::invoke(std::forward<F>(f), *alt);        // JSON&
            } else {
                return std::invoke(std::forward<F>(f), alt.get());   // JsonScope&
            }
        }, baseDocument);
    }

    template<typename F>
    decltype(auto) visitBase(F&& f) const {
        return std::visit([&]<typename Alt>(Alt&& alt) -> decltype(auto) {
            using AltT = std::decay_t<Alt>;
            if constexpr (std::is_same_v<AltT, std::shared_ptr<JSON>>) {
                return std::invoke(std::forward<F>(f), *alt);        // JSON&
            } else {
                return std::invoke(std::forward<F>(f), alt.get());   // JsonScope&
            }
        }, baseDocument);
    }

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
    JsonScope& shareManagedScope(std::string const& prefix) {
        return shareScope(prefix);
    }

public:
    //------------------------------------------
    // Constructors

    // Constructing a JsonScope from a JSON document and a prefix
    JsonScope(JSON& doc, std::string const& prefix, std::string const& name = "Unnamed JsonScope");

    // Constructing a JsonScope from another JsonScope and a sub-prefix
    JsonScope(JsonScope& other, std::string const& prefix, std::string const& name = "Unnamed JsonScope");

    // Default constructor, we create a self-owned empty JSON document
    JsonScope(std::string const& name = "Unnamed JsonScope");

    //------------------------------------------
    // Special member functions

    // TODO: Implement all copy/move semantics properly

    JsonScope(JsonScope const& other);
    JsonScope(JsonScope&& other) noexcept;
    JsonScope& operator=(JsonScope const& other);
    JsonScope& operator=(JsonScope&& other) noexcept;

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
        // JsonScope should be the only class able to convert unscopedKey to full key
        friend class JsonScope;

        // Accept any T that is constructible into std::string_view
        template<typename T, typename = std::enable_if_t<std::is_constructible_v<std::string_view, T>>>
        unscopedKey(T const& k) : key(std::string_view(k)) {}

        // Disable copying and moving to ensure safety of the string_view
        unscopedKey(unscopedKey const&) = delete;
        unscopedKey& operator=(unscopedKey const&) = delete;
        unscopedKey(unscopedKey&&) = delete;
        unscopedKey& operator=(unscopedKey&&) = delete;

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
        updateModules();
        return Constants::ErrorTable::NONE();
    }

    //------------------------------------------
    // Sharing a scope

    // Proper scope sharing with nested unscoped key generation
    JsonScope& shareScope(unscopedKey const& key) {
        return visitBase([&](auto& alt) -> JsonScope& {
            return alt.shareManagedScope(key.full(this));
        });
    }

    //------------------------------------------
    // Getter

    template<typename T>
    T get(unscopedKey const& key, T const& defaultValue = T()) const {
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

    void setSubDoc(unscopedKey const& key, JsonScope const& subDoc) {
        // Slightly more complicated: If we wish to set the sub-document from another JsonScope,
        // we need to extract the underlying JSON document from it in the correct scope.
        JSON subDocScope = subDoc.getSubDoc("");
        visitBase([&](auto& alt) -> void {
            alt.setSubDoc(key.full(this), subDocScope);
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

    void deserialize(std::string const& serialOrLink);
};
} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_HPP
