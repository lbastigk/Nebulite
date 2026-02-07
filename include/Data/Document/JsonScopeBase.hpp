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
#include "Constants/Alignment.hpp"
#include "Constants/ThreadSettings.hpp"
#include "Data/Document/RjDirectAccess.hpp"
#include "Data/Document/KeyType.hpp"
#include "Data/OrderedDoublePointers.hpp"
#include "Utility/Threading.hpp"

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
 */
class JsonScopeBase {
protected:
    std::shared_ptr<JSON> baseDocument;

    /**
     * @brief A helper variable that is modified to signal certain functions as non-const.
     */
    int64_t helperNonConstVar = 0;

private:
    /**
     * @brief The Prefix of the scope. A nullopt indicates that this JsonScopeBase is a dummy (no access allowed).
     * @details Dummy scopes do not allow any access to the underlying JSON document.
     *          The retrieval of the scope prefix of a dummy scope will fail, exiting the program.
     */
    std::optional<std::string> scopePrefix;

    //------------------------------------------
    // Ordered double pointers system

    /**
     * @brief Mapped ordered double pointers for expression references.
     */
    alignas(Constants::Alignment::SIMD_ALIGN) std::array<MappedOrderedDoublePointers, ORDERED_DOUBLE_POINTERS_MAPS> expressionRefs;

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
     * @throws std::runtime_error if this is a dummy scope.
     */
    [[nodiscard]] std::string const& getScopePrefix() const {
        if (!scopePrefix.has_value()) {
            throw std::runtime_error("JsonScopeBase: Access not granted. Attempted to get scope prefix of a dummy scope. Did you mean to use the caller's scope?");
        }
        return scopePrefix.value();
    }

    [[nodiscard]] ScopedKeyView getRootScope() const {
        return ScopedKeyView{getScopePrefix(), ""};
    }

    //------------------------------------------
    // Sharing a scope (Base only, no Domain Functionality)

    // When requesting a scope to share, we always assume its relative to our current scope
    // So we can pass a key as string and generate the full key internally based on our scopePrefix

    [[nodiscard]] JsonScopeBase& shareScopeBase(std::string const& key) const ;

    [[nodiscard]] JsonScopeBase& shareDummyScopeBase() const ;

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

    void setVariant(ScopedKeyView const& key, RjDirectAccess::simpleValue const& value);
    void setVariant(ScopedKey const& key, RjDirectAccess::simpleValue const& value){setVariant(key.view(), value);}

    void setSubDoc(ScopedKeyView const& key, JSON const& subDoc);
    void setSubDoc(ScopedKey const& key, JSON const& subDoc){setSubDoc(key.view(), subDoc);}

    void setSubDoc(ScopedKeyView const& key, JsonScopeBase const& subDoc);
    void setSubDoc(ScopedKey const& key, JsonScopeBase const& subDoc){setSubDoc(key.view(), subDoc);}

    void setEmptyArray(ScopedKeyView const& key);
    void setEmptyArray(ScopedKey const& key){setEmptyArray(key.view());}

    //------------------------------------------
    // Special sets for threadsafe maths operations

    void set_add(ScopedKeyView const& key, double const& val);
    void set_add(ScopedKey const& key, double const& val) {set_add(key.view(), val);}

    void set_multiply(ScopedKeyView const& key, double const& val);
    void set_multiply(ScopedKey const& key, double const& val) {set_multiply(key.view(), val);}

    void set_concat(ScopedKeyView const& key, std::string const& valStr);
    void set_concat(ScopedKey const& key, std::string const& valStr) {set_concat(key.view(), valStr);}

    //------------------------------------------
    // Locking

    [[nodiscard]] std::scoped_lock<std::recursive_mutex> lock() const ;

    //------------------------------------------
    // Getters: Unique id based retrieval

    MappedOrderedDoublePointers* getOrderedCacheListMap() {
#if ORDERED_DOUBLE_POINTERS_MAPS == 1
        return &expressionRefs[0];
#else
        // Both versions are about equally performant according to benchmarks
        return &expressionRefs[Utility::Threading::threadIdToUniformDistribution(ORDERED_DOUBLE_POINTERS_MAPS)];
        //return &expressionRefs[Utility::Threading::atomicThreadRoll(ORDERED_DOUBLE_POINTERS_MAPS)];
#endif
    }

    //------------------------------------------
    // Key Types, Sizes

    [[nodiscard]] KeyType memberType(ScopedKeyView const& key) const ;
    [[nodiscard]] KeyType memberType(ScopedKey const& key) const {return memberType(key.view());}

    [[nodiscard]] std::string memberTypeString(ScopedKeyView const& key) const ;
    [[nodiscard]] std::string memberTypeString(ScopedKey const& key) const {return memberTypeString(key.view());}

    [[nodiscard]] size_t memberSize(ScopedKeyView const& key) const ;
    [[nodiscard]] size_t memberSize(ScopedKey const& key) const {return memberSize(key.view());}

    void removeMember(ScopedKeyView const& key);
    void removeMember(ScopedKey const& key) {removeMember(key.view());}

    std::vector<ScopedKey> listAvailableKeys(ScopedKeyView const& key) const ;
    std::vector<ScopedKey> listAvailableKeys(ScopedKey const& key) const {return listAvailableKeys(key.view());}

    // Helper struct for holding a member as well as its full key
    struct MemberAndKey {
        std::string member;
        ScopedKey key;

        MemberAndKey(std::string const& member_, ScopedKey const& key_) : member(member_), key(key_) {}
    };

    std::vector<MemberAndKey> listAvailableMembersAndKeys(ScopedKeyView const& key) const ;
    std::vector<MemberAndKey> listAvailableMembersAndKeys(ScopedKey const& key) const {return listAvailableMembersAndKeys(key.view());}

    //------------------------------------------
    // Serialize/Deserialize

    [[nodiscard]] std::string serialize() const ;
    [[nodiscard]] std::string serialize(ScopedKeyView const& key) const ;
    [[nodiscard]] std::string serialize(ScopedKey const& key) const {return serialize(key.view());}

    virtual void deserialize(std::string const& serialOrLink);

    //------------------------------------------
    // Access test

    /**
     * @brief Asserts access to the given key within this scope.
     * @details If access is not granted, this function will throw a runtime error.
     *          This may be used in any class constructors or functions that require guaranteed access to certain keys,
     *          to catch access violations early.
     * @param key The scoped key to check access for.
     */
    void assertAccess(ScopedKeyView const& key) const ;
    void assertAccess(ScopedKey const& key) const { assertAccess(key.view()); }
};
} // namespace Nebulite::Data
#include "Data/Document/JsonScopeBase.tpp"
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_HPP
