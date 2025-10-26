/**
 * @file RjDirectAccess.hpp
 * 
 * Direct access to RapidJSON values
 */

#ifndef NEBULITE_UTILITY_RJDIRECTACCESS_HPP
#define NEBULITE_UTILITY_RJDIRECTACCESS_HPP

//------------------------------------------
// Includes

// General
#include <mutex>
#include <string>
#include <variant>

// External
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/encodings.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

// Nebulite
#include "Utility/StringHandler.hpp"
#include "Utility/Capture.hpp"

namespace Nebulite::Core {
    class GlobalSpace; // Forward declaration
}

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class RjDirectAccess
 * @brief Provides direct access and manipulation of RapidJSON values.
 */
class RjDirectAccess{
public:
    using simpleValue = std::variant<int32_t, int64_t, uint32_t, uint64_t, double, std::string, bool>;

    /**
     * @brief Getting a simple value from a rapidjson value, using the right type stored in the document.
     * 
     * @param value Pointer to the variant to store the value.
     * @param val Pointer to the rapidjson value to get the value from.
     * @return true if a supported type was found and value was set, false otherwise (e.g. Object, Array, Null)
     */
    static bool getSimpleValue(simpleValue* value, rapidjson::Value const* val){
        // Get supported types
        if(val->IsInt()){
            *value = val->GetInt();
        } else if(val->IsInt64()){
            *value = val->GetInt64();
        } else if(val->IsUint()){
            *value = val->GetUint();
        } else if(val->IsUint64()){
            *value = val->GetUint64();
        } else if(val->IsDouble()){
            *value = val->GetDouble();
        } else if(val->IsString()){
            *value = std::string(val->GetString(), val->GetStringLength());
        } else if(val->IsBool()){
            *value = val->GetBool();
        } else {
            // Unsupported type (e.g., Object, Array, Null)
            return false;
        }
        return true;
    }

    //------------------------------------------
    // Templated Getter, Setter

    /**
     * @brief Fallback to direct rapidjson access for getting values.
     * 
     * @param key The key of the value to retrieve.
     * @param defaultValue The default value to return if the key is not found.
     * @param val The rapidjson value to search within.
     * @return The retrieved value or the default value.
     */
    template <typename T> 
    static T get(char const* key, T const defaultValue, rapidjson::Value& val);

    /**
     * @brief Fallback to direct rapidjson access for setting values.
     * 
     * This function sets a value in the rapidjson document, ensuring that the key exists.
     * If the key does not exist, it is created.
     * 
     * @param key The key of the value to set.
     * @param value The value to set.
     * @param val The rapidjson value to modify.
     */
    template <typename T> 
    static bool set(char const* key, T const& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

    //------------------------------------------
    // Conversion

    /**
     * @brief Converts a rapidjson value to a C++ type.
     * 
     * @tparam T The C++ type to convert to.
     * @param jsonValue The rapidjson value to convert.
     * @param result The C++ variable to store the result.
     * @param defaultvalue The default value to use if conversion fails.
     */
    template <typename T>
    static void ConvertFromJSONValue(rapidjson::Value const& jsonValue, T& result, T const& defaultvalue = T());

    /**
     * @brief Converts a C++ type to a rapidjson value.
     * 
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
     * 
     * @param key The key to search for.
     * @param val The rapidjson value to search within.
     * @return A pointer to the found rapidjson value, or nullptr if not found.
     */
    static rapidjson::Value* traverse_path(char const* key, rapidjson::Value const& val);

    /**
     * @brief Traverses a rapidjson value to find or create a value within identified by its key.
     * 
     * @param key The key to search for.
     * @param val The rapidjson value to search within.
     * @param allocator The allocator to use for creating new values.
     * @return A pointer to the found or created rapidjson value.
     * Note that the returned value may be nullptr if the given key is invalid 
     * (e.g., trying to index into a non-array or using a malformed index).
     */
    static rapidjson::Value* ensure_path(char const* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

    /**
     * @brief Traverses a rapidjson value to find the parent of a value identified by its key.
     * 
     * - parent.child           -> returns parent,       finalKey = child, arrayIndex = -1
     * - parent.child[index]    -> returns parent.child, finalKey = child, arrayIndex = index
     * - parent[index]          -> returns parent,       finalKey = "",    arrayIndex = index
     * 
     * @param key The key to search for.
     * @param root The rapidjson value to search within.
     * @param finalKey The final key or index of the value to find the parent of.
     * @param arrayIndex The index if the final key is an array index, -1 otherwise.
     * @return A pointer to the parent rapidjson value, or nullptr if not found
     */
    static rapidjson::Value* traverse_to_parent(char const* fullKey, rapidjson::Value const& root, std::string& finalKey, int& arrayIndex);

    //------------------------------------------
    // Serialization/Deserialization

    /**
     * @brief Serializes a rapidjson document to a string.
     * 
     * @param doc The rapidjson document to serialize.
     * @return The serialized JSON string.
     */
    static std::string serialize(const rapidjson::Document& doc);

    /**
     * @brief Deserializes a JSON string into a rapidjson document.
     * 
     * @param doc The rapidjson document to populate.
     * @param serialOrLink The JSON string to deserialize.
     * @param global Pointer to the GlobalSpace, used for resolving links.
     */
    static void deserialize(rapidjson::Document& doc, std::string const& serialOrLink, Nebulite::Core::GlobalSpace* global);
    

    //------------------------------------------
    // Helper functions

    /**
     * @brief Sorts a rapidjson value, including all its sub-values.
     * 
     * @param value The rapidjson value to sort.
     * @param allocator The allocator to use for creating new rapidjson values.
     * @return A new rapidjson value representing the sorted input.
     */
    static rapidjson::Value sortRecursive(rapidjson::Value const& value, rapidjson::Document::AllocatorType& allocator);

    /**
     * @brief Strips comments from a JSONC string for a JSON-compatible output.
     * 
     * @param jsonc The JSONC string to process.
     * @return The JSON-compatible string.
     */
    static std::string stripComments(std::string const& jsonc);

    /**
     * @brief Empties a rapidjson document.
     * 
     * @param doc The rapidjson document to empty.
     */
    static void empty(rapidjson::Document &doc);

    /**
     * @brief Removes a member from a rapidjson object by key.
     */
    static void remove_member(char const* key, rapidjson::Value& val);

    /**
     * @brief Checks if a string is in JSON or JSONC format.
     * 
     * @param str The string to check.
     * @return true if the string is JSON or JSONC, false otherwise.
     */
    static bool is_json_or_jsonc(std::string const& str);

    /**
     * @brief Validates if a key string is valid for traversal.
     * 
     * @param key The key string to validate.
     * @return true if the key is valid, false otherwise.
     */
    static bool isValidKey(std::string const& key);

private:
    /**
     * @brief Extracts the next part of a key from a dot/bracket notation key string.
     * 
     * Moves keyView forward past the extracted part.
     */
    static std::string_view extractKeyPart(std::string_view* keyView);
};
} // namespace Nebulite::Utility

//------------------------------------------
// Direct access get/set

template <typename T>
T Nebulite::Utility::RjDirectAccess::get(char const* key, T const defaultValue, rapidjson::Value& val){
    rapidjson::Value* keyVal = Nebulite::Utility::RjDirectAccess::traverse_path(key,val);
    if(keyVal == nullptr){
        // Value doesnt exist in doc, return default
        return defaultValue;
    }
    else{
        // Base case: convert currentVal to T using JSONHandler
        T tmp;
        Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue<T>(*keyVal, tmp, defaultValue);
        return tmp;
    }
}

template <typename T>
bool Nebulite::Utility::RjDirectAccess::set(char const* key, T const& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator){
    // Ensure key path exists
    rapidjson::Value* keyVal = Nebulite::Utility::RjDirectAccess::ensure_path(key, val, allocator);
    if (keyVal != nullptr){
        Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<T>(value, *keyVal, allocator);
        return true;
    } else {
        return false;
    }
}

//------------------------------------------
// All conversion variants from/to rapidjson values

//------------------------------------------
// 1.) to JSON value
//------------------------------------------

// cppcheck-suppress constParameterReference
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<bool>(bool const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)              {
    jsonValue.SetBool(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<int>(int const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)                {
    jsonValue.SetInt(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<uint32_t>(uint32_t const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)      {
    jsonValue.SetUint(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<uint64_t>(uint64_t const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)      {
    jsonValue.SetUint64(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<double>(double const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)          {
    jsonValue.SetDouble(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<long>(long const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)              {
    jsonValue.SetInt64(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<long long>(long long const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)    {
    jsonValue.SetInt64(data);
    (void)allocator; // Suppress unused parameter warning
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<std::string>(std::string const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    jsonValue.SetString(
        data.c_str(), 
        static_cast<rapidjson::SizeType>(data.length()), allocator
    );
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<char const*>(char const* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    if (data){
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<char*>(char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    if (data){
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<rapidjson::Value*>(rapidjson::Value* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)         {jsonValue.CopyFrom(*data, allocator);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<rapidjson::Document*>(rapidjson::Document* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)   {jsonValue.CopyFrom(*data, allocator);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<rapidjson::Document>(const rapidjson::Document& data,rapidjson::Value& jsonValue,rapidjson::Document::AllocatorType& allocator)       {jsonValue.CopyFrom(data, allocator);}

// Template specialization for std::variant
// So we don't have to manually call std::visit every time
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue(const simpleValue& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    std::visit([&](auto const& value){
        using T = std::decay_t<decltype(value)>;
        ConvertToJSONValue<T>(value, jsonValue, allocator);
    }, data);
}

//------------------------------------------
// 2.) from JSON Value
//------------------------------------------

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, bool& result,  bool const& defaultvalue){
    if (jsonValue.IsBool()){
        result = jsonValue.GetBool();
    } else {
        result = defaultvalue;
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, int& result,  int const& defaultvalue){
    if (jsonValue.IsInt()){
        result = jsonValue.GetInt();
    }
    else if(jsonValue.IsBool()){
        result = jsonValue.GetBool();
    }
    else{
        result = defaultvalue;
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, uint32_t& result,  uint32_t const& defaultvalue){
    if(jsonValue.IsUint()){
        result = jsonValue.GetUint();
        return;
    }
    else if(jsonValue.IsNumber()){
        int tmp = jsonValue.GetInt();
        if(tmp >= 0){
            result = static_cast<uint32_t>(tmp);
            return;
        }
    }
    else if(jsonValue.IsString()){
        std::istringstream iss(jsonValue.GetString());
        iss >> result;
        return;
    }
    else{
        result = defaultvalue;
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, uint64_t& result,  uint64_t const& defaultvalue){
    if (jsonValue.IsString()){
        std::istringstream iss(jsonValue.GetString());
        iss >> result;
    } else if (jsonValue.IsUint64()){
        result = jsonValue.GetUint64();
    } else if (jsonValue.IsUint()){
        result = static_cast<uint64_t>(jsonValue.GetUint());
    } else if (jsonValue.IsNumber()){
        int64_t tmp = jsonValue.GetInt64();
        if (tmp >= 0){
            result = static_cast<uint64_t>(tmp);
        } else {
            result = defaultvalue;
        }
    } else {
        result = defaultvalue;
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, double& result, double const& defaultvalue){
    if (jsonValue.IsNumber()){
        result = jsonValue.GetDouble();
    }
    else if(jsonValue.IsString()){
        std::string strValue = jsonValue.GetString();
        if(Nebulite::Utility::StringHandler::isNumber(strValue)){
            result = std::stod(strValue);
        }
        else{
            result = defaultvalue;
        }
    }
    else {
        result = defaultvalue;
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, std::string& result, std::string const& defaultvalue){
    if (jsonValue.IsBool()){
        result = jsonValue.GetBool() ? "true" : "false";
    }
    else if (jsonValue.IsString()){
        result = std::string(jsonValue.GetString());
    }
    else if (jsonValue.IsInt()){
        result = std::to_string(jsonValue.GetInt());
    }
    else if (jsonValue.IsUint()){
        result = std::to_string(jsonValue.GetUint());
    }
    else if (jsonValue.IsInt64()){
        result = std::to_string(jsonValue.GetInt64());
    }
    else if (jsonValue.IsUint64()){
        result = std::to_string(jsonValue.GetUint64());
    }
    else if (jsonValue.IsDouble()){
        result = std::to_string(jsonValue.GetDouble());
    }
    else if (jsonValue.IsNull()){
        result = "null";
    }
    else if (jsonValue.IsArray()){
        result = "{Array}";
    }
    else if (jsonValue.IsObject()){
        result = "{Object}";  // Just a placeholder since objects can't easily be converted to a single string
    }
    else {
        result = defaultvalue;
    }
}
#endif // NEBULITE_UTILITY_RJDIRECTACCESS_HPP