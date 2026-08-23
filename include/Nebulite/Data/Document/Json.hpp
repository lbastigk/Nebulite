#ifndef NEBULITE_DATA_DOCUMENT_JSON_HPP
#define NEBULITE_DATA_DOCUMENT_JSON_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <cstdint> // NOLINT
#include <expected>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

// External
#include <absl/container/flat_hash_map.h>
#include <rapidjson/document.h>

// Nebulite
#include "Nebulite/Data/Document/JsonCache.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Data {
/**
 * @class Json
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
class Json {
    mutable JsonCache cache;

    /**
     * @brief A helper variable that is modified to signal certain functions as non-const.
     */
    std::uint64_t helperNonConstVar = 0;

    /**
     * @brief The underlying RapidJSON document.
     * @details Is mutable, as we regularly need to flush contents into it from const get-calls.
     */
    mutable rapidjson::Document doc;

    // Mutex for thread safety
    mutable std::recursive_mutex mtx;

    /**
     * @brief Synchronizes all children of a given key.
     * @details For example, if parent_key is "config", it will sync
     *          "config.option1", "config.option2.suboption", etc.
     *          as well as "config[0]", "config[1].suboption", etc.
     *          with the rapidjson values.
     */
    void synchronizeChildren(std::string_view parentKey) const ;

    /**
     * @brief Flush all DIRTY entries in the cache back to the RapidJSON document.
     * @details This ensures that the RapidJSON document is always structurally valid
     *          and up-to-date with the cached values.
     * @param key The key to flush. Finds the parent key and flushes all entries beginning with the parent key.
     * @throws std::runtime_error if setting a value in the RapidJSON document fails due to structural issues.
     */
    void flush(std::string_view key) const ;

    //------------------------------------------
    // Return Value Transformation system

    /**
     * @brief Apply transformations found in the key string and retrieve the modified value.
     * @tparam T The type of the value to retrieve.
     * @param key The key string containing transformations.
     * @return The modified value of type T, or none on failure.
     */
    template <typename T>
    std::expected<T, SimpleValueRetrievalError> getWithTransformations(std::string_view key) const ;

    /**
     * @brief Apply transformations found in the key string and retrieve the modified document.
     * @param key The key string containing transformations.
     * @param outDoc The output JSON document to store the modified result.
     * @return True on success, false on failure.
     * @note We use an external outDoc to avoid copying the entire document on return/on optional::getValue().
     */
    bool getSubDocWithTransformations(std::string_view key, Json& outDoc) const ;

    static std::vector<std::string_view> splitKeyWithTransformations(std::string_view key);

    //------------------------------------------
    // Scope sharing system

    absl::flat_hash_map<std::string, std::unique_ptr<JsonScope>> managedScopes;
    std::unique_ptr<JsonScope> fullScopeInstance;
    std::unique_ptr<JsonScope> dummyScopeInstance;

public:
    //------------------------------------------
    // Constructor/Destructor

    /**
     * @brief Constructs a new JSON document.
     */
    Json();

    ~Json();

    //------------------------------------------
    // Move/Copy

    // No copy
    Json(Json const&) = delete;
    Json& operator=(Json const&) = delete;

    // Allow move
    Json(Json&& other) noexcept;
    Json& operator=(Json&& other) noexcept;

    //------------------------------------------
    // Basic public constants

    /**
     * @brief A list of reserved characters that cannot be used in key names.
     * @details - '[]' : Used for array indexing.
     *          - '{}' : Could cause issues with object traversal and transformations, and just in general a bad idea for keys
     *          - '()' : Just in general a bad idea for a key
     *          - '.'  : Used for nested object traversal.
     *          - '|'  : Used for piping transformations.
     *          - '"'  : Used for string encapsulation.
     *          - '''  : Used for string encapsulation
     *          - ':'  : Used for Read-Only docs to separate link and key.
     */
    static auto constexpr reservedCharacters = "[]{}().|\"':";

    /**
     * @brief Often used special characters for value retrieval
     */
    struct SpecialCharacter {
        static auto constexpr arrayOpen = RjDirectAccess::SpecialCharacter::arrayOpen;
        static auto constexpr arrayClose = RjDirectAccess::SpecialCharacter::arrayClose;
        static auto constexpr dot = RjDirectAccess::SpecialCharacter::dot;
        static auto constexpr transformationPipe = '|';
        static auto constexpr linkKeySeparator = ':';
    };

    //------------------------------------------
    // Scope sharing

    /**
     * @brief Lazy-initialized full JsonScope representing the entire document.
     * @return Reference to the full JsonScope.
     */
    JsonScope& fullScope();

    /**
     * @brief Shares part JSON document as a JsonScope that is managed internally.
     * @details Sometimes we cannot use full JsonScopes due circular issues, causing a cascade of Domain initializations.
     *          In those cases, use JsonScope via this method.
     * @param prefix The prefix representing the part of the JSON document to share.
     *               If empty, shares the entire document.
     * @return A JsonScope reference representing a part of the JSON document.
     */
    JsonScope& shareManagedScope(std::string_view prefix);

    JsonScope& getDummyScope();

    //------------------------------------------
    // Custom copy method

    /**
     * @brief Copies the entire content from another JSON document into this one.
     * @param other The other JSON document to copy from.
     */
    void copyFrom(Json const& other);

    //------------------------------------------
    // Static helpers

    /**
     * @brief Checks if a string is in JSON or JSONC format.
     * @param str The string to check.
     * @return true if the string is JSON or JSONC, false otherwise.
     */
    static bool isJsonOrJsonc(std::string_view str);

    /**
     * @brief Finds the parent key of a given key in the JSON document.
     * @param key The key to find the parent of.
     * @return The parent key as a string_view. If the key is at the root level, returns an empty string_view.
     */
    static std::string_view findParentKey(std::string_view key);

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
    template <typename T> std::expected<T, SimpleValueRetrievalError> get(std::string_view key) const ;

    /**
     * @brief Gets a variant value from the JSON document.
     * @details This function retrieves a variant value from the JSON document.
     *          If the key does not exist, void is returned.
     * @param key The key of the value to retrieve.
     * @return The variant value associated with the key, or an error if the retrieval failed.
     */
    std::expected<RjDirectAccess::SimpleValue, SimpleValueRetrievalError> getVariant(std::string_view key) const ;

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
    Json getSubDoc(std::string_view key) const ;

    /**
     * @brief Gets a pointer to a double value pointer in the JSON document.
     * @return A pointer to the double value associated with the key.
     */
    double* getStableDoublePointer(std::string_view key) const ;

    /**
     * @brief Provides access to the internal mutex for thread-safe operations.
     */
    std::unique_lock<std::recursive_mutex> lock() const ;

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
    template <typename T> void set(std::string_view key, T const& val);

    /**
     * @brief Sets a variant value of supported simple values in the JSON document.
     *        Stable double pointer of the value is updated as well. Child keys are invalidated.
     * @param key The key of the value to set.
     * @param val The variant value to set.
     */
    void setVariant(std::string_view key, RjDirectAccess::SimpleValue const& val);

    /**
     * @brief Sets a sub-document in the JSON document.
     * @details If the key already exists, the sub-document is updated.
     *          Note that both the child and parent documents' caches are flushed before setting.
     * @param key The key of the sub-document to set.
     * @param child The sub-document to set.
     * @param childKey The key in the child document to set as the root of the sub-document. If empty, the entire child document is used.
     */
    void setSubDoc(std::string_view key, Json const& child, std::string_view childKey = "");

    /**
     * @brief Sets an empty array in the JSON document.
     * @details This function sets an empty array in the JSON document.
     *          If the key already exists, the array is updated.
     *          Note that the document is flushed before setting.
     * @param key The key of the array to set.
     */
    void setEmptyArray(std::string_view key);

    /**
     * @brief Sets an empty object in the JSON document.
     * @details This function sets an empty object in the JSON document.
     *          If the key already exists, the object is updated.
     *          Note that the document is flushed before setting.
     * @param key The key of the object to set.
     */
    void setEmptyObject(std::string_view key);

    //------------------------------------------
    // Special sets for threadsafe maths operations

    /**
     * @brief Performs an addition operation on a numeric value in the JSON document.
     */
    void setAdditive(std::string_view key, double val);

    void setAdditive(std::string_view key, std::int64_t val);

    /**
     * @brief Performs a multiplication operation on a numeric value in the JSON document.
     */
    void setMultiplicative(std::string_view key, double val);

    void setMultiplicative(std::string_view key, std::int64_t val);

    /**
     * @brief Performs a concatenation operation on a string value in the JSON document.
     */
    void setConcatenative(std::string_view key, std::string_view valStr);

    //------------------------------------------
    // Key Types, Sizes

    /**
     * @brief Checks the type stored of a key in the JSON document.
     * @details This function checks the type stored of a key in the JSON document.
     *          If the key does not exist, the type is considered null.
     * @param key The key to check.
     * @return The type of the key.
     */
    KeyType memberType(std::string_view key) const ;

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
    std::string memberTypeString(std::string_view key) const ;

    /**
     * @brief Checks the size of a key in the JSON document.
     * @details If the key does not exist, the size is considered 0.
     *          If the key represents a document, the size is considered 1.
     * @param key The key to check.
     * @return The size of the key.
     */
    std::size_t memberSize(std::string_view key) const ;

    /**
     * @brief Removes a key from the JSON document.
     * @details If the key does not exist, no action is taken.
     *          Note that the document is flushed before removing the key.
     *          If an array element is removed, the remaining elements are shifted to fill the gap,
     *          and their keys are updated accordingly. This is important for loops that rely on array indices!
     *          If you must remove multiple array elements, consider starting at the end of the array and moving backwards to avoid index shifting issues.
     * @param key The key to remove.
     */
    void removeMember(std::string_view key);

    /**
     * @brief Moves a member from one key to another in the JSON document.
     * @details Note that the current implementation is more of a copy+delete! Using just copyMember is faster if you don't need the original deleted.
     * @param fromKey The key of the member to move.
     * @param toKey The key to move the member to.
     * @note If the value in fromKey does not exist, toKey will be set to null.
     */
    void moveMember(std::string_view fromKey, std::string_view toKey);

    /**
     * @brief Copies a member from one key to another in the JSON document, without deleting the original.
     * @param fromKey The key of the member to copy.
     * @param toKey The key to copy the member to.
     */
    void copyMember(std::string_view fromKey, std::string_view toKey);


    /**
     * @brief Lists all available member keys in a rapidjson object.
     * @param key The key to list member keys from. (Optional: leave empty to list from root)
     * @return A vector of strings containing all available keys.
     *         - For objects, returns member names.
     *         - For arrays, returns indices in bracket notation (e.g., "[0]", "[1]", ...).
     *         - For any other type, returns an empty vector.
     */
    std::vector<std::string> listAvailableMembers(std::string_view key = "") const ;

    //------------------------------------------
    // Serialize/Deserialize

    /**
     * @brief Serializes the entire document or a portion of the document
     * @param key The key to serialize. (Optional: leave empty to serialize entire document)
     * @param type Type of serialization. Defaults to pretty printing.
     * @return The serialized JSON string.
     */
    std::string serialize(std::string_view key = "", RjDirectAccess::SerializationType type = RjDirectAccess::SerializationType::pretty) const ;

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
    void deserialize(std::string_view serialOrLink);

    //------------------------------------------
    // JSON - Rapidjson

    /**
     * @brief Gets a simple value form the rapidjson documents and synchronizes the cache with its value.
     * @param key The key of the value to retrieve
     * @return The value, or a SimpleValueRetrievalError if retrieval failed (e.g. key doesn't exist, type not supported, etc.)
     */
    std::expected<RjDirectAccess::SimpleValue, SimpleValueRetrievalError> getSimpleValueFromDocument(std::string_view key) const ;
};
} // namespace Nebulite::Data
#include "Nebulite/Data/Document/Json.tpp" // NOLINT(misc-include-cleaner)
#endif // NEBULITE_DATA_DOCUMENT_JSON_HPP
