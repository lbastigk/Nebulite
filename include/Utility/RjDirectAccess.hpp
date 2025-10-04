/**
 * @file RjDirectAccess.hpp
 * 
 * Direct access to RapidJSON values
 */

#pragma once

//------------------------------------------
// Includes

// TODO: Remove unused includes

// General
#include <mutex>
#include <typeinfo>
#include <cxxabi.h>
#include <string>
#include <variant>
#include <type_traits>
#include <typeindex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <memory>
#include <vector>
#include <algorithm>

// External
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"
#include "encodings.h"
#include "istreamwrapper.h"
#include "ostreamwrapper.h"

// Nebulite
#include "Utility/StringHandler.hpp"



namespace Nebulite::Utility {
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
    static bool getSimpleValue(simpleValue* value, rapidjson::Value* val) {
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
    // Getter, Setter

    /**
     * @brief Fallback to direct rapidjson access for getting values.
     * 
     * @param key The key of the value to retrieve.
     * @param defaultValue The default value to return if the key is not found.
     * @param val The rapidjson value to search within.
     * @return The retrieved value or the default value.
     */
    template <typename T> static T get(const char* key, const T defaultValue, rapidjson::Value& val);

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
    template <typename T> static void set(const char* key, const T& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

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
    static void ConvertFromJSONValue(const rapidjson::Value& jsonValue, T& result, const T& defaultvalue = T());

    /**
     * @brief Converts a C++ type to a rapidjson value.
     * 
     * @tparam T The C++ type to convert from.
     * @param data The C++ variable to convert.
     * @param jsonValue The rapidjson value to store the result.
     * @param allocator The allocator to use for creating new rapidjson values.
     */
    template <typename T>
    static void ConvertToJSONValue(const T& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator);

    //------------------------------------------
    // Document traversal

    /**
     * @brief Traverses rapidjson value to find a value within identified by its key.
     * 
     * @param key The key to search for.
     * @param val The rapidjson value to search within.
     * @return A pointer to the found rapidjson value, or nullptr if not found.
     */
    static rapidjson::Value* traverse_path(const char* key, rapidjson::Value& val);

    /**
     * @brief Traverses a rapidjson value to find or create a value within identified by its key.
     * 
     * @param key The key to search for.
     * @param val The rapidjson value to search within.
     * @param allocator The allocator to use for creating new values.
     * @return A pointer to the found or created rapidjson value.
     */
    static rapidjson::Value* ensure_path(const char* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

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
    static rapidjson::Value* traverse_to_parent(const char* fullKey, rapidjson::Value& root, std::string& finalKey, int& arrayIndex);

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
     */
    static void deserialize(rapidjson::Document& doc, std::string serialOrLink);
    

    //------------------------------------------
    // Helper functions

    /**
     * @brief Sorts a rapidjson value, including all its sub-values.
     * 
     * @param value The rapidjson value to sort.
     * @param allocator The allocator to use for creating new rapidjson values.
     * @return A new rapidjson value representing the sorted input.
     */
    static rapidjson::Value sortRecursive(const rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator);

    /**
     * @brief Strips comments from a JSONC string for a JSON-compatible output.
     * 
     * @param jsonc The JSONC string to process.
     * @return The JSON-compatible string.
     */
    static std::string stripComments(const std::string& jsonc);

    /**
     * @brief Empties a rapidjson document.
     * 
     * @param doc The rapidjson document to empty.
     */
    static void empty(rapidjson::Document &doc);

    /**
     * @brief Removes a member from a rapidjson object by key.
     */
    static void remove_member(const char* key, rapidjson::Value& val);
};
} // namespace Nebulite::Utility

//------------------------------------------
// Direct access get/set

template <typename T>
T Nebulite::Utility::RjDirectAccess::get(const char* key, const T defaultValue, rapidjson::Value& val) {
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
void Nebulite::Utility::RjDirectAccess::set(const char* key, const T& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
    // Ensure key path exists
    rapidjson::Value* keyVal = Nebulite::Utility::RjDirectAccess::ensure_path(key, val, allocator);
    if (keyVal != nullptr) {
        Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<T>(value, *keyVal, allocator);
    } else {
        std::cerr << "Failed to create or access path: " << key << std::endl;
    }
}

//------------------------------------------
// All conversion variants from/to rapidjson values

//------------------------------------------
// 1.) to JSON value
//------------------------------------------

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<bool>(const bool& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)              {jsonValue.SetBool(data);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<int>(const int& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)                {jsonValue.SetInt(data);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<uint32_t>(const uint32_t& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)      {jsonValue.SetUint(data);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<uint64_t>(const uint64_t& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)      {jsonValue.SetUint64(data);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<double>(const double& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)          {jsonValue.SetDouble(data);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<long>(const long& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)              {jsonValue.SetInt64(data);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<long long>(const long long& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)    {jsonValue.SetInt64(data);}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<std::string>(const std::string& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    jsonValue.SetString(
        data.c_str(), 
        static_cast<rapidjson::SizeType>(data.length()), allocator
    );
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<const char*>(const char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    if (data) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<char*>(char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data) {
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
template <> inline void Nebulite::Utility::RjDirectAccess::ConvertToJSONValue(
    const simpleValue& data, 
    rapidjson::Value& jsonValue, 
    rapidjson::Document::AllocatorType& allocator) {
    
    std::visit([&](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        ConvertToJSONValue<T>(value, jsonValue, allocator);
    }, data);
}

//------------------------------------------
// 2.) from JSON Value
//------------------------------------------

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, bool& result, const bool& defaultvalue){result = jsonValue.GetBool();}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, int& result, const int& defaultvalue) {
    if (jsonValue.IsInt()) {
        result = jsonValue.GetInt();
    }
    else if(jsonValue.IsBool()){
        result = jsonValue.GetBool();
    }
    else{
        result = 0;
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, uint32_t& result, const uint32_t& defaultvalue){result = jsonValue.GetUint();}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, uint64_t& result, const uint64_t& defaultvalue){
    if (jsonValue.IsString()) {
        std::istringstream iss(jsonValue.GetString());
        iss >> result;
    } else if (jsonValue.IsUint64()) {
        result = jsonValue.GetUint64();
    } else if (jsonValue.IsUint()) {
        result = static_cast<uint64_t>(jsonValue.GetUint());
    } else if (jsonValue.IsInt64() && jsonValue.GetInt64() >= 0) {
        result = static_cast<uint64_t>(jsonValue.GetInt64());
    } else {
        throw std::runtime_error("JSON value is not a valid uint64_t");
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, double& result, const double& defaultvalue){
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

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, std::string& result, const std::string& defaultvalue){
    if (jsonValue.IsBool()) {
        result = jsonValue.GetBool() ? "true" : "false";
    }
    else if (jsonValue.IsString()) {
        result = std::string(jsonValue.GetString());
    }
    else if (jsonValue.IsInt()) {
        result = std::to_string(jsonValue.GetInt());
    }
    else if (jsonValue.IsUint()) {
        result = std::to_string(jsonValue.GetUint());
    }
    else if (jsonValue.IsInt64()) {
        result = std::to_string(jsonValue.GetInt64());
    }
    else if (jsonValue.IsUint64()) {
        result = std::to_string(jsonValue.GetUint64());
    }
    else if (jsonValue.IsDouble()) {
        result = std::to_string(jsonValue.GetDouble());
    }
    else if (jsonValue.IsNull()) {
        result = "null";
    }
    else if (jsonValue.IsArray()) {
        result = "{Array}";
    }
    else if (jsonValue.IsObject()) {
        result = "{Object}";  // Just a placeholder since objects can't easily be converted to a single string
    }
    else {
        result = "unsupported type";
    }
}

template <> inline void Nebulite::Utility::RjDirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, rapidjson::Document& result, const rapidjson::Document& defaultvalue){
    result.CopyFrom(jsonValue, result.GetAllocator());
}