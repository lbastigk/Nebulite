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


//-------------------------------------------------------
//&Doc template specializations
//need to be global for C++
template <typename T>
struct is_rapidjson_document_ptr : std::false_type {};

template <>
struct is_rapidjson_document_ptr<rapidjson::Document*> : std::true_type {};

template <typename T>
struct is_rapidjson_value_ptr : std::false_type {};
template <>
struct is_rapidjson_value_ptr<rapidjson::Value&> : std::true_type {};


class JSONHandler {
public:
    class Get {
    public:
        template <typename T>
        static T Any(rapidjson::Document& doc, const std::string& fullKey, const T& defaultValue = T());
        static void subDocOld(rapidjson::Document& doc, const std::string& key, rapidjson::Document& destination);
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
    //Template structs to check for datatype matches

    //Map
    template <typename T>
    struct is_map : std::false_type {};

    template <typename Key, typename Value, typename... Args>
    struct is_map<std::map<Key, Value, Args...>> : std::true_type {};

    //Vector
    template <typename T>
    struct is_vector : std::false_type {};

    template <typename T>
    struct is_vector<std::vector<T>> : std::true_type {};

    //Pair
    template <typename T>
    struct is_pair : std::false_type {};

    template <typename T1, typename T2>
    struct is_pair<std::pair<T1, T2>> : std::true_type {};

    //----------------------------------------------------------------------
    // Helper function to convert data to JSON values (specializations may be required for custom types)
    template <typename T>
    static void ConvertToJSONValue(const T& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator);
    template <typename T>
    static void ConvertFromJSONValue(const rapidjson::Value& jsonValue, T& result);
};

// Defining template functions here, otherwise the linker spits out errors en masse...

template <typename T>
T JSONHandler::Get::Any(rapidjson::Document& doc, const std::string& fullKey, const T& defaultValue) {
    //Handle key nesting:
    int pos = fullKey.find('.');
    if (pos != -1) {
        //Key nesting present, get subdoc, call get Any again
        rapidjson::Document tmp;

        //Get subdoc
        JSONHandler::Get::subDoc(doc, fullKey.substr(0, pos), tmp);

        //Get subvalue from doc
        return Any(tmp, fullKey.substr(pos + 1), defaultValue);
    }
    else {
        //No key nesting, return value from doc
        if (doc.IsObject() && doc.HasMember(fullKey.c_str())) {
            const rapidjson::Value& value = doc[fullKey.c_str()];

            if (!value.IsNull()) {
                try {
                    T result;
                    ConvertFromJSONValue(value, result);
                    return result;
                }
                catch (const std::exception&) {
                    // Handle potential conversion errors here
                    // You can throw an exception, log an error, or handle the error in any way you prefer.
                }
            }
            else {
                // Handle the case where the value is null or missing
                // You can throw an exception, log an error, or set a default value.
            }
        }

        // Return the default value if any error occurs
        return defaultValue;
    }  
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


//----------------------------------------------------------------------
// Helper functions to convert data to JSON values (specializations may be required for custom types)

//TODO:
// Switch to specific defines using fun<type> instead of if-elses, example:
// template<>
// void JSONHandler::ConvertToJSONValue<int>(const int& value, rapidjson::GenericValue<rapidjson::UTF8<char>>& jsonValue, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) {
//     jsonValue.SetInt(value);
// }

template <typename T>
void JSONHandler::ConvertToJSONValue(const T& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    //Single Variable Objects
    if constexpr (std::is_same_v<T, bool>) {
        jsonValue.SetBool(data);
    }
    else if constexpr (std::is_same_v<T, int>) {
        jsonValue.SetInt(data);
    }
    else if constexpr (std::is_same_v<T, float>) {
        jsonValue.SetFloat(data);
    }
    else if constexpr (std::is_same_v<T, double>) {
        jsonValue.SetDouble(data);
    }
    else if constexpr (std::is_same_v<T, float>) {
        jsonValue.SetDouble(static_cast<double>(data));
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
template <typename T>
void JSONHandler::ConvertFromJSONValue(const rapidjson::Value& jsonValue, T& result) {
    // Implement the reverse conversion logic for each supported type
    if constexpr (std::is_same_v<T, bool>) {
        result = jsonValue.GetBool();
    }
    else if constexpr (std::is_same_v<T, int>) {
        result = jsonValue.GetInt();
    }
    else if constexpr (std::is_same_v<T, float>) {
        result = jsonValue.GetFloat();
    }
    else if constexpr (std::is_same_v<T, double>) {
        result = jsonValue.GetDouble();
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
