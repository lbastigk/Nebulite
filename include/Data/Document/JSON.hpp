/**
 * @file JSON.hpp
 * @brief A wrapper around rapidjson to simplify JSON manipulation in Nebulite.
 */

#ifndef NEBULITE_DATA_JSON_HPP
#define NEBULITE_DATA_JSON_HPP

//------------------------------------------
// Includes

// Standard library
#include <expected>
#include <mutex>
#include <string>
#include <variant>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Constants/Alignment.hpp"
#include "Data/Document/KeyType.hpp"
#include "Data/Document/JsonRvalueTransformer.hpp"
#include "Data/Document/RjDirectAccess.hpp"
#include "Data/Document/SimpleValueError.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Data {
/**
 * @class JSON
 * @brief A wrapper around rapidjson to simplify JSON manipulation in Nebulite.
 * @details Features:
 *          - caching for fast access to frequently used values,
 *          - stable double pointers for even faster access in math-heavy scenarios
 *          - easy to use set/get methods with type conversion
 *          - serialize/deserialize methods for easy saving/loading
 *          - member type and size checking
 *          - usage of return value transformations on get (length, type checks, arithmetic operations, etc.)
 *          - thread-safe access with mutex locking
 *          - optimized for performance using ordered double pointers and quick cache for unique IDs,
 *            allowing fast access to numeric values in a sorted manner.
 */
class JSON {
public:
    //------------------------------------------
    // Basic public constants

    /**
     * @brief A list of reserved characters that cannot be used in key names.
     * @details - '[]' : Used for array indexing.
     *          - '.'  : Used for nested object traversal.
     *          - '|'  : Used for piping transformations.
     *          - '"'  : Used for string encapsulation.
     *          - ':'  : Used for Read-Only docs to separate link and key.
     * @todo Proper API documentation for JSON key naming rules.
     *       Including a 'why' for each character.
     */
    static auto constexpr reservedCharacters = "[]{}.|\":";

private:
    static double constexpr standardNumericValue = 0.0;

    /**
     * @brief The amount of pre-cached double values per Document.
     */
    static auto constexpr CACHELINE_SIZE = 1024 / sizeof(double);

    /**
     * @brief Pre-allocated cacheline for fast double value access.
     * @details Instead of always allocating new double values, we use a pre-allocated cacheline.
     *          This reduces memory fragmentation and improves cache locality.
     */
    alignas(Constants::Alignment::SIMD_ALIGN) mutable std::array<double, CACHELINE_SIZE> CACHELINE = {0.0};

    /**
     * @brief Current index in the cacheline for the next double value.
     */
    mutable size_t cacheline_index = 0;

    /**
     * @struct CacheEntry
     * @brief Represents a cached entry in the JSON document, including its value, state, and stable pointer for double values.
     */
    struct CacheEntry {
        /**
         * @enum EntryState
         * @brief Represents the state of a cached entry in the JSON document.
         *        How it works:
         *        - On reloading a full document, all entries become DELETED.
         *        - If we access a double pointer of a deleted/nonexistent value, we mark the entry as VIRTUAL,
         *          as it's a resurrected entry, but its potentially not the real value due to casting.
         *        - A value becomes DIRTY if it was previously CLEAN, and we notice a change in its double value.
         *        - On flushing, all DIRTY entries become CLEAN again. VIRTUAL entries remain VIRTUAL as they are not flushed.
         *        - Values may be marked DELETED if their parent is modified or deleted.
         */
        enum class EntryState : uint8_t {
            CLEAN, // Synchronized with RapidJSON document, real value. NOTE: This may be invalid at any time if double pointer is used elsewhere! This just marks the last known state.
            DIRTY, // Modified in cache, needs flushing to RapidJSON, real value
            DERIVED, // Deleted/nonexistent entry that was accessed via double pointer
            DELETED, // Deleted entry due to deserialization, inner value is invalid
            MALFORMED // A key that is known to be malformed due to transformations. Used in getStableDoublePointer for integrity.
        };

        //------------------------------------------
        // No copying or moving

        CacheEntry(CacheEntry const&) = delete;
        CacheEntry& operator=(CacheEntry const&) = delete;
        CacheEntry(CacheEntry&&) = delete;
        CacheEntry& operator=(CacheEntry&&) = delete;

        //------------------------------------------
        // Data members

        RjDirectAccess::simpleValue value = standardNumericValue;
        double last_double_value = standardNumericValue;
        double* stable_double_ptr = nullptr; // Stable pointer to double value
        EntryState state = EntryState::DIRTY; // Default to dirty: each new entry needs flushing
        bool managedInternalDouble = false; // Whether the stable double pointer is managed internally or externally (from cacheline)

        CacheEntry(std::array<double, CACHELINE_SIZE>& cacheLine, size_t& index) {
            if (index >= CACHELINE_SIZE) [[unlikely]] {
                stable_double_ptr = new double(standardNumericValue);
                managedInternalDouble = true;
            }
            else [[likely]] {
                // Assign stable double pointer from cacheline
                stable_double_ptr = &cacheLine[index++];
                *stable_double_ptr = standardNumericValue;
                managedInternalDouble = false;
            }
        }

        ~CacheEntry() {
            if (managedInternalDouble) {
                delete stable_double_ptr;
            }
        }
    };

    /**
     * @brief The Caching system used for fast access to frequently used values.
     * @details Is mutable, as caching itself is used in get-calls, which are const.
     */
    mutable absl::flat_hash_map<std::string, std::unique_ptr<CacheEntry>> cache;

    /**
     * @brief A helper variable that is modified to signal certain functions as non-const.
     */
    int64_t helperNonConstVar = 0;

    /**
     * @brief The underlying RapidJSON document.
     * @details Is mutable, as we regularly need to flush contents into it from const get-calls.
     */
    mutable rapidjson::Document doc;

    // Mutex for thread safety
    mutable std::recursive_mutex mtx;

    /**
     * @brief Inserts a rapidjson value into the cache, converting it to the appropriate C++ type.
     * @param key The key of the value to cache.
     * @param val The rapidjson value to cache.
     * @return The converted value of type T, or nullopt if conversion fails or the value is not cacheable.
     */
    template <typename T>
    std::optional<T> jsonValueToCache(std::string const& key, rapidjson::Value const* val) const ;

    /**
     * @brief Synchronizes all children of a given key.
     * @details For example, if parent_key is "config", it will sync
     *          "config.option1", "config.option2.suboption", etc.
     *          as well as "config[0]", "config[1].suboption", etc.
     *          with the rapidjson values.
     */
    void synchronizeChildren(std::string const& parentKey) const ;

    /**
     * @brief Helper function to convert any type from cache into another type.
     * @param var The variant value stored in the cache.
     * @return The converted value of type newType, or nullopt if conversion fails.
     */
    template <typename newType>
    static std::optional<newType> convertVariant(RjDirectAccess::simpleValue const& var);

    /**
     * @brief Flush all DIRTY entries in the cache back to the RapidJSON document.
     * @details This ensures that the RapidJSON document is always structurally valid
     *          and up-to-date with the cached values.
     * @todo Consider adding an optional prefix parameter to only flush a subset of the cache.
     */
    void flush() const ;

    //------------------------------------------
    // Return Value Transformation system

    /**
     * @brief Lazy-initialized instance of the JsonRvalueTransformer for applying transformations on get operations.
     * @return Reference to the JsonRvalueTransformer instance.
     */
    static JsonRvalueTransformer* getTransformer();

    /**
     * @brief Apply transformations found in the key string and retrieve the modified value.
     * @tparam T The type of the value to retrieve.
     * @param key The key string containing transformations.
     * @return The modified value of type T, or none on failure.
     */
    template <typename T>
    std::expected<T, SimpleValueRetrievalError> getWithTransformations(std::string const& key) const ;

    /**
     * @brief Apply transformations found in the key string and retrieve the modified document.
     * @param key The key string containing transformations.
     * @param outDoc The output JSON document to store the modified result.
     * @return True on success, false on failure.
     * @note We use an external outDoc to avoid copying the entire document on return/on optional::getValue().
     */
    bool getSubDocWithTransformations(std::string const& key, JSON& outDoc) const ;

    //------------------------------------------
    // Scope sharing system

    absl::flat_hash_map<std::string, std::unique_ptr<JsonScope>> managedScopeBases;
    std::unique_ptr<JsonScope> fullScopeBaseInstance;
    std::unique_ptr<JsonScope> dummyScopeBaseInstance;

public:
    //------------------------------------------
    // Assertions

    // Make sure cache size is a power of two for optimal performance
    static_assert(Utility::CompileTimeEvaluate::isPowerOfTwo(CACHELINE_SIZE), "CACHELINE_SIZE must be a power of two for optimal performance.");

    //------------------------------------------
    // Constructor/Destructor

    /**
     * @brief Constructs a new JSON document.
     */
    JSON();

    ~JSON();

    //------------------------------------------
    // Overload of assign operators

    // No copy
    JSON(JSON const&) = delete;
    JSON& operator=(JSON const&) = delete;

    // Allow move
    JSON(JSON&& other) noexcept;
    JSON& operator=(JSON&& other) noexcept;

    //------------------------------------------
    // Scope sharing

    /**
     * @brief Lazy-initialized full JsonScope representing the entire document.
     * @return Reference to the full JsonScope.
     */
    JsonScope& fullScopeBase();

    /**
     * @brief Shares part JSON document as a JsonScope that is managed internally.
     * @details Sometimes we cannot use full JsonScopes due circular issues, causing a cascade of Domain initializations.
     *          In those cases, use JsonScope via this method.
     * @param prefix The prefix representing the part of the JSON document to share.
     *               If empty, shares the entire document.
     * @return A JsonScope reference representing a part of the JSON document.
     */
    JsonScope& shareManagedScopeBase(std::string const& prefix = "");
    JsonScope& shareManagedScopeBase(std::string_view const& prefix) { return shareManagedScopeBase(std::string(prefix)); }
    JsonScope& shareManagedScopeBase(char const* prefix) { return shareManagedScopeBase(std::string(prefix)); }

    JsonScope& getDummyScopeBase();

    //------------------------------------------
    // Custom copy method

    /**
     * @brief Copies the entire content from another JSON document into this one.
     * @param other The other JSON document to copy from.
     */
    void copyFrom(JSON const& other);

    //------------------------------------------
    // Validity check

    /**
     * @brief Checks if a string is in JSON or JSONC format.
     * @param str The string to check.
     * @return true if the string is JSON or JSONC, false otherwise.
     */
    static bool isJsonOrJsonc(std::string const& str);

    //------------------------------------------
    // Argument splitting for transformations

    static std::vector<std::string> splitKeyWithTransformations(std::string const& key);

    //------------------------------------------
    // Set methods

    /**
     * @brief Sets a value of the specified type in the JSON document.
     * @details If the key already exists, the value and its stable double pointer
     *          is updated. Child keys are invalidated.
     * @tparam T The type of the value to set.
     * @param key The key of the value to set.
     * @param val The value to set.
     */
    template <typename T> void set(std::string const& key, T const& val);
    template <typename T> void set(std::string_view const& key, T const& val) { set<T>(std::string(key), val); }
    template <typename T> void set(char const* key, T const& val) { set<T>(std::string(key), val); }

    /**
     * @brief Sets a variant value of supported simple values in the JSON document.
     *        Stable double pointer of the value is updated as well. Child keys are invalidated.
     * @param key The key of the value to set.
     * @param val The variant value to set.
     */
    void setVariant(std::string const& key, RjDirectAccess::simpleValue const& val);
    void setVariant(std::string_view const& key, RjDirectAccess::simpleValue const& val) { setVariant(std::string(key), val); }
    void setVariant(char const* key, RjDirectAccess::simpleValue const& val) { setVariant(std::string(key), val); }

    /**
     * @brief Sets a sub-document in the JSON document.
     * @details If the key already exists, the sub-document is updated.
     *          Note that both the child and parent documents' caches are flushed before setting.
     * @param key The key of the sub-document to set.
     * @param child The sub-document to set.
     * @param childKey The key in the child document to set as the root of the sub-document. If empty, the entire child document is used.
     */
    void setSubDoc(char const* key, JSON const& child, char const* childKey = "") ;
    void setSubDoc(std::string const& key, JSON const& child, char const* childKey = "") { setSubDoc(key.c_str(), child, childKey); }
    void setSubDoc(std::string_view const& key, JSON const& child, char const* childKey = "") { setSubDoc(std::string(key).c_str(), child, childKey); }

    /**
     * @brief Sets an empty array in the JSON document.
     * @details This function sets an empty array in the JSON document.
     *          If the key already exists, the array is updated.
     *          Note that the document is flushed before setting.
     * @param key The key of the array to set.
     */
    void setEmptyArray(char const* key);
    void setEmptyArray(std::string const& key) { setEmptyArray(key.c_str()); }
    void setEmptyArray(std::string_view const& key) { setEmptyArray(std::string(key).c_str()); }

    //------------------------------------------
    // Special sets for threadsafe maths operations

    /**
     * @brief Performs an addition operation on a numeric value in the JSON document.
     */
    void set_add(std::string_view const& key, double const& val);

    /**
     * @brief Performs a multiplication operation on a numeric value in the JSON document.
     */
    void set_multiply(std::string_view const& key, double const& val);

    /**
     * @brief Performs a concatenation operation on a string value in the JSON document.
     */
    void set_concat(std::string_view const& key, std::string const& valStr);

    //------------------------------------------
    // Get methods

    /**
     * @brief Gets a value from the JSON document.
     * @details This function retrieves a value of the specified type from the JSON document.
     *          If the key does not exist, the default value is returned.
     * @tparam T The type of the value to retrieve.
     * @param key The key of the value to retrieve.
     * @return The value associated with the key, or an error.
     */
    template <typename T> std::expected<T, SimpleValueRetrievalError> get(std::string const& key) const;
    template <typename T> std::expected<T, SimpleValueRetrievalError> get(std::string_view const& key) const { return get<T>(std::string(key)); }
    template <typename T> std::expected<T, SimpleValueRetrievalError> get(char const* key) const { return get<T>(std::string(key)); }

    /**
     * @brief Gets a variant value from the JSON document.
     * @details This function retrieves a variant value from the JSON document.
     *          If the key does not exist, void is returned.
     * @param key The key of the value to retrieve.
     * @return The variant value associated with the key, or an error if the retrieval failed.
     */
    std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> getVariant(std::string const& key) const ;
    std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> getVariant(std::string_view const& key) const { return getVariant(std::string(key)); }
    std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> getVariant(char const* key) const { return getVariant(std::string(key)); }

    /**
     * @brief Gets a sub-document from the JSON document.
     * @details If the key does not exist, an empty JSON object is returned.
     *          Note that the cache is flushed into the document.
     *          If the key is a basic type, its value is returned.
     *          You may use `memberType("")` to check the type stored in the JSON.
     *          You may use `get<T>("",T())` on the returned sub-document to get the simple value.
     * @param key The key of the sub-document to retrieve.
     * @return The sub-document associated with the key, or an empty JSON object if the key does not exist.
     */
    JSON getSubDoc(std::string const& key) const ;
    JSON getSubDoc(std::string_view const& key) const { return getSubDoc(std::string(key)); }
    JSON getSubDoc(char const* key) const { return getSubDoc(std::string(key)); }

    /**
     * @brief Gets a pointer to a double value pointer in the JSON document.
     * @return A pointer to the double value associated with the key.
     */
    double* getStableDoublePointer(std::string const& key) const;
    double* getStableDoublePointer(std::string_view const& key) const { return getStableDoublePointer(std::string(key)); }
    double* getStableDoublePointer(char const* key) const { return getStableDoublePointer(std::string(key)); }

    /**
     * @brief Provides access to the internal mutex for thread-safe operations.
     */
    std::unique_lock<std::recursive_mutex> lock() const ;

    //------------------------------------------
    // Key Types, Sizes

    /**
     * @brief Checks the type stored of a key in the JSON document.
     * @details This function checks the type stored of a key in the JSON document.
     *          If the key does not exist, the type is considered null.
     * @param key The key to check.
     * @return The type of the key.
     */
    KeyType memberType(std::string const& key) const ;
    KeyType memberType(std::string_view const& key) const { return memberType(std::string(key)); }
    KeyType memberType(char const* key) const { return memberType(std::string(key)); }

    /**
     * @brief Checks the type of the key in the JSON document and returns it as a string.
     * @details This function checks the type of the key in the JSON document and returns it as a string.
     *          If the key does not exist, the type is considered "null".
     *          Returned type strings:
     *          - "null" : Key does not exist or is null.
     *          - "value:<type>:<size>" : Key exists and is a simple value (number, string)
     *          - "value:<type>" : If no size is applicable (e.g. for bool), or if we want to omit size info for simplicity.
     *          - "array:<size>" : Key exists and is an array.
     *          - "object:<size>" : Key exists and is an object. Size is the number of members in the object.
     *          Examples:
     *          - "value:int:32" : A 32-bit integer value.
     *          - "value:float:64" : A 64-bit double value.
     *          - "value:string:10" : A string value with length 10.
     *          - "object:5" : An object with 5 members.
     * @param key The key to check.
     * @return The type of the key as a string.
     */
    std::string memberTypeString(std::string const& key) const ;
    std::string memberTypeString(std::string_view const& key) const { return memberTypeString(std::string(key)); }
    std::string memberTypeString(char const* key) const { return memberTypeString(std::string(key)); }

    /**
     * @brief Checks the size of a key in the JSON document.
     * @details If the key does not exist, the size is considered 0.
     *          If the key represents a document, the size is considered 1.
     * @param key The key to check.
     * @return The size of the key.
     */
    size_t memberSize(std::string const& key) const ;
    size_t memberSize(std::string_view const& key) const { return memberSize(std::string(key)); }
    size_t memberSize(char const* key) const { return memberSize(std::string(key)); }

    /**
     * @brief Removes a key from the JSON document.
     * @details If the key does not exist, no action is taken.
     *          Note that the document is flushed before removing the key.
     *          If an array element is removed, the remaining elements are shifted to fill the gap,
     *          and their keys are updated accordingly. This is important for loops that rely on array indices!
     *          If you must remove multiple array elements, consider starting at the end of the array and moving backwards to avoid index shifting issues.
     * @param key The key to remove.
     */
    void removeMember(char const* key);
    void removeMember(std::string const& key) { removeMember(key.c_str()); }
    void removeMember(std::string_view const& key) { removeMember(std::string(key).c_str()); }

    /**
     * @brief Moves a member from one key to another in the JSON document.
     * @details Note that the current implementation is more of a copy+delete! Using just copyMember is faster if you don't need the original deleted.
     * @param fromKey The key of the member to move.
     * @param toKey The key to move the member to.
     * @todo Optimize to a real move if possible.
     */
    void moveMember(char const* fromKey, char const* toKey);
    void moveMember(std::string const& fromKey, std::string const& toKey) { moveMember(fromKey.c_str(), toKey.c_str()); }
    void moveMember(std::string_view const& fromKey, std::string_view const& toKey) { moveMember(std::string(fromKey).c_str(), std::string(toKey).c_str()); }

    /**
     * @brief Copies a member from one key to another in the JSON document, without deleting the original.
     * @param fromKey The key of the member to copy.
     * @param toKey The key to copy the member to.
     */
    void copyMember(char const* fromKey, char const* toKey);
    void copyMember(std::string const& fromKey, std::string const& toKey) { copyMember(fromKey.c_str(), toKey.c_str()); }
    void copyMember(std::string_view const& fromKey, std::string_view const& toKey) { copyMember(std::string(fromKey).c_str(), std::string(toKey).c_str()); }

    /**
     * @brief Lists all available keys in a rapidjson object.
     * @param key The key to list keys from. (Optional: leave empty to list from root)
     * @return A vector of strings containing all available keys.
     *         - For objects, returns member names.
     *         - For arrays, returns indices in bracket notation (e.g., "[0]", "[1]", ...).
     *         - For any other type, returns an empty vector.
     */
    std::vector<std::string> listAvailableKeys(std::string const& key = "") const ;

    //------------------------------------------
    // Serialize/Deserialize

    /**
     * @brief Serializes the entire document or a portion of the document
     * @param key The key to serialize. (Optional: leave empty to serialize entire document)
     * @param type Type of serialization. Defaults to pretty printing.
     * @return The serialized JSON string.
     */
    std::string serialize(std::string const& key = "", RjDirectAccess::SerializationType const& type = RjDirectAccess::SerializationType::pretty) const ;

    /**
     * @brief Deserializes a JSON string or loads from a file, with optional modifications.
     * @param serialOrLink The JSON string to deserialize or the file path to load from + optional functioncall transformations
     *                     Examples:
     *                     - `{"key": "value"}` - Deserializes a JSON string
     *                     - `file.json` - Loads a JSON file
     *                     - `file.json|set key1.key2[5] 100` - Loads a JSON file and sets a value
     *                     - `file.json|key1.key2[5]=100` - Legacy feature for setting a value
     *                     - `file.json|set-from-json key1.key2[5] otherFile.json:key` - Sets key1.key2[5] from the value of key in otherFile.json
     */
    void deserialize(std::string const& serialOrLink);
};
} // namespace Nebulite::Data
#include "JSON.tpp"
#endif // NEBULITE_DATA_JSON_HPP
