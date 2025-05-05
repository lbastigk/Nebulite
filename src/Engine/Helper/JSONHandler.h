#pragma once

// CLASS: JSONHandler
//
// A tool to unify rapidjson document handling in one place
// and make modifications easier
//

//-------------------------------------------------------
// Dependencies

// Rapidjson dependencies
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"
#include "encodings.h"
#include "istreamwrapper.h"
#include "ostreamwrapper.h"

// Nebulite dependencies
#include "FileManagement.h"

// Other dependencies
#include <type_traits>
#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <vector>

class JSONHandler {
public:
    class Get {
    public:
        template <typename T, typename JSONValueType>
        static T Any(JSONValueType& value, const std::string& fullKey, const T& defaultValue);
        static void subDoc(rapidjson::Document& doc, const std::string& key, rapidjson::Document& destination);
        static void listOfKeys(rapidjson::Document& doc, std::vector<std::string>& keys);
        static int keyAmount(rapidjson::Document& doc);
    };

    class Set {
    public:
        template <typename T>
        static void Any(rapidjson::Document& doc, const std::string& fullKey, const T data, bool onlyIfExists = false);
        
        static void subDoc(rapidjson::Document& doc, const std::string& key, rapidjson::Value& subdoc);
    };

    // General JSONHandler functions
    
    // Serialization and deserialization
    static rapidjson::Document deserialize(std::string serialOrLink);
    static std::string serialize(const rapidjson::Document& doc);
    static std::string serializeVal(const rapidjson::Value& val);
    static std::string ConvertJSONValue(rapidjson::Value& jsonValue);

    static bool isValid(std::string str);
    static void copyDoc(rapidjson::Document& destination, rapidjson::Document *toCopy);
    static void empty(rapidjson::Document &doc);
private:

    //----------------------------------------------------------------------
    // Helper function to convert data to JSON values (specializations may be required for custom types)
    template <typename T>
    static void ConvertToJSONValue(const T& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator);

    template <typename K, typename V> 
    static inline void ConvertToJSONValue(const std::map<K, V>& data,rapidjson::Value& jsonValue,rapidjson::Document::AllocatorType& allocator) {
        rapidjson::Value jsonMap(rapidjson::kObjectType);

        for (const auto& entry : data) {
            rapidjson::Value key(entry.first.c_str(), allocator);
            rapidjson::Value value;
            ConvertToJSONValue(entry.second, value, allocator);
            jsonMap.AddMember(key, value, allocator);
        }
        jsonValue = jsonMap;
    }
    template <typename T>
    static inline void ConvertToJSONValue(const std::vector<T>& data,rapidjson::Value& jsonValue,rapidjson::Document::AllocatorType& allocator) {
        rapidjson::Value jsonArray(rapidjson::kArrayType);

        for (const auto& item : data) {
            rapidjson::Value itemValue;
            ConvertToJSONValue(item, itemValue, allocator);
            jsonArray.PushBack(itemValue, allocator);
        }
        jsonValue = jsonArray;
    }


    template <typename T>
    static void ConvertFromJSONValue(const rapidjson::Value& jsonValue, T& result);
};


template <typename T, typename JSONValueType>
T JSONHandler::Get::Any(JSONValueType& value, const std::string& fullKey, const T& defaultValue) {
    const rapidjson::Value* current = &value;
    size_t start = 0;

    while (start < fullKey.length()) {
        size_t end = fullKey.find('.', start);
        std::string key = (end == std::string::npos)
                          ? fullKey.substr(start)
                          : fullKey.substr(start, end - start);

        if (!current->IsObject() || !current->HasMember(key.c_str())) {
            return defaultValue;
        }

        current = &(*current)[key.c_str()];

        if (end == std::string::npos) break;
        start = end + 1;
    }

    if (!current->IsNull()) {
        try {
            T result;
            ConvertFromJSONValue(*current, result);
            return result;
        } catch (const std::exception&) {
            // Handle conversion failure
        }
    }
    return defaultValue;
}

template <typename T>
void JSONHandler::Set::Any(rapidjson::Document& doc, const std::string& fullKey, const T data, bool onlyIfExists) {
    
    // Ensure that doc is initialized as an object
    if (!doc.IsObject()) {
        doc.SetObject();
    }

    //Handle edge case where first char might be '-'
    // TODO...

    //Handle key nesting:
    int pos = fullKey.find('.');
    if (pos != -1) {
        //Key nesting present, go key by key:
        // e.g. key1.rest , meaning rest could be one key or more.
        // 1.) load subdoc of key1
        // 2.) call function recursively on subdoc, this time using rest as key
        // 3.) insert subdoc into main doc

        //-------------------------------------
        // 1.)
        // Check if doc does not have member
        // create member
        if (!doc.HasMember(fullKey.substr(0, pos).c_str())) {
            rapidjson::Value key(fullKey.substr(0, pos).c_str(), doc.GetAllocator());
            rapidjson::Value value(rapidjson::kObjectType);
            doc.AddMember(key, value, doc.GetAllocator());
        }

        //Get subdoc, call set Any again
        rapidjson::Document tmp;

        //Get subdoc
        JSONHandler::Get::subDoc(doc, fullKey.substr(0, pos), tmp);

        //-------------------------------------
        // 2.)
        //manipulate temp subdoc
        JSONHandler::Set::Any(tmp, fullKey.substr(pos + 1), data, onlyIfExists);

        //-------------------------------------
        // 3.)
        //Insert temp back into main doc
        JSONHandler::Set::subDoc(doc, fullKey.substr(0, pos), tmp);

        //JSONHandler::empty(tmp);  // not needed
    }
    else {
        //No key nesting

        // Convert the data to a JSON value using the helper function
        rapidjson::Value jsonValue;
        ConvertToJSONValue(data, jsonValue, doc.GetAllocator());

        // Add the JSON value to the document with the specified name
        rapidjson::Value jsonVarName(fullKey.c_str(), doc.GetAllocator());
        if (doc.HasMember(jsonVarName)) {
            doc[jsonVarName] = jsonValue;
        }
        else if (!onlyIfExists) {
            doc.AddMember(jsonVarName, jsonValue, doc.GetAllocator());
        }
    }    
}

template <typename T>
void _Any(rapidjson::Document& doc, const std::string& fullKey, const T data, bool onlyIfExists) {
    if (!doc.IsObject()) {
        doc.SetObject();
    }

    rapidjson::Value* current = &doc;
    size_t start = 0;

    // Traverse or create nested objects
    while (true) {
        size_t end = fullKey.find('.', start);
        std::string key = (end == std::string::npos)
                          ? fullKey.substr(start)
                          : fullKey.substr(start, end - start);

        if (!current->IsObject()) {
            current->SetObject();
        }

        if (end != std::string::npos) {
            // We're not at the final key, ensure the sub-object exists
            if (!current->HasMember(key.c_str())) {
                rapidjson::Value keyVal(key.c_str(), doc.GetAllocator());
                rapidjson::Value newObj(rapidjson::kObjectType);
                current->AddMember(keyVal, newObj, doc.GetAllocator());
            }

            current = &(*current)[key.c_str()];
            start = end + 1;
        } else {
            // Final key reached: set the value
            rapidjson::Value keyVal(key.c_str(), doc.GetAllocator());
            rapidjson::Value jsonValue;
            ConvertToJSONValue(data, jsonValue, doc.GetAllocator());

            if (current->HasMember(key.c_str())) {
                (*current)[key.c_str()] = jsonValue;
            } else if (!onlyIfExists) {
                current->AddMember(keyVal, jsonValue, doc.GetAllocator());
            }
            break;
        }
    }
}


//----------------------------------------------------------------------
// Helper functions to convert data to or from JSON values (specializations may be required for custom types)


// 
template <> inline void JSONHandler::ConvertToJSONValue<bool>(const bool& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetBool(data);}
template <> inline void JSONHandler::ConvertToJSONValue<int>(const int& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetInt(data);}
template <> inline void JSONHandler::ConvertToJSONValue<uint32_t>(const uint32_t& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetUint(data);}
template <> inline void JSONHandler::ConvertToJSONValue<uint64_t>(const uint64_t& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetUint64(data);}
template <> inline void JSONHandler::ConvertToJSONValue<float>(const float& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetFloat(data);}
template <> inline void JSONHandler::ConvertToJSONValue<double>(const double& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetDouble(data);}

template <> inline void JSONHandler::ConvertToJSONValue<std::string>(const std::string& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    jsonValue.SetString(data.c_str(), static_cast<rapidjson::SizeType>(data.length()), allocator);
}

template<>
inline void JSONHandler::ConvertToJSONValue<const char*>(const char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template<>
inline void JSONHandler::ConvertToJSONValue<char*>(char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template<>
inline void JSONHandler::ConvertToJSONValue<std::pair<int, std::string>>(const std::pair<int, std::string>& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    // Example conversion for a pair. You may adapt based on your application’s structure.
    rapidjson::Value pairObject(rapidjson::kObjectType);
    rapidjson::Value firstValue;
    ConvertToJSONValue(data.first, firstValue, allocator);
    pairObject.AddMember("first", firstValue, allocator);
    
    rapidjson::Value secondValue;
    ConvertToJSONValue(data.second, secondValue, allocator);
    pairObject.AddMember("second", secondValue, allocator);
    
    jsonValue = pairObject;
}



template <>
inline void JSONHandler::ConvertToJSONValue<rapidjson::Value*>(rapidjson::Value* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {jsonValue.CopyFrom(*data, allocator);}

template <>
inline void JSONHandler::ConvertToJSONValue<rapidjson::Document*>(rapidjson::Document* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {jsonValue.CopyFrom(*data, allocator);}



/*
template <typename T>
void JSONHandler::ConvertToJSONValue(const T& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    //Single Variable Objects
    if constexpr (std::is_same_v<T, bool>) {
        jsonValue.SetBool(data);
    }
    else if constexpr (std::is_same_v<T, int>) {
        jsonValue.SetInt(data);
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
        jsonValue.SetUint(data);
    }
    else if constexpr (std::is_same_v<T, uint64_t>) {
        jsonValue.SetUint64(data);
    }
    else if constexpr (std::is_same_v<T, float>) {
        jsonValue.SetFloat(data);
    }
    else if constexpr (std::is_same_v<T, double>) {
        jsonValue.SetDouble(data);
    }

    //Strings and similiar
    else if constexpr (std::is_same_v<T, std::string>) {    //C++ string
        jsonValue.SetString(data.c_str(), allocator);
    }
    else if constexpr (std::is_same_v<T, const char*>) {    //Const char string: direct passing into argument of a string
        jsonValue.SetString(data, allocator);
    }
    else if constexpr (std::is_same_v<T, char*>) {          //char string
        jsonValue.SetString(data, allocator);
    }


    //Multivariable Objects
    else if constexpr (is_pair<T>::value) {
        // Create a JSON array for the pair
        rapidjson::Value jsonArray(rapidjson::kArrayType);

        // Convert each item in the pair to a JSON value
        rapidjson::Value firstValue;
        ConvertToJSONValue(data.first, firstValue, allocator);
        jsonArray.PushBack(firstValue, allocator);

        rapidjson::Value secondValue;
        ConvertToJSONValue(data.second, secondValue, allocator);
        jsonArray.PushBack(secondValue, allocator);

        // Write into the final jsonValue
        jsonValue = jsonArray;
    }
    else if constexpr (is_map<T>::value) {
        rapidjson::Value jsonMap(rapidjson::kObjectType);

        for (const auto& entry : data) {
            rapidjson::Value key(entry.first.c_str(), allocator);
            rapidjson::Value value;
            ConvertToJSONValue(entry.second, value, allocator);
            jsonMap.AddMember(key, value, allocator);
        }
        jsonValue = jsonMap;
    }
    else if constexpr (is_vector<T>::value) {
        // Create a JSON array for the vector
        rapidjson::Value jsonArray(rapidjson::kArrayType);

        for (const auto& item : data) {
            // Convert each item in the vector to a JSON value
            rapidjson::Value jsonValue;
            ConvertToJSONValue(item, jsonValue, allocator);

            // Add the JSON value to the array
            jsonArray.PushBack(jsonValue, allocator);
        }
        jsonValue = jsonArray;
    }
    else if constexpr (is_rapidjson_document_ptr<T>::value) {
        jsonValue.CopyFrom(*data, allocator);
    }
    else if constexpr (is_rapidjson_value_ptr<T>::value) {
        return data;
    }

    
    //ELSE, rapidjson value directly
    else {
        
    }
}

*/


// To JSON Value
template <> inline void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, bool& result){result = jsonValue.GetBool();}
template <> inline void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, int& result){result = jsonValue.GetInt();}
template <> inline void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, uint32_t& result){result = jsonValue.GetUint();}
template <> inline void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, uint64_t& result){result = jsonValue.GetUint64();}
template <> inline void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, float& result){
    if (jsonValue.IsNumber()){
        result = jsonValue.GetFloat();
    }
    else{
        result = (float)std::stod(jsonValue.GetString());
    }
}
template <> inline void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, double& result){
    if (jsonValue.IsNumber()){
        result = jsonValue.GetDouble();
    }
    else{
        result = std::stod(jsonValue.GetString());
    }}
template <> inline void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, std::string& result){
    // TODO: Does rapidjson support auto-conversion?

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
template <> inline void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, rapidjson::Document& result){
    result.CopyFrom(jsonValue, result.GetAllocator());
}

/*
template <typename T>
void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, T& result) {
    // Implement the reverse conversion logic for each supported type
    if constexpr (std::is_same_v<T, bool>) {
        result = jsonValue.GetBool();
    }
    else if constexpr (std::is_same_v<T, int>) {
        result = jsonValue.GetInt();
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
        result = jsonValue.GetUint();
    }
    else if constexpr (std::is_same_v<T, uint64_t>) {
        result = jsonValue.GetUint64();
    }
    else if constexpr (std::is_same_v<T, float>) {
        if (jsonValue.IsNumber()){
            result = jsonValue.GetFloat();
        }
        else{
            result = (float)std::stod(jsonValue.GetString());
        }
    }
    else if constexpr (std::is_same_v<T, double>) {
        if (jsonValue.IsNumber()){
            result = jsonValue.GetDouble();
        }
        else{
            result = std::stod(jsonValue.GetString());
        }
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        //allow more flexibility for return type string:

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
    else if constexpr (std::is_same_v<T, rapidjson::Document>) {
        result.CopyFrom(jsonValue, result.GetAllocator());
    }
    else if constexpr (is_rapidjson_document_ptr<T>::value) {
        result.CopyFrom(jsonValue, result->GetAllocator());
    }
    else {
        result = "null";
    }
}

*/

