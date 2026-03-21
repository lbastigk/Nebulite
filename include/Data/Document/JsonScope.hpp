#ifndef NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_HPP
#define NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <expected>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

// Nebulite
#include "Constants/Alignment.hpp"
#include "Constants/ThreadSettings.hpp"
#include "Data/Document/RjDirectAccess.hpp"
#include "Data/Document/KeyType.hpp"
#include "Data/Document/SimpleValueError.hpp"
#include "Data/OrderedDoublePointers.hpp"
#include "Utility/Coordination/IdGenerator.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Data {
class JSON;
class ScopedKey;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Data {
/**
 * @class Nebulite::Data::JsonScope
 * @brief The JsonScope class provides a scoped interface for accessing and modifying JSON documents.
 * @details It allows for modifications to a JSON document within a specific scope,
 *          that is a key-prefixed section of the document. This is useful for modular data management,
 *          where different parts of a JSON document can be managed independently.
 *          Holds little data itself, mostly acts as a scoped view over an existing JSON document or another JsonScope.
 */
class JsonScope final {
public:
    // Thread runners are unique, no locking needed
    static auto constexpr noLockArraySize = Constants::ThreadSettings::Maximum::totalThreadCount + 4; // A bit extra, just in case

protected:
    std::shared_ptr<JSON> baseDocument;

    /**
     * @brief A helper variable that is modified to signal certain functions as non-const.
     */
    int64_t helperNonConstVar = 0;

private:
    /**
     * @brief The Prefix of the scope. A nullopt indicates that this JsonScope is a dummy (no access allowed).
     * @details Dummy scopes do not allow any access to the underlying JSON document.
     *          The retrieval of the scope prefix of a dummy scope will fail, exiting the program.
     */
    std::optional<std::string> scopePrefix;

    //------------------------------------------
    // Ordered double pointers system

    /**
     * @brief Mapped ordered double pointers intended for non-locking access
     */
    alignas(Constants::Alignment::SIMD_ALIGN) std::array<MappedOrderedDoublePointers, noLockArraySize> odpCache;

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

    // Constructing a JsonScope from a JSON document and a prefix
    explicit JsonScope(JSON& doc, std::optional<std::string> const& prefix);

    // Constructing a JsonScope from another JsonScope and a sub-prefix
    explicit JsonScope(JsonScope const& other, std::optional<std::string> const& prefix);

    // Default constructor, we create a self-owned empty JSON document
    explicit JsonScope();

    //------------------------------------------
    // Special member functions

    // Disabled copy/move to avoid issues with Domain ownership and infinite recursion

    JsonScope(JsonScope const& other) = delete;
    JsonScope(JsonScope&& other) noexcept = delete;
    JsonScope& operator=(JsonScope const& other) = delete;
    JsonScope& operator=(JsonScope&& other) noexcept = delete;

    ~JsonScope();

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
            throw std::runtime_error("JsonScope: Access not granted. Attempted to get scope prefix of a dummy scope. Did you mean to use the caller's scope?");
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

    [[nodiscard]] JsonScope& shareScopeBase(ScopedKeyView const& key) const ;
    [[nodiscard]] JsonScope& shareScopeBase(ScopedKey const& key) const {return shareScopeBase(key.view());}
    [[nodiscard]] JsonScope& shareScopeBase(std::string const& key) const ;

    // Share a dummy scope, where all access is denied
    [[nodiscard]] JsonScope& shareDummyScopeBase() ;

    //------------------------------------------
    // Getter

    template<typename T> std::expected<T, SimpleValueRetrievalError> get(ScopedKeyView const& key) const ;
    template<typename T> std::expected<T, SimpleValueRetrievalError> get(ScopedKey const& key) const {return get<T>(key.view());}

    [[nodiscard]] std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> getVariant(ScopedKeyView const& key) const ;
    [[nodiscard]] std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> getVariant(ScopedKey const& key) const {return getVariant(key.view());}

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

    void setSubDoc(ScopedKeyView const& key, JsonScope const& subDoc);
    void setSubDoc(ScopedKey const& key, JsonScope const& subDoc){setSubDoc(key.view(), subDoc);}

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

    [[nodiscard]] std::unique_lock<std::recursive_mutex> lock() const ;

    //------------------------------------------
    // Extra fast ordered cache list retrieval with minimal locking

    static size_t assignThreadIndex() {
        static auto indexCounter = Utility::Coordination::IdGenerator::atomicThreadIncrementGenerator();
        thread_local size_t threadIndex = indexCounter();
        return threadIndex;
    }

    odpvec* ensureOrderedCacheList(uint64_t const& uniqueId, std::vector<ScopedKeyView> const& keys) {
        thread_local size_t threadIndex = assignThreadIndex();
        if (threadIndex >= noLockArraySize) {
            throw std::runtime_error("Thread index exceeds non-locking array size! Too many threads accessing ordered cache lists, increase noLockArraySize or reduce thread count.");
        }
        return odpCache[threadIndex].ensureOrderedCacheListNoLock(uniqueId, keys);
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

    void moveMember(ScopedKeyView const& fromKey, ScopedKeyView const& toKey);
    void moveMember(ScopedKey const& fromKey, ScopedKey const& toKey) {moveMember(fromKey.view(), toKey.view());}
    void moveMember(ScopedKeyView const& fromKey, ScopedKey const& toKey) {moveMember(fromKey, toKey.view());}
    void moveMember(ScopedKey const& fromKey, ScopedKeyView const& toKey) {moveMember(fromKey.view(), toKey);}

    void copyMember(ScopedKeyView const& fromKey, ScopedKeyView const& toKey);
    void copyMember(ScopedKey const& fromKey, ScopedKey const& toKey) {copyMember(fromKey.view(), toKey.view());}
    void copyMember(ScopedKeyView const& fromKey, ScopedKey const& toKey) {copyMember(fromKey, toKey.view());}
    void copyMember(ScopedKey const& fromKey, ScopedKeyView const& toKey) {copyMember(fromKey.view(), toKey);}

    [[nodiscard]] std::vector<ScopedKey> listAvailableKeys(ScopedKeyView const& key) const ;
    [[nodiscard]] std::vector<ScopedKey> listAvailableKeys(ScopedKey const& key) const {return listAvailableKeys(key.view());}

    // Helper struct for holding a member as well as its full key
    struct MemberAndKey {
        std::string member;
        ScopedKey key;
    };

    [[nodiscard]] std::vector<MemberAndKey> listAvailableMembersAndKeys(ScopedKeyView const& key) const ;
    [[nodiscard]] std::vector<MemberAndKey> listAvailableMembersAndKeys(ScopedKey const& key) const {return listAvailableMembersAndKeys(key.view());}

    //------------------------------------------
    // Serialize/Deserialize

    [[nodiscard]] std::string serialize() const ;
    [[nodiscard]] std::string serialize(ScopedKeyView const& key) const ;
    [[nodiscard]] std::string serialize(ScopedKey const& key) const {return serialize(key.view());}

    void deserialize(std::string const& serialOrLink) const ;

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
#include "Data/Document/JsonScope.tpp"
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_HPP
