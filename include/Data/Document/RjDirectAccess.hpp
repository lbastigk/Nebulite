/**
 * @file RjDirectAccess.hpp
 * 
 * Direct access to RapidJSON values
 */

#ifndef NEBULITE_DATA_RJDIRECTACCESS_HPP
#define NEBULITE_DATA_RJDIRECTACCESS_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <variant>

// External
#include <rapidjson/document.h>

//------------------------------------------
namespace Nebulite::Data {
/**
 * @class RjDirectAccess
 * @brief Provides direct access and manipulation of RapidJSON values.
 */
class RjDirectAccess {
public:
    /**
     * @brief Definition of a simple value variant type.
     *        All of these types are supported for direct access.
     */
    using simpleValue = std::variant<
        int32_t,
        int64_t,
        uint32_t,
        uint64_t,
        double,
        std::string,
        bool
    >;

    /**
     * @brief Getting a simple value from a rapidjson value, using the right type stored in the document.
     * @param val Pointer to the rapidjson value to get the value from.
     * @return An optional simpleValue containing the value if successful, or std::nullopt if the type is unsupported.
     */
    static std::optional<simpleValue> getSimpleValue(rapidjson::Value const* val);

    //------------------------------------------
    // Templated Getter, Setter

    /**
     * @brief Fallback to direct rapidjson access for getting values.
     * @param key The key of the value to retrieve.
     * @param defaultValue The default value to return if the key is not found.
     * @param val The rapidjson value to search within.
     * @return The retrieved value or the default value.
     */
    template <typename T>
    static T get(char const* key, T const& defaultValue, rapidjson::Value& val);

    /**
     * @brief Fallback to direct rapidjson access for setting values.
     *        This function sets a value in the rapidjson document, ensuring that the key exists.
     *        If the key does not exist, it is created.
     * @param key The key of the value to set.
     * @param value The value to set.
     * @param val The rapidjson value to modify.
     * @param allocator The allocator to use for creating new rapidjson values.
     * @return true if the value was set successfully, false otherwise.
     */
    template <typename T>
    static bool set(char const* key, T const& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

    //------------------------------------------
    // Conversion

    /**
     * @brief Converts a rapidjson value to a C++ type.
     * @tparam T The C++ type to convert to.
     * @param jsonValue The rapidjson value to convert.
     * @param result The C++ variable to store the result.
     * @param defaultValue The default value to use if conversion fails.
     */
    template <typename T>
    static void ConvertFromJSONValue(rapidjson::Value const& jsonValue, T& result, T const& defaultValue = T());

    /**
     * @brief Converts a C++ type to a rapidjson value.
     * @tparam T The C++ type to convert from.
     * @param data The C++ variable to convert.
     * @param jsonValue The rapidjson value to store the result.
     * @param allocator The allocator to use for creating new rapidjson values.
     */
    template <typename T>
    static void ConvertToJSONValue(T const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator);

    //------------------------------------------
    // Document traversal

    /**
     * @brief Traverses rapidjson value to find a value within identified by its key.
     * @param key The key to search for.
     * @param val The rapidjson value to search within.
     * @return A pointer to the found rapidjson value, or nullptr if not found.
     */
    static rapidjson::Value* traversePath(char const* key, rapidjson::Value const& val);

    /**
     * @brief Traverses a rapidjson value to find or create a value within identified by its key.
     *
     * @param key The key to search for.
     * @param val The rapidjson value to search within.
     * @param allocator The allocator to use for creating new values.
     * @return A pointer to the found or created rapidjson value.
     *         Note that the returned value may be nullptr if the given key is invalid
     *         (e.g., trying to index into a non-array or using a malformed index).
     */
    static rapidjson::Value* ensurePath(char const* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

    /**
     * @brief Traverses a rapidjson value to find the parent of a value identified by its key.
     *        - `parent.child`           -> returns `parent`,       finalKey = `child`,   arrayIndex = -1
     *        - `parent.child[index]`    -> returns `parent.child`, finalKey = `child`,   arrayIndex = index
     *        - `parent[index]`          -> returns `parent`,       finalKey = "",        arrayIndex = index
     * @param fullKey The key to search for.
     * @param root The rapidjson value to search within.
     * @param finalKey The final key or index of the value to find the parent of.
     * @param arrayIndex The index if the final key is an array index, -1 otherwise.
     * @return A pointer to the parent rapidjson value, or nullptr if not found
     */
    static rapidjson::Value* traverseToParent(char const* fullKey, rapidjson::Value const& root, std::string& finalKey, int& arrayIndex);

    //------------------------------------------
    // Serialization/Deserialization

    /**
     * @brief Serializes a rapidjson document to a string.
     * @param doc The rapidjson document to serialize.
     * @return The serialized JSON string.
     */
    static std::string serialize(rapidjson::Document const& doc);

    /**
     * @brief Deserializes a JSON string into a rapidjson document.
     * @param doc The rapidjson document to populate.
     * @param serialOrLink The JSON string to deserialize.
     */
    static void deserialize(rapidjson::Document& doc, std::string const& serialOrLink);


    //------------------------------------------
    // Helper functions

    /**
     * @brief Sorts a rapidjson value, including all its sub-values.
     * @param value The rapidjson value to sort.
     * @param allocator The allocator to use for creating new rapidjson values.
     * @return A new rapidjson value representing the sorted input.
     */
    static rapidjson::Value sortRecursive(rapidjson::Value const& value, rapidjson::Document::AllocatorType& allocator);

    /**
     * @brief Strips comments from a JSONC string for a JSON-compatible output.
     * @param jsonc The JSONC string to process.
     * @return The JSON-compatible string.
     */
    static std::string stripComments(std::string const& jsonc);

    /**
     * @brief Empties a rapidjson document.
     * @param doc The rapidjson document to empty.
     */
    static void empty(rapidjson::Document& doc);

    /**
     * @brief Removes a member from a rapidjson object by key.
     * @param key The key of the member to remove.
     * @param val The rapidjson object to remove the member from.
     */
    static void removeMember(char const* key, rapidjson::Value& val);

    /**
     * @brief Checks if a string is in JSON or JSONC format.
     * @param str The string to check.
     * @return true if the string is JSON or JSONC, false otherwise.
     */
    static bool isJsonOrJsonc(std::string const& str);

    /**
     * @brief Validates if a key string is valid for traversal.
     * @param key The key string to validate.
     * @return true if the key is valid, false otherwise.
     */
    static bool isValidKey(std::string const& key);

private:
    /**
     * @brief Extracts the next part of a key from a dot/bracket notation key string.
     *        Moves keyView forward past the extracted part.
     * @param keyView View to extract from and modify.
     * @return The extracted key part as a std::string.
     */
    static std::string extractKeyPart(std::string_view& keyView);
};
} // namespace Nebulite::Data
#include "RjDirectAccess.tpp"
#endif // NEBULITE_DATA_RJDIRECTACCESS_HPP
