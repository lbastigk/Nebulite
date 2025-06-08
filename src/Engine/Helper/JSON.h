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
#include <map>
#include <unordered_map>
#include "absl/container/flat_hash_map.h"

// Internal dependencies
#include "JSONHandler.h"

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
        void deserialize(std::string serial_or_link);              // if key is empty, deserialize entire doc

        // flushing map content into doc
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
            std::unordered_map<std::type_index, SimpleJSONValue> derived_values;
        };
        absl::flat_hash_map<std::string, CacheEntry> cache;

        // TODO:
        // Work in progress idea: additionally to the cache, 
        // if a type for cache[key] is requested that does not match the type of cache[key]
        // store the converted value
        // Naming idea: type-differing-cache
        // This reduces conversion overhead, example:
        // -> stored type is string
        // -> math functions keep requestion double
        // 
        // Importantly, the type of cache is the main type, as this was the actual set-type
        //
        // To consider:
        // - on set, the type-differering-cache for that key has to be deleted
        // - on get, a type comparison between expected return value and stored type        
        //
        // Example of usage:
        /*
            Nebulite::JSON json;
            json.set<std::string>("number","1234");     // storage: {"number" : "1234"}
                                                        // full explanation of this function later on
            std::cout << json.get<int>("number");       // get calls get_type, notices that std::string is stored instead
                                                        // checks derived cache, no conversion stored
                                                        // converts string to int, stores int in derived cache
                                                        // returns value from derived_cache
            std::cout << json.get<int>("number");       // get calls get_type, notices that std::string is stored instead
                                                        // checks derived cache, conversion stored
                                                        // returns value from derived cache -> faster, no stoi used
            json.set<int>("number",12345)               // Delete derived cache for that key, as it doesnt match the master value in cache anymore
                                                        // no comparison needed, just assume a new set is always different
        */

        // Idea for helper functions that are called after validation that key is in cache:
        // these functions are called inside get/set once it is validated that cache[key] exists

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
    };
}

/*
template <typename T>
T Nebulite::JSON::convert_variant(const SimpleJSONValue& val, const T& defaultValue) {
    return std::visit([&](const auto& stored) -> T {
        using StoredT = std::decay_t<decltype(stored)>;

        if constexpr (std::is_convertible_v<StoredT, T>) {
            return static_cast<T>(stored);
        } else if constexpr (std::is_same_v<StoredT, std::string> && std::is_arithmetic_v<T>) {
            try {
                if constexpr (std::is_integral_v<T>)
                    return static_cast<T>(std::stoll(stored));
                else
                    return static_cast<T>(std::stod(stored));
            } catch (...) {
                return defaultValue;
            }
        } else if constexpr (std::is_arithmetic_v<StoredT> && std::is_same_v<T, std::string>) {
            return std::to_string(stored);
        } else {
            return defaultValue;
        }
    }, val);
}
*/

template <typename T>
T Nebulite::JSON::convert_variant(const SimpleJSONValue& val, const T& defaultValue) {
    return std::visit([&](const auto& stored) -> T {
        using StoredT = std::decay_t<decltype(stored)>;

        // Types: arithmetic, string 
        // To consider:
        // - Direct
        // - arithmetic -> string
        // - string     -> arithmetic
        // Not: 

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
        // if not found in cache, access actual doc through fallback
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
// Todo: new implementation of ConvertFromJSONValue inside Nebuute::JSON class
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
        JSONHandler::ConvertFromJSONValue<T>(*keyVal, tmp, defaultValue);
        return tmp;
    }
}

template <typename T>
void Nebulite::JSON::fallback_set(const char* key, const T& value, rapidjson::Value& val) {
    // Ensure key path exists
    rapidjson::Value* keyVal = ensure_path(key, val, doc.GetAllocator());
    if (keyVal != nullptr) {
        JSONHandler::ConvertToJSONValue<T>(value, *keyVal, doc.GetAllocator());
    } else {
        std::cerr << "Failed to create or access path: " << key << std::endl;
    }
}

