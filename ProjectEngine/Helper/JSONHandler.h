#pragma once

#ifdef _WIN32 

#include "rapidjson-master/include/rapidjson/document.h"
#include "rapidjson-master/include/rapidjson/writer.h"
#include "rapidjson-master/include/rapidjson/stringbuffer.h"
#include "rapidjson-master/include/rapidjson/prettywriter.h"
#include "rapidjson-master/include/rapidjson/encodings.h"
#include "rapidjson-master/include/rapidjson/istreamwrapper.h"
#include "rapidjson-master/include/rapidjson/ostreamwrapper.h"

#else

#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"
#include "encodings.h"
#include "istreamwrapper.h"
#include "ostreamwrapper.h"

#endif

#include <type_traits>
#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <vector>

#include "FileManagement.h"



//using namespace rapidjson;


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

//One space to handle Rapidjson read/write operations
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

    static rapidjson::Document deserialize(std::string serialOrLink);
    static std::string serialize(const rapidjson::Document& doc);
    static std::string serializeVal(const rapidjson::Value& val);
    static std::string ConvertJSONValue(rapidjson::Value& jsonValue);
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

template <typename T>
T JSONHandler::Get::Any(rapidjson::Document& doc, const std::string& fullKey, const T& defaultValue) {
    //Handle edge case where first char might be '-'
    // TODO...
    
    //Handle key nesting:
    int pos = fullKey.find('-');
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
        else {
            // Handle the case where the key is not found in the document or the document is not an object
            // You can throw an exception, log an error, or set a default value.
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
    int pos = fullKey.find('-');
    if (pos != -1) {
        //Key nesting present

        //Check if doc has member
        if (!doc.HasMember(fullKey.substr(0, pos).c_str())) {
            //Add an object to doc, with:
            // key = fullKey.substr(0, pos)
            // so that:
            //doc{"key":{}}
            rapidjson::Value newObject(rapidjson::kObjectType);
            doc.AddMember(rapidjson::StringRef(fullKey.substr(0, pos).c_str()), newObject, doc.GetAllocator());
        }

        //Get subdoc, call set Any again
        rapidjson::Document tmp;

        //Get subdoc
        JSONHandler::Get::subDoc(doc, fullKey.substr(0, pos), tmp);

        //manipulate temp
        Any(tmp, fullKey.substr(pos + 1), data, onlyIfExists);

        //Insert temp back into main doc
        subDoc(doc, fullKey.substr(0, pos), tmp);
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
