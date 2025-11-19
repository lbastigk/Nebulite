/**
 * @file JSON.hpp
 * @brief A wrapper around rapidjson to simplify JSON manipulation in Nebulite.
 */

#ifndef NEBULITE_UTILITY_JSON_HPP
#define NEBULITE_UTILITY_JSON_HPP

//------------------------------------------
// Includes

// Standard library
#include <cfloat>
#include <cmath>
#include <cxxabi.h>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <variant>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/JsonModifier.hpp"
#include "Utility/OrderedDoublePointers.hpp"
#include "Utility/RjDirectAccess.hpp"

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class JSON
 * @brief A wrapper around rapidjson to simplify JSON manipulation in Nebulite.
 *        Features:
 *        - caching for fast access to frequently used values,
 *        - stable double pointers for even faster access in math-heavy scenarios
 *        - easy to use set/get methods with type conversion
 *        - serialize/deserialize methods for easy saving/loading
 *        - member type and size checking
 *        - usage of parsing commands to modify JSON on load
 *        - usage of return value modifiers on get (length, type checks, arithmetic operations, etc.)
 *        - thread-safe access with mutex locking
 *        - optimized for performance using ordered double pointers and quick cache for unique IDs,
 *          allowing fast access to numeric values in a sorted manner.
 */
NEBULITE_DOMAIN(JSON) {
public:
    //------------------------------------------
    // Unique id Cache size
    // Declared above as public for easy access
    // and so that inner data structures can use it as well.

    /**
     * @brief Size of the unique ID quick cache for double pointers.
     */
    static constexpr size_t uidQuickCacheSize = 30;

private:
    /**
     * @enum EntryState
     * @brief Represents the state of a cached entry in the JSON document.
     *        - CLEAN: The entry is synchronized with the RapidJSON document and holds a real value.
     *        - DIRTY: The entry has been modified in the cache and needs to be flushed to the RapidJSON document.
     *        - VIRTUAL: The entry exists for pointer stability but does not have a real value set (defaults to 0).
     *        - DELETED: The entry is marked for deletion but is not removed from the cache to maintain pointer stability.
     *
     *        The basic idea is: On reloading a full document, all entries become DELETED. If we access a double pointer,
     *        we mark the entry as VIRTUAL, as it's a resurrected entry, but its potentially not the real value due to casting.
     *        A value becomes DIRTY if it was previously CLEAN, and we notice a change in its double value.
     *        On flushing, all DIRTY entries become CLEAN again. VIRTUAL entries remain VIRTUAL as they are not flushed.
     */
    enum class EntryState : uint8_t {
        CLEAN, // Synchronized with RapidJSON document, real value. NOTE: This may be invalid at any time if double pointer is used elsewhere! This just marks the last known state.
        DIRTY, // Modified in cache, needs flushing to RapidJSON, real value
        VIRTUAL, // Deleted entry that was re-synced, but may not be the real value due to casting
        DELETED // Deleted entry due to deserialization, inner value is invalid
    };

    /**
     * @struct CacheEntry
     * @brief Represents a cached entry in the JSON document, including its value, state, and stable pointer for double values.
     */
    struct CacheEntry {
        CacheEntry() = default;

        ~CacheEntry() {
            delete stable_double_ptr;
        }

        //------------------------------------------
        // No copying or moving

        CacheEntry(CacheEntry const&) = delete;
        CacheEntry& operator=(CacheEntry const&) = delete;
        CacheEntry(CacheEntry&&) = delete;
        CacheEntry& operator=(CacheEntry&&) = delete;

        //------------------------------------------
        // Data members

        RjDirectAccess::simpleValue value = 0.0; // Default virtual entries to 0
        double last_double_value = 0.0; // For change detection
        double* stable_double_ptr = new double(0.0); // Stable pointer to double value
        EntryState state = EntryState::DIRTY; // Default to dirty: each new entry needs flushing
    };

    /**
     * @brief Inserts a rapidjson value into the cache, converting it to the appropriate C++ type.
     * @param key The key of the value to cache.
     * @param val The rapidjson value to cache.
     * @param defaultValue The default value to use if conversion fails.
     */
    template <typename T>
    T jsonValueToCache(std::string const& key, rapidjson::Value const* val, T const& defaultValue);


    /**
     * @brief Invalidate all child keys of a given parent key.
     *        For example, if parent_key is "config", it will invalidate
     *        "config.option1", "config.option2.suboption", etc.
     *        as well as "config[0]", "config[1].suboption", etc.
     */
    void invalidate_child_keys(std::string const& parent_key);

    /**
     * @brief The Caching system used for fast access to frequently used values.
     */
    absl::flat_hash_map<std::string, std::unique_ptr<CacheEntry>> cache;

    // Callback type for cache invalidation notifications
    using InvalidationCallback = std::function<void(std::string const& key, double* old_ptr, double* new_ptr)>;

    // Actual RapidJSON document
    rapidjson::Document doc;

    // Mutex for thread safety
    std::recursive_mutex mtx;

    /**
     * @brief Helper function to convert any type from cache into another type.
     * @param var The variant value stored in the cache.
     * @param defaultValue The default value to return if conversion fails.
     * @return The converted value of type newType, or defaultValue on failure.
     * @todo The if-branches for string "true"/"false" to int/double conversions are currently commented out,
     *       as they cause issues when compiling on macOS. Reason unknown, says it can't convert to the respective type,
     *       even though it is explicitly the correct type. Needs further investigation.
     *       Most likely reason is that the compiler doesn't like the nested if-branch inside the constexpr if?
     *       Even a redundant static_cast to the return didn't help.
     */
    template <typename newType>
    newType convertVariant(RjDirectAccess::simpleValue const& var, newType const& defaultValue = newType{});

    /**
     * @brief Flush all DIRTY entries in the cache back to the RapidJSON document.
     *        This ensures that the RapidJSON document is always structurally valid
     *        and up-to-date with the cached values.
     */
    void flush();

    /**
     * @brief Structure to hold multiple maps for expression references.
     *        Currently, only reference "other" is used, but later on references like "parent" or "child" could be added.
     */
    struct ExpressionRefs {
        MappedOrderedDoublePointers as_other;
    } expressionRefs[ORDERED_DOUBLE_POINTERS_MAPS];

    /**
     * @brief Super quick double cache based on unique IDs, no hash lookup.
     *        Used for the first few entries. It's recommended to reserve
     *        low value uids for frequently used keys.
     * @todo Add a reserve-mechanism in globalspace for ids, so they are low value.
     */
    std::array<double*, uidQuickCacheSize> uidDoubleCache{nullptr};

    /**
     * @brief JSON Modifier instance for applying modifiers on get operations.
     */
    JsonModifier jsonModifier;

public:
    //------------------------------------------
    // Constructor/Destructor

    /**
     * @brief Constructs a new JSON document.
     * @param name The name of the JSON document. Recommended for debugging purposes.
     */
    JSON(std::string const& name = "Unnamed JSON Document");

    ~JSON() override;

    //------------------------------------------
    // Overload of assign operators

    JSON(JSON const&) = delete;
    JSON& operator=(JSON const&) = delete;
    JSON(JSON&& other) noexcept;
    JSON& operator=(JSON&& other) noexcept;

    //------------------------------------------
    // Static members

    /**
     * @brief A list of reserved characters that cannot be used in key names.
     *        - '[]' : Used for array indexing.
     *        - '.'  : Used for nested object traversal.
     *        - '|'  : Used for piping modifiers.
     *        - '"'  : Used for string encapsulation.
     * @todo Proper API documentation for JSON key naming rules.
     *       Including a 'why' for each character.
     */
    const static std::string reservedCharacters;

    //------------------------------------------
    // Domain-specific methods

    Constants::Error update() override;

    //------------------------------------------
    // Validity check

    /**
     * @brief Checks if a string is in JSON or JSONC format.
     * @param str The string to check.
     * @return true if the string is JSON or JSONC, false otherwise.
     */
    static bool is_json_or_jsonc(std::string const& str) {
        return RjDirectAccess::is_json_or_jsonc(str);
    }

    //------------------------------------------
    // Set methods

    /**
     * @brief Sets a value in the JSON document.
     *        This function sets a value of the specified type in the JSON document.
     *        If the key already exists, the value is updated.
     * @tparam T The type of the value to set.
     * @param key The key of the value to set.
     * @param val The value to set.
     */
    template <typename T>
    void set(std::string const& key, T const& val);

    /**
     * @brief Sets a sub-document in the JSON document.
     *        If the key already exists, the sub-document is updated.
     *        Note that both the child and parent documents' caches are flushed before setting.
     * @param key The key of the sub-document to set.
     * @param child The sub-document to set.
     */
    void setSubDoc(char const* key, JSON* child);

    /**
     * @brief Sets an empty array in the JSON document.
     *        This function sets an empty array in the JSON document.
     *        If the key already exists, the array is updated.
     *        Note that the document is flushed before setting.
     * @param key The key of the array to set.
     */
    void set_empty_array(char const* key);

    //------------------------------------------
    // Special sets for threadsafe maths operations

    /**
     * @brief Performs an addition operation on a numeric value in the JSON document.
     */
    void set_add(std::string const& key, double const& val);

    /**
     * @brief Performs a multiplication operation on a numeric value in the JSON document.
     */
    void set_multiply(std::string const& key, double const& val);

    /**
     * @brief Performs a concatenation operation on a string value in the JSON document.
     */
    void set_concat(std::string const& key, std::string const& valStr);

    //------------------------------------------
    // Get methods

    /**
     * @brief Gets a value from the JSON document.
     *        This function retrieves a value of the specified type from the JSON document.
     *        If the key does not exist, the default value is returned.
     * @tparam T The type of the value to retrieve.
     * @param key The key of the value to retrieve.
     * @param defaultValue The default value to return if the key does not exist.
     * @return The value associated with the key, or the default value if the key does not exist.
     */
    template <typename T>
    T get(std::string const& key, T const& defaultValue = T());

    /**
     * @brief Gets a sub-document from the JSON document.
     *        If the key does not exist, an empty JSON object is returned.
     *        Note that the cache is flushed into the document.
     * @param key The key of the sub-document to retrieve.
     * @return The sub-document associated with the key, or an empty JSON object if the key does not exist.
     */
    JSON getSubDoc(std::string const& key);

    /**
     * @brief Provides access to the internal mutex for thread-safe operations.
     *        Allowing modules to lock the JSON document.
     */
    std::scoped_lock<std::recursive_mutex> lock() { return std::scoped_lock(mtx); }

    /**
     * @brief Gets a pointer to a double value pointer in the JSON document.
     * @return A pointer to the double value associated with the key.
     */
    double* getStableDoublePointer(std::string const& key);

    /**
     * @brief Gets a pointer to a double value pointer in the JSON document based on a unique ID.
     * @param uid The unique ID of the key, must be smaller than uidQuickCacheSize !
     * @param key The key of the value to retrieve.
     * @return A pointer to the double value associated with the key.
     */
    double* getUidDoublePointer(uint64_t const& uid, std::string const& key) {
        if (uidDoubleCache[uid] == nullptr) {
            uidDoubleCache[uid] = getStableDoublePointer(key);
        }
        return uidDoubleCache[uid];
    }

    //------------------------------------------
    // Key Types, Sizes

    /**
     * @enum KeyType
     * @brief Enum representing the type stored of a key in the JSON document.
     */
    enum class KeyType : uint8_t {
        null,
        object,
        value,
        array
    };

    /**
     * @brief Checks the type stored of a key in the JSON document.
     *        This function checks the type stored of a key in the JSON document.
     *        If the key does not exist, the type is considered null.
     * @param key The key to check.
     * @return The type of the key.
     * @todo Make sure to return the correct type if modifiers are applied.
     */
    KeyType memberType(std::string const& key);

    /**
     * @brief Checks the size of a key in the JSON document.
     *        This function checks the size of a key in the JSON document.
     *        If the key does not exist, the size is considered 0.
     *        If the key represents a document, the size is considered 1.
     * @param key The key to check.
     * @return The size of the key.
     */
    size_t memberSize(std::string const& key);

    /**
     * @brief Removes a key from the JSON document.
     *        This function removes a key from the JSON document.
     *        If the key does not exist, no action is taken.
     *        Note that the document is flushed before removing the key.
     * @param key The key to remove.
     */
    void removeKey(char const* key);

    //------------------------------------------
    // Serialize/Deserialize

    /**
     * @brief Serializes the entire document or a portion of the document
     * @param key The key to serialize. (Optional: leave empty to serialize entire document)
     * @return The serialized JSON string.
     */
    std::string serialize(std::string const& key = "");

    /**
     * @brief Deserializes a JSON string or loads from a file, with optional modifications.
     * @param serial_or_link The JSON string to deserialize or the file path to load from + optional functioncall modifiers
     *        Examples:
     *        - `{"key": "value"}` - Deserializes a JSON string
     *        - `file.json` - Loads a JSON file
     *        - `file.json|set key1.key2[5] 100` - Loads a JSON file and sets a value
     *        - `file.json|key1.key2[5]=100` - Legacy feature for setting a value
     *        - `file.json|set-from-json key1.key2[5] otherFile.json:key` - Sets key1.key2[5] from the value of key in otherFile.json
     */
    void deserialize(std::string const& serial_or_link);

    //------------------------------------------
    // Assorted list of double pointers

    /**
     * @brief Retrieves the map of ordered double pointers for "other" expression references.
     * @return A pointer to the map of ordered double pointers for "other" reference.
     */
    MappedOrderedDoublePointers* getExpressionRefsAsOther();
};
} // namespace Nebulite::Utility
#include "Utility/JSON.tpp"
#endif // NEBULITE_UTILITY_JSON_HPP