#ifndef NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_HPP
#define NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <memory>
#include <string>
#include <optional>
#include <mutex>
#include <thread>
#include <utility>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Data/Document/RjDirectAccess.hpp"
#include "Data/Document/KeyType.hpp"
#include "Data/OrderedDoublePointers.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Data {
class JSON;
class ScopedKey;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Data {

template<typename T, std::size_t N, typename Arg, std::size_t... I>
constexpr std::array<T, N> make_array_with_arg_impl(Arg&& arg, std::index_sequence<I...>) {
    return std::array<T, N>{ { (static_cast<void>(I), T(std::forward<Arg>(arg)))... } };
}

template<typename T, std::size_t N, typename Arg>
constexpr std::array<T, N> make_array_with_arg(Arg&& arg) {
    return make_array_with_arg_impl<T, N>(std::forward<Arg>(arg), std::make_index_sequence<N>{});
}

/**
 * @class Nebulite::Data::JsonScopeBase
 * @brief The JsonScopeBase class provides a scoped interface for accessing and modifying JSON documents.
 * @details It allows for modifications to a JSON document within a specific scope,
 *          that is a key-prefixed section of the document. This is useful for modular data management,
 *          where different parts of a JSON document can be managed independently.
 *          Holds little data itself, mostly acts as a scoped view over an existing JSON document or another JsonScope.
 * @todo Move to namespace Nebulite::Core where all other Domain-related classes are located.
 * @todo Use scoped keys in DomainModules to reduce accidental key misusage:
 *       JsonScope::scopedKey key = {"subkey_in_scope", getDoc().getScopePrefix()};
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
     */
    std::array<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS> expressionRefs;

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

    // Necessary helper for shareScope
    [[nodiscard]] JsonScopeBase& shareManagedScopeBase(std::string const& prefix) const ;

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

    // Disabled copy/move to avoid issues with Domain ownership and infinite recursion

    JsonScopeBase(JsonScopeBase const& other) = delete;
    JsonScopeBase(JsonScopeBase&& other) noexcept = delete;
    JsonScopeBase& operator=(JsonScopeBase const& other) = delete;
    JsonScopeBase& operator=(JsonScopeBase&& other) noexcept = delete;

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

    [[nodiscard]] ScopedKeyView getRootScope() const {
        return ScopedKeyView{getScopePrefix(), ""};
    }

    //------------------------------------------
    // Sharing a scope

    [[nodiscard]] JsonScopeBase& shareScopeBase(ScopedKeyView const& key) const ;
    [[nodiscard]] JsonScopeBase& shareScopeBase(ScopedKey const& prefix) const {return shareScopeBase(prefix.view());}

    //------------------------------------------
    // Getter

    template<typename T> T get(ScopedKeyView const& key, T const& defaultValue = T()) const ;
    template<typename T> T get(ScopedKey const& key, T const& defaultValue = T()) const {return get<T>(key.view(), defaultValue);}

    [[nodiscard]] std::optional<RjDirectAccess::simpleValue> getVariant(ScopedKeyView const& key) const ;
    [[nodiscard]] std::optional<RjDirectAccess::simpleValue> getVariant(ScopedKey const& key) const {return getVariant(key.view());}

    [[nodiscard]] JSON getSubDoc(ScopedKeyView const& key) const ;
    [[nodiscard]] JSON getSubDoc(ScopedKey const& key) const ;

    [[nodiscard]] double* getStableDoublePointer(ScopedKeyView const& key) const ;
    [[nodiscard]] double* getStableDoublePointer(ScopedKey const& key) const {return getStableDoublePointer(key.view());}

    //------------------------------------------
    // Setter

    template<typename T> void set(ScopedKeyView const& key, T const& value);
    template<typename T> void set(ScopedKey const& key, T const& value){set(key.view(), value);}

    void setVariant(ScopedKeyView const& key, RjDirectAccess::simpleValue const& value) const ;
    void setVariant(ScopedKey const& key, RjDirectAccess::simpleValue const& value) const {setVariant(key.view(), value);}

    void setSubDoc(ScopedKeyView const& key, JSON const& subDoc) const ;
    void setSubDoc(ScopedKey const& key, JSON const& subDoc) const {setSubDoc(key.view(), subDoc);}

    void setSubDoc(ScopedKeyView const& key, JsonScopeBase const& subDoc) const ;
    void setSubDoc(ScopedKey const& key, JsonScopeBase const& subDoc) const {setSubDoc(key.view(), subDoc);}

    void setEmptyArray(ScopedKeyView const& key) const ;
    void setEmptyArray(ScopedKey const& key) const {setEmptyArray(key.view());}

    //------------------------------------------
    // Special sets for threadsafe maths operations

    void set_add(ScopedKeyView const& key, double const& val) const ;
    void set_add(ScopedKey const& key, double const& val) const {set_add(key.view(), val);}

    void set_multiply(ScopedKeyView const& key, double const& val) const ;
    void set_multiply(ScopedKey const& key, double const& val) const {set_multiply(key.view(), val);}

    void set_concat(ScopedKeyView const& key, std::string const& valStr) const ;
    void set_concat(ScopedKey const& key, std::string const& valStr) const {set_concat(key.view(), valStr);}

    //------------------------------------------
    // Locking

    [[nodiscard]] std::scoped_lock<std::recursive_mutex> lock() const ;

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

    [[nodiscard]] KeyType memberType(ScopedKeyView const& key) const ;
    [[nodiscard]] KeyType memberType(ScopedKey const& key) const {return memberType(key.view());}

    [[nodiscard]] size_t memberSize(ScopedKeyView const& key) const ;
    [[nodiscard]] size_t memberSize(ScopedKey const& key) const {return memberSize(key.view());}

    void removeKey(ScopedKeyView const& key) const ;
    void removeKey(ScopedKey const& key) const {removeKey(key.view());}

    //------------------------------------------
    // Serialize/Deserialize

    [[nodiscard]] std::string serialize() const ;

    [[nodiscard]] std::string serialize(ScopedKeyView const& key) const ;
    [[nodiscard]] std::string serialize(ScopedKey const& key) const {return serialize(key.view());}

    virtual void deserialize(std::string const& serialOrLink);
};
} // namespace Nebulite::Data
#include "Data/Document/JsonScopeBase.tpp"
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_HPP
