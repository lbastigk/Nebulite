#pragma once
/*
Nebulite::JSON is meant as a rapidjson wrapper to streamline the trivial json requests like get and set that Nebulite does
A cache is implemented for fast setting/getting of keys. Only if needed are those values flushed into the document
*/

// Rapidjson dependencies
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"
#include "encodings.h"
#include "istreamwrapper.h"
#include "ostreamwrapper.h"

// Other dependencies
#include <typeinfo>
#include <cxxabi.h>
#include <string>
#include <variant>
#include <type_traits>
#include <typeindex>
#include "absl/container/flat_hash_map.h"

// Internal dependencies
//#include "JSONHandler.h"
#include "FileManagement.h"

namespace Nebulite{
    // Template for supported cache storages
    template <typename T>
    struct is_simple_value : std::disjunction<
        std::is_same<T, int32_t>,
        std::is_same<T, int64_t>,
        std::is_same<T, uint32_t>,
        std::is_same<T, uint64_t>,
        std::is_same<T, double>,
        std::is_same<T, float>,
        std::is_same<T, std::string>,
        std::is_same<T, bool>
    > {};

    template <typename T>
    inline constexpr bool is_simple_value_v = is_simple_value<T>::value;
    
    // JSON class for wrapping rapidjson (JSONHandler for now), 
    // allowing for Nebulite::JSON objects with fast caching of variables
    // that are flushed into the rapidjson doc on serialization
    class JSON{
    public:
        JSON();

        // Reserved operative characters that cant be used for keynames
        static std::string reservedCharacters;

        // Get any value
        template <typename T> T get(const char* key, const T defaultValue = T());
        Nebulite::JSON get_subdoc(const char* key);

        // Set any value
        template <typename T> void set(const char* key, const T& value);
        void set_subdoc(const char* key, Nebulite::JSON& child);

        // Set empty
        void set_empty_array(const char* key);

        // Get type of key
        enum KeyType{
            document = -1,
            null = 0,
            value = 1,
            array = 2
        };
        KeyType memberCheck(std::string key);

        // Get size of key
        // -1 - is document
        // 0  - key doesnt exist
        // 1  - standard key
        // >1 - array
        uint32_t memberSize(std::string key);

        uint32_t cacheSize(){return cache.size();};

        // Serializing/Deserializing
        std::string serialize(std::string key = "");
        void deserialize(std::string serial_or_link);              // if key is empty, deserializes entire doc

        // flushing map content into doc
        // TODO:
        // While the current implementation of flushing break for more complex data structures, 
        // due to the handling of cache if keys are set 
        // the current usecase does not intend to use them
        // More testing needed to find issues that need to be resolved in set/flush
        // Low priority
        void flush();

        // Empty document
        void empty();

        // For compatiblity with older systems, get doc directly:
        rapidjson::Document* getDoc() const {
            return const_cast<rapidjson::Document*>(&doc);
        }
    private:
        //--------------------------------------------------------------------
        // Value storage

        // main doc
        rapidjson::Document doc;

        //--------------------------------------------------------------------
        // caching Simple variables
        using SimpleJSONValue = std::variant<
            int32_t, int64_t, 
            uint32_t, uint64_t,
            float, double, 
            std::string, 
            bool
        >;
        struct CacheEntry {
            SimpleJSONValue main_value;
            absl::flat_hash_map<std::type_index, SimpleJSONValue> derived_values;
        };
        absl::flat_hash_map<std::string, CacheEntry> cache;

        //--------------------------------------------------------------------
        // Helper functions for get/set:
        // they are called once it is validated that cache[key] exists

        // set into cache and clear derived_cache
        template <typename T> void set_type(std::string key, const T& value);

        // get from cache 
        // if T does not match cache, get drom derived_cache
        // if derived_cache does not contain value, convert from cache to derived_cache, store:
        template <typename T> T get_type(CacheEntry& entry, const T& defaultValue);

        template <typename T>
        T convert_variant(const SimpleJSONValue& val, const T& defaultValue = T());

        //--------------------------------------------------------------------
        // Fallback get and set

        // Get any value
        template <typename T> T fallback_get(const char* key, const T defaultValue, rapidjson::Value& val);

        // Set any value
        template <typename T> void fallback_set(const char* key, const T& value, rapidjson::Value& val);

        //--------------------------------------------------------------------
        // Document traversal

        rapidjson::Value* traverseKey(const char* key, rapidjson::Value& val);

        rapidjson::Value* ensure_path(const char* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);

        //--------------------------------------------------------------------
        // Important Helper functions
        class Helper{
        public:
            template <typename T>
            static void Set(rapidjson::Document& doc, const std::string& fullKey, const T data); 

            template <typename T>
            static void ConvertFromJSONValue(const rapidjson::Value& jsonValue, T& result, const T& defaultvalue = T());

            template <typename T>
            static void ConvertToJSONValue(const T& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator);

            static rapidjson::Value sortRecursive(const rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator);
            static rapidjson::Document deserialize(std::string serialOrLink);
            static std::string serialize(const rapidjson::Document& doc);
            static void empty(rapidjson::Document &doc);
        };
    };
}

template <typename T>
T Nebulite::JSON::convert_variant(const SimpleJSONValue& val, const T& defaultValue) {
    return std::visit([&](const auto& stored) -> T {
        using StoredT = std::decay_t<decltype(stored)>;

        // Directly cast if types are convertible
        if constexpr (std::is_convertible_v<StoredT, T>) {
            return static_cast<T>(stored);
        }

        // Handle string to bool (optional, but common)
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<T, bool>) {
            return stored == "true" || stored == "1";
        }

        // Handle string to arithmetic
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_arithmetic_v<T>) {
            try {
                if constexpr (std::is_integral_v<T>) {
                    if constexpr (std::is_signed_v<T>)
                        return static_cast<T>(std::stoll(stored));
                    else
                        return static_cast<T>(std::stoull(stored));
                } else {
                    return static_cast<T>(std::stod(stored));
                }
            } catch (...) {
                return defaultValue;
            }
        }

        // Handle arithmetic to string
        else if constexpr (std::is_arithmetic_v<StoredT> && std::is_same_v<T, std::string>) {
            return std::to_string(stored);
        }

        // Fallback
        else {
            return defaultValue;
        }
    }, val);
}

template <typename T>
void Nebulite::JSON::set_type(std::string key, const T& value) {
    CacheEntry& entry = cache[key];     // creates or updates entry
    entry.main_value = value;
    entry.derived_values.clear();       // reset derived types
}


template <typename T>
T Nebulite::JSON::get_type(CacheEntry& entry, const T& defaultValue) {
    // Check if the main value holds the exact type
    if (std::holds_alternative<T>(entry.main_value)) {
        return std::get<T>(entry.main_value);
    }

    // Check the derived values cache
    auto it = entry.derived_values.find(std::type_index(typeid(T)));
    if (it != entry.derived_values.end()) {
        return std::get<T>(it->second);
    }

    // Convert and cache it
    T converted = convert_variant<T>(entry.main_value, defaultValue);
    entry.derived_values[std::type_index(typeid(T))] = converted;
    return converted;
}

template <typename T>
T Nebulite::JSON::get(const char* key, const T defaultValue) {
    if constexpr (is_simple_value_v<T> || std::is_same_v<T, const char*>) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            return get_type<T>(it->second, defaultValue);
        }
        // if not found in cache, access actual doc through fallback and store in cache
        T tmp = fallback_get<T>(key, defaultValue, doc);
        set(key,tmp);
        return tmp;
    }
    // Fallback to doc
    return fallback_get<T>(key, defaultValue, doc);
}

template <typename T>
void Nebulite::JSON::set(const char* key, const T& value) {
    if constexpr (is_simple_value_v<T>) {
        set_type(key,value);
    } 
    else if constexpr (std::is_same_v<T, const char*> || (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>)) {
        // Convert char arrays and const char* to std::string
        set_type(key,std::string(value));
    } 
    else {
        // Remove from cache to prevent type mismatch
        cache.erase(key);
        fallback_set<T>(key, value, doc);
    }
}

//---------------------------------------------------------------------
// Fallbacks
// Todo: new implementation of ConvertFromJSONValue inside Nebulite::JSON class
// Perhaps it's possible to remove some of the base checks/Make the code cleaner in general

template <typename T>
T Nebulite::JSON::fallback_get(const char* key, const T defaultValue, rapidjson::Value& val) {
    rapidjson::Value* keyVal = traverseKey(key,val);
    if(keyVal == nullptr){
        // Value doesnt exist in doc, return default
        return defaultValue;
    }
    else{
        // Base case: convert currentVal to T using JSONHandler
        T tmp;
        Nebulite::JSON::Helper::ConvertFromJSONValue<T>(*keyVal, tmp, defaultValue);
        return tmp;
    }
}

template <typename T>
void Nebulite::JSON::fallback_set(const char* key, const T& value, rapidjson::Value& val) {
    // Ensure key path exists
    rapidjson::Value* keyVal = ensure_path(key, val, doc.GetAllocator());
    if (keyVal != nullptr) {
        Nebulite::JSON::Helper::ConvertToJSONValue<T>(value, *keyVal, doc.GetAllocator());
    } else {
        std::cerr << "Failed to create or access path: " << key << std::endl;
    }
}






//----------------------------------------------------------------------
// Helper functions from older JSONHandler Class

template <typename T>
void Nebulite::JSON::Helper::Set(rapidjson::Document& doc, const std::string& fullKey, const T data) {
    if (!doc.IsObject()) {
        doc.SetObject();
    }

    rapidjson::Value* current = &doc;
    std::string_view keyView(fullKey);

    while (!keyView.empty()) {
        size_t dotPos = keyView.find('.');
        std::string_view key = (dotPos == std::string_view::npos) ? keyView : keyView.substr(0, dotPos);

        if (!current->IsObject()) {
            current->SetObject();
        }

        if (dotPos != std::string_view::npos) {
            // Not the final key: ensure nested object exists
            if (!current->HasMember(std::string(key).c_str())) {
                rapidjson::Value keyVal(std::string(key).c_str(), doc.GetAllocator());
                rapidjson::Value newObj(rapidjson::kObjectType);
                current->AddMember(keyVal, newObj, doc.GetAllocator());
            }
            current = &(*current)[std::string(key).c_str()];
            keyView.remove_prefix(dotPos + 1);
        } 
        else {
            // Final key: set the value
            rapidjson::Value keyVal(std::string(key).c_str(), doc.GetAllocator());
            rapidjson::Value jsonValue;
            ConvertToJSONValue(data, jsonValue, doc.GetAllocator());

            if (current->HasMember(std::string(key).c_str())) {
                (*current)[std::string(key).c_str()] = jsonValue;
            } 
            else {
                current->AddMember(keyVal, jsonValue, doc.GetAllocator());
            }
            break;
        }
    }
}

// to JSON value
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<bool>(const bool& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetBool(data);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<int>(const int& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetInt(data);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<uint32_t>(const uint32_t& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetUint(data);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<uint64_t>(const uint64_t& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetUint64(data);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<float>(const float& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetFloat(data);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<double>(const double& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetDouble(data);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<long>(const long& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){jsonValue.SetInt64(data);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<std::string>(const std::string& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    jsonValue.SetString(data.c_str(), static_cast<rapidjson::SizeType>(data.length()), allocator);
}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<const char*>(const char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<char*>(char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<std::pair<int, std::string>>(const std::pair<int, std::string>& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
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
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<rapidjson::Value*>(rapidjson::Value* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {jsonValue.CopyFrom(*data, allocator);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<rapidjson::Document*>(rapidjson::Document* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {jsonValue.CopyFrom(*data, allocator);}
template <> inline void Nebulite::JSON::Helper::ConvertToJSONValue<rapidjson::Document>(const rapidjson::Document& data,rapidjson::Value& jsonValue,rapidjson::Document::AllocatorType& allocator) {jsonValue.CopyFrom(data, allocator);}

// from JSON Value
template <> inline void Nebulite::JSON::Helper::ConvertFromJSONValue(const rapidjson::Value& jsonValue, bool& result, const bool& defaultvalue){result = jsonValue.GetBool();}
template <> inline void Nebulite::JSON::Helper::ConvertFromJSONValue(const rapidjson::Value& jsonValue, int& result, const int& defaultvalue) {
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
template <> inline void Nebulite::JSON::Helper::ConvertFromJSONValue(const rapidjson::Value& jsonValue, uint32_t& result, const uint32_t& defaultvalue){result = jsonValue.GetUint();}
template <> inline void Nebulite::JSON::Helper::ConvertFromJSONValue(const rapidjson::Value& jsonValue, uint64_t& result, const uint64_t& defaultvalue) {
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
template <> inline void Nebulite::JSON::Helper::ConvertFromJSONValue(const rapidjson::Value& jsonValue, float& result, const float& defaultvalue){
    if (jsonValue.IsNumber()){
        result = jsonValue.GetFloat();
    }
    else{
        result = (float)std::stod(jsonValue.GetString());
    }
}
template <> inline void Nebulite::JSON::Helper::ConvertFromJSONValue(const rapidjson::Value& jsonValue, double& result, const double& defaultvalue){
    if (jsonValue.IsNumber()){
        result = jsonValue.GetDouble();
    }
    else if(jsonValue.IsString()){
        result = std::stod(jsonValue.GetString());
    }
    else {
        result = defaultvalue;
    }
}
template <> inline void Nebulite::JSON::Helper::ConvertFromJSONValue(const rapidjson::Value& jsonValue, std::string& result, const std::string& defaultvalue){
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
template <> inline void Nebulite::JSON::Helper::ConvertFromJSONValue(const rapidjson::Value& jsonValue, rapidjson::Document& result, const rapidjson::Document& defaultvalue){
    result.CopyFrom(jsonValue, result.GetAllocator());
}
