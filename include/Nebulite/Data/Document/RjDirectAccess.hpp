#ifndef NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_HPP
#define NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// External
#include <rapidjson/document.h>
#include <rapidjson/reader.h>

//------------------------------------------
/**
 * @namespace RjDirectAccess
 * @brief Provides direct access and manipulation of RapidJSON values.
 */
namespace Nebulite::Data::RjDirectAccess {
    /**
     * @brief Used flags for rapidjson parsing, allowing comments and trailing commas in JSON.
     */
    static auto constexpr rapidjsonParseFlags = rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag;

    /**
     * @brief Definition of a simple value variant type.
     *        All of these types are supported for direct access.
     */
    using SimpleValue = std::variant<
        std::int32_t,
        std::int64_t,
        std::uint32_t,
        std::uint64_t,
        double,
        std::string,
        bool
    >;

    template <typename NewType>
    std::optional<NewType> convertSimpleValue(SimpleValue const& simpleValue);

    /**
     * @brief Getting a simple value from a rapidjson value, using the right type stored in the document.
     * @param val Pointer to the rapidjson value to get the value from.
     * @return An optional SimpleValue containing the value if successful, or std::nullopt if the type is unsupported.
     */
    std::optional<SimpleValue> getSimpleValue(rapidjson::Value const* val);

    /**
     * @brief Getting a simple value from a rapidjson value, using the right type stored in the document.
     * @tparam RjValType The type of the rapidjson value, should be a rapidjson::Document to ensure correct key traversal.
     * @param key The key to traverse in the document to find the value.
     * @param doc The rapidjson document to search within.
     * @return An optional SimpleValue containing the value if successful, or std::nullopt if the type is unsupported.
     */
    template<typename RjValType>
    std::optional<SimpleValue> getSimpleValue(std::string_view key, RjValType& doc);

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
    T get(char const* key, T const& defaultValue, rapidjson::Value& val);

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
    bool set(char const* key, T const& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

    //------------------------------------------
    // Conversion

    /**
     * @brief Converts a rapidjson value to a given type.
     * @tparam T The type to convert to.
     * @param jsonValue The rapidjson value to convert.
     * @param result The variable to store the result of.
     * @param defaultValue The default value to use if conversion fails.
     */
    template <typename T> requires (!std::is_trivially_copyable_v<T>)
    void convertFromJsonValue(rapidjson::Value const& jsonValue, T& result, T const& defaultValue = T());

    /**
     * @brief Converts a rapidjson value to a given type.
     * @tparam T The type to convert to.
     * @param jsonValue The rapidjson value to convert.
     * @param result The variable to store the result of.
     * @param defaultValue The default value to use if conversion fails.
     */
    template <typename T> requires std::is_trivially_copyable_v<T>
    void convertFromJsonValue(rapidjson::Value const& jsonValue, T& result, T defaultValue = T());

    /**
     * @brief Converts a given type to a rapidjson value.
     * @tparam T The type to convert from.
     * @param data The variable to convert.
     * @param jsonValue The rapidjson value to store the result.
     * @param allocator The allocator to use for creating new rapidjson values.
     */
    template <typename T> requires (!std::is_trivially_copyable_v<T>)
    void convertToJsonValue(T const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator);

    /**
     * @brief Converts a given type to a rapidjson value.
     * @tparam T The type to convert from.
     * @param data The variable to convert.
     * @param jsonValue The rapidjson value to store the result.
     * @param allocator The allocator to use for creating new rapidjson values.
     */
    template <typename T> requires std::is_trivially_copyable_v<T>
    void convertToJsonValue(T data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator);

    //------------------------------------------
    // Document traversal

    /**
     * @brief Traverses rapidjson value to find a value within identified by its key.
     * @param key The key to search for.
     * @param val The rapidjson value to search within.
     * @return A pointer to the found rapidjson value, or nullptr if not found.
     */
    rapidjson::Value* traversePath(std::string_view key, rapidjson::Value& val);

    /**
     * @brief Structure to hold the result of traversing a rapidjson value to find the parent of a value identified by its key.
     */
    struct traverseResult {
        rapidjson::Value* parent;
        std::string_view poppedMember;
        int poppedIndex;
    };

    /**
     * @brief Traverses a rapidjson value to find the parent of a value identified by its key.
     *        - `parent.child`           -> returns `parent`,       poppedMember = `child`,   poppedIndex = -1
     *        - `parent.child[index]`    -> returns `parent.child`, poppedMember = `child`,   poppedIndex = index
     *        - `parent[index]`          -> returns `parent`,       poppedMember = "",        poppedIndex = index
     * @param keyStr The child key
     * @param root The rapidjson value to search within.
     * @return A traverseResult containing the parent value and traversal information.
     */
    traverseResult traverseToParent(std::string_view keyStr, rapidjson::Value& root);

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
    rapidjson::Value* ensurePath(std::string_view key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

    //------------------------------------------
    // Serialization/Deserialization

    /**
     * @brief Enum representing the type of serialization for JSON output.
     *        - compact: No extra whitespace or indentation.
     *        - pretty: Indented and formatted for readability.
     */
    enum class SerializationType : bool {
        compact,
        pretty,
    };

    /**
     * @brief Serializes a rapidjson document to a string.
     * @param doc The rapidjson document to serialize.
     * @param type Type of serialization. Defaults to pretty printing.
     * @return The serialized JSON string.
     */
    std::string serialize(rapidjson::Document const& doc, SerializationType type = SerializationType::pretty);

    std::string serialize(rapidjson::Value const& val, SerializationType type = SerializationType::pretty);

    /**
     * @brief Deserializes a JSON string into a rapidjson document.
     * @param doc The rapidjson document to populate.
     * @param serialOrLink The JSON string to deserialize.
     */
    void deserialize(rapidjson::Document& doc, std::string_view serialOrLink);

    /**
     * @brief Deserializes a JSON string into a rapidjson document.
     * @details Only use if the given string is guaranteed to be a valid JSON string!
     */
    void deserializeFromJson(rapidjson::Document& doc, std::string_view json);

    //------------------------------------------
    // Helper functions

    /**
     * @brief Sorts a rapidjson value, including all its sub-values.
     * @param value The rapidjson value to sort.
     * @param allocator The allocator to use for creating new rapidjson values.
     * @return A new rapidjson value representing the sorted input.
     */
    rapidjson::Value sortRecursive(rapidjson::Value const& value, rapidjson::Document::AllocatorType& allocator);

    /**
     * @brief Strips comments from a JSONC string for a JSON-compatible output.
     * @param jsonc The JSONC string to process.
     * @return The JSON-compatible string.
     */
    std::string stripComments(std::string_view jsonc);

    /**
     * @brief Checks if a string is in JSON or JSONC format.
     * @param str The string to check.
     * @return true if the string is JSON or JSONC, false otherwise.
     */
    bool isJsonOrJsonc(std::string_view str);

    //------------------------------------------
    // Member management

    /**
     * @brief Empties a rapidjson document.
     * @param doc The rapidjson document to empty.
     */
    void empty(rapidjson::Document& doc);

    /**
     * @brief Removes a member from a rapidjson object by key.
     * @param key The key of the member to remove.
     * @param val The rapidjson object to remove the member from.
     */
    void removeMember(std::string_view key, rapidjson::Value& val);

    /**
     * @brief Validates if a key string is valid for traversal.
     * @param key The key string to validate.
     * @return true if the key is valid, false otherwise.
     */
    bool isValidKey(std::string_view key);

    /**
     * @brief Lists all available keys in a rapidjson object.
     * @param val The rapidjson object to list keys from.
     * @return A vector of strings containing all available keys.
     *         - For objects, returns member names.
     *         - For arrays, returns indices in bracket notation (e.g., "[0]", "[1]", ...).
     *         - For any other type, returns an empty vector.
     */
    std::vector<std::string> listAvailableMembers(rapidjson::Value const& val);

    // Special characters for key parsing
    struct SpecialCharacter {
        static auto constexpr arrayOpen = '[';
        static auto constexpr arrayClose = ']';
        static auto constexpr dot = '.';
    };
} // namespace Nebulite::Data::RjDirectAccess
#include "RjDirectAccess.tpp" // NOLINT(misc-include-cleaner)
#endif // NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_HPP
