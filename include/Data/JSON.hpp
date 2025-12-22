/**
 * @file JSON.hpp
 * @brief A wrapper around rapidjson to simplify JSON manipulation in Nebulite.
 */

#ifndef NEBULITE_DATA_JSON_HPP
#define NEBULITE_DATA_JSON_HPP

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
#include "Data/JsonRvalueTransformer.hpp"
#include "Data/OrderedDoublePointers.hpp"
#include "Data/RjDirectAccess.hpp"

//------------------------------------------
namespace Nebulite::Data {
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
 *        - usage of return value transformations on get (length, type checks, arithmetic operations, etc.)
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

        CacheEntry() = default;

        ~CacheEntry() {delete stable_double_ptr;}

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

    //------------------------------------------
    // Ordered double pointers system

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
    std::array<double*, uidQuickCacheSize> uidDoubleCache{nullptr};

    //------------------------------------------
    // Return Value Transformation system

    /**
     * @brief Instance for applying transformations on get operations.
     */
    JsonRvalueTransformer transformer;

    /**
     * @brief Apply transformations found in the key string and retrieve the modified value.
     * @tparam T The type of the value to retrieve.
     * @param key The key string containing transformations.
     * @return The modified value of type T, or none on failure.
     */
    template <typename T>
    std::optional<T> getWithTransformations(std::string const& key);

    /**
     * @brief Apply transformations found in the key string and retrieve the modified document.
     * @param key The key string containing transformations.
     * @param outDoc The output JSON document to store the modified result.
     * @return True on success, false on failure.
     * @note We use an external outDoc to avoid copying the entire document on return/on optional::getValue().
     */
    bool getSubDocWithTransformations(std::string const& key, JSON& outDoc);

public:
    //------------------------------------------
    // Constructor/Destructor

    /**
     * @brief Constructs a new JSON document.
     * @param name The name of the JSON document. Recommended for debugging purposes.
     */
    explicit JSON(std::string const& name = "Unnamed JSON Document");

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
     *        - '|'  : Used for piping transformations.
     *        - '"'  : Used for string encapsulation.
     *        - ':'  : Used for Read-Only docs to separate link and key.
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
    static bool isJsonOrJsonc(std::string const& str) {
        return RjDirectAccess::isJsonOrJsonc(str);
    }

    //------------------------------------------
    // Set methods

    /**
     * @brief Sets a value of the specified type in the JSON document.
     *        If the key already exists, the value and its stable double pointer
     *        is updated. Child keys are invalidated.
     * @tparam T The type of the value to set.
     * @param key The key of the value to set.
     * @param val The value to set.
     */
    template <typename T>
    void set(std::string const& key, T const& val);

    /**
     * @brief Sets a variant value of supported simple values in the JSON document.
     *        Stable double pointer of the value is updated as well. Child keys are invalidated.
     * @param key The key of the value to set.
     * @param val The variant value to set.
     */
    void setVariant(std::string const& key, RjDirectAccess::simpleValue const& val);

    /**
     * @brief Sets a sub-document in the JSON document.
     *        If the key already exists, the sub-document is updated.
     *        Note that both the child and parent documents' caches are flushed before setting.
     * @param key The key of the sub-document to set.
     * @param child The sub-document to set.
     */
    void setSubDoc(char const* key, JSON& child);

    /**
     * @brief Sets an empty array in the JSON document.
     *        This function sets an empty array in the JSON document.
     *        If the key already exists, the array is updated.
     *        Note that the document is flushed before setting.
     * @param key The key of the array to set.
     */
    void setEmptyArray(char const* key);

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
     * @brief Gets a variant value from the JSON document.
     *        This function retrieves a variant value from the JSON document.
     *        If the key does not exist, void is returned.
     * @param key The key of the value to retrieve.
     * @return The variant value associated with the key, or void if the key does not exist.
     */
    std::optional<RjDirectAccess::simpleValue> getVariant(std::string const& key);

    /**
     * @brief Gets a sub-document from the JSON document.
     *        If the key does not exist, an empty JSON object is returned.
     *        Note that the cache is flushed into the document.
     *        If the key is a basic type, its value is returned.
     *        You may use `memberType("")` to check the type stored in the JSON.
     *        You may use `get<T>("",T())` on the returned sub-document to get the simple value.
     * @param key The key of the sub-document to retrieve.
     * @return The sub-document associated with the key, or an empty JSON object if the key does not exist.
     */
    JSON getSubDoc(std::string const& key);

    /**
     * @brief Gets a pointer to a double value pointer in the JSON document.
     * @return A pointer to the double value associated with the key.
     */
    double* getStableDoublePointer(std::string const& key);

    /**
     * @brief Provides access to the internal mutex for thread-safe operations.
     *        Allowing modules to lock the JSON document.
     */
    std::scoped_lock<std::recursive_mutex> lock() { return std::scoped_lock(mtx); }

    //------------------------------------------
    // Getters: Unique id based retrieval

    /**
     * @brief Retrieves the map of ordered double pointers for expression references.
     * @return A pointer to the map of ordered double pointers for reference.
     * @note Use a proper unique Identifier!
     *       - in Expressions, the map is only used for "other" references and uses the expression as hash
     *       - in Static Rulesets, the map is used for both "self" and "other" references, and uses the function name as hash
     *
     *       Later on, with more context such as parent, we need to find different hashes for each context.
     *       perhaps: <context>::<function>
     *       and for static rulesets, just using ::<function> is enough.
     *       But we can use multiple retrievals if we desire, specifying different contexts.
     * @todo Implement uid generation for the map here instead of in the globalspace
     */
    MappedOrderedDoublePointers* getOrderedCacheListMap();

    //------------------------------------------
    // Key Types, Sizes

    /**
     * @enum KeyType
     * @brief Enum representing the type stored of a key in the JSON document.
     */
    enum class KeyType : uint8_t {
        null,
        value,
        array,
        object
    };

    /**
     * @brief Checks the type stored of a key in the JSON document.
     *        This function checks the type stored of a key in the JSON document.
     *        If the key does not exist, the type is considered null.
     * @param key The key to check.
     * @return The type of the key.
     */
    KeyType memberType(std::string const& key);

    /**
     * @brief Checks the size of a key in the JSON document.
     *        If the key does not exist, the size is considered 0.
     *        If the key represents a document, the size is considered 1.
     * @param key The key to check.
     * @return The size of the key.
     */
    size_t memberSize(std::string const& key);

    /**
     * @brief Removes a key from the JSON document.
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