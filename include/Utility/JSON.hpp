/**
 * @file JSON.hpp
 * @brief A wrapper around rapidjson to simplify JSON manipulation in Nebulite.
 */

#pragma once

//------------------------------------------
// Includes

// General
#include <mutex>
#include <typeinfo>
#include <cxxabi.h>
#include <string>
#include <variant>
#include <type_traits>
#include <typeindex>

// External
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"
#include "encodings.h"
#include "istreamwrapper.h"
#include "ostreamwrapper.h"
#include "absl/container/flat_hash_map.h"

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Utility/FileManagement.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
namespace Nebulite{
namespace Utility {

/**
 * @class Nebulite::Utility::JSON
 * 
 * @brief A wrapper around rapidjson to simplify JSON manipulation in Nebulite.
 * 
 * Features:
 * 
 * - Caching system for fast access to frequently used values
 * 
 * - Automatic casting and casted-value caching, speeding up access as any casted value is cached and no longer needs to be re-evaluated.
 * 
 * - Direct access to cached double value pointers for quick expression evaluation.
 * 
 * - Complex data operations through functioncall API.
 * 
 * Note that, due to the way caching is implemented, setting and getting is not fully logical.
 * 
 * Example:
 * 
 * Setting key: `var1.var2` first and then `var1` does not overwrite `var1.var2`.
 * If the document is then serialized, the cache is flushed and the values are written to the document.
 * Meaning the last key set will take precedence.
 * Instead, values should only be set if they are guaranteed to be simple values rather than complex objects.
 * For moving and copying complex objects, use the appropriate methods of its FuncTree.
 * 
 * Due to performance concerns, a rework of the caching system is a low priority.
 * 
 * @todo One way of fixing would be to compare any new cached values with existing ones and deleting inner caches:
 * - new cache `var1` is substring of existing cache `var1.var2` -> delete `var1.var2`
 * - Since we only need to compare new cached values with existing ones, we can optimize the process.
 */
NEBULITE_DOMAIN(JSON) {
public:
    JSON();

    ~JSON();

    //------------------------------------------
    // Overload of assign operators
    /**
     * @todo reinitialization of domain is probably missing at some spots
     */
    JSON(const JSON&) = delete;
    JSON(JSON&& other) noexcept;
    JSON& operator=(const JSON&) = delete;
    JSON& operator=(JSON&& other) noexcept 
    {
        if (this != &other) {
            std::scoped_lock lock(mtx, other.mtx);
            doc = std::move(other.doc);
            cache = std::move(other.cache);
        }
        return *this;
    }
    
    /**
     * @brief Copies the contents of another JSON object into this one.
     * 
     * This function performs a deep copy of the document and cache from the
     * other JSON object, ensuring that all data is transferred correctly.
     * 
     * @note If this fails, it might be helpful to fall back to 
     * serializing and deserializing the JSON.
     */
    void copyFrom(const JSON* other) {
        std::scoped_lock lock(mtx, other->mtx);
        Nebulite::Utility::JSON::DirectAccess::empty(doc);
        
        if (other == nullptr) return;  // safeguard
        
        // Copy document content
        doc.SetObject();
        doc.CopyFrom(other->doc, doc.GetAllocator());

        // Copy cache
        cache.clear();
        cache = other->cache;
    }

    /**
     * @brief A list of reserved characters that cannot be used in key names.
     */
    const static std::string reservedCharacters;

    //------------------------------------------
    // Getter

    /**
     * @brief Gets a value from the JSON document.
     * 
     * This function retrieves a value of the specified type from the JSON document.
     * If the key does not exist, the default value is returned.
     * 
     * @tparam T The type of the value to retrieve.
     * @param key The key of the value to retrieve.
     * @param defaultValue The default value to return if the key does not exist.
     * @return The value associated with the key, or the default value if the key does not exist.
     */
    template <typename T> T get(const char* key, const T defaultValue = T());

    /**
     * @brief Gets a sub-document from the JSON document.
     * 
     * This function retrieves a sub-document from the JSON document.
     * If the key does not exist, an empty JSON object is returned.
     * 
     * Note that the document is flushed.
     * 
     * @param key The key of the sub-document to retrieve.
     * @return The sub-document associated with the key, or an empty JSON object if the key does not exist.
     */
    Nebulite::Utility::JSON get_subdoc(const char* key);

    /**
     * @brief Provides access to the internal mutex for thread-safe operations.
     * Allowing modules to lock the JSON document.
     */
    std::lock_guard<std::recursive_mutex> lock(){return std::lock_guard<std::recursive_mutex>(mtx);};

    //------------------------------------------
    // Setter

    /**
     * @brief Sets a value in the JSON document.
     * 
     * This function sets a value of the specified type in the JSON document.
     * If the key already exists, the value is updated.
     * 
     * @tparam T The type of the value to set.
     * @param key The key of the value to set.
     * @param value The value to set.
     * 
     * @todo The current implementation of set/set_subdoc is not enough to ensure compatibility with json
     * The reason is: setting value var1 in the cache does not delete var1.var2 in the cache!
     * For each setting, a substring match is needed to fix that,
     * which would potentially break the cache usage as its slow
     * Perhaps implementing as separate helper function for now, and removing if its too slow?
     * In set(key,...) calls: -> removeFieldsFromCache(key)
     */
    template <typename T> void set(const char* key, const T& value);

    /**
     * @brief Sets a sub-document in the JSON document.
     * 
     * This function sets a sub-document in the JSON document.
     * If the key already exists, the sub-document is updated.
     * 
     * Note that both the child and parent documents' caches are flushed before setting.
     * 
     * @param key The key of the sub-document to set.
     * @param child The sub-document to set.
     */
    void set_subdoc(const char* key, Nebulite::Utility::JSON& child);

    //------------------------------------------
    // Set empty

    /**
     * @brief Sets an empty array in the JSON document.
     * 
     * This function sets an empty array in the JSON document.
     * If the key already exists, the array is updated.
     * 
     * Note that the document is flushed before setting.
     * 
     * @param key The key of the array to set.
     */
    void set_empty_array(const char* key);

    /**
     * @brief Removes a key from the JSON document.
     * 
     * This function removes a key from the JSON document.
     * If the key does not exist, no action is taken.
     * 
     * Note that the document is flushed before removing the key.
     * 
     * @param key The key to remove.
     */
    void remove_key(const char* key);

    //------------------------------------------
    // Special sets for threadsafe maths operations

    /**
     * @brief Performs an addition operation on a numeric value in the JSON document.
     */
    void set_add     (const char* key, double val);

    /**
     * @brief Performs a multiplication operation on a numeric value in the JSON document.
     */
    void set_multiply(const char* key, double val);

    /**
     * @brief Performs a concatenation operation on a string value in the JSON document.
     */
    void set_concat  (const char* key, const char* valStr);

    //------------------------------------------
    // Key Types, Sizes
    
    /**
     * @enum KeyType
     * @brief Enum representing the type of a key in the JSON document.
     */
    enum KeyType{
        document = -1,
        null = 0,
        value = 1,
        array = 2
    };

    /**
     * @brief Checks the type of a key in the JSON document.
     * 
     * This function checks the type of a key in the JSON document.
     * If the key does not exist, the type is considered null.
     * 
     * @param key The key to check.
     * @return The type of the key.
     * 
     * @todo: add function for mildFlush that does not clear cache!
     */
    KeyType memberCheck(std::string key);

    /**
     * @brief Checks the size of a key in the JSON document.
     * 
     * This function checks the size of a key in the JSON document.
     * 
     * If the key does not exist, the size is considered 0.
     * 
     * If the key represents a document, the size is considered -1.
     * 
     * @param key The key to check.
     * @return The size of the key.
     */
    uint32_t memberSize(std::string key);

    /**
     * @brief Returns the size of the cache.
     * 
     * This function returns the size of the cache.
     * 
     * @return The size of the cache.
     */
    uint32_t cacheSize(){
        std::lock_guard<std::recursive_mutex> lock(mtx); 
        return cache.size();
    };

    //------------------------------------------
    // Serializing/Deserializing

    /**
     * @brief Serializes the entire document or a portion of the document
     * 
     * @param key The key to serialize. (Optional: leave empty to serialize entire document)
     * @return The serialized JSON string.
     */
    std::string serialize(std::string key = "");

    /**
     * @brief Deserializes a JSON string or loads from a file, with optional modifications.
     * 
     * @param serial_or_link The JSON string to deserialize or the file path to load from + optional functioncall modifiers
     * 
     * Examples:
     * 
     * - `{"key": "value"}` - Deserializes a JSON string
     * 
     * - `file.json` - Loads a JSON file
     * 
     * - `file.json|set key1.key2[5] 100` - Loads a JSON file and sets a value
     * 
     * - `file.json|key1.key2[5]=100` - Legacy feature for setting a value
     * 
     * - `file.json|set-from-json key1.key2[5] otherFile.json:key` - Sets key1.key2[5] from the value of key in otherFile.json
     * 
     * See `JDM_*.hpp` files for available functioncalls.
     */
    void deserialize(std::string serial_or_link);              // if key is empty, deserializes entire doc

    //------------------------------------------
    // Fast cache system for expressions

    /**
     * @brief Provides a pointer to a double value in the cache for quick expression evaluation.
     * 
     * In order to further speed up expression evaluation, Nebulite::Utility::JSON offers direct access to cached double values.
     * This works by storing pointers to the cached values, allowing for quick retrieval and modification.
     * 
     * @param key The key of the value to retrieve.
     * @return A pointer to the cached double value. 
     * The pointer is guaranteed to be valid, even if the key is not found.
     * The value is initialized to 0.0 if the key is not found.
     */
    double* getDoublePointerOf(const std::string& key) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        if(quick_expr_double_cache.find(key) != quick_expr_double_cache.end()) {
            return quick_expr_double_cache[key];
        }
        else{
            double* newPtr = new double();
            quick_expr_double_cache[key] = newPtr;
            *newPtr = get<double>(key.c_str(), 0.0);
            return newPtr;
        }
    }

    //------------------------------------------
    // Cache/Doc manipulation

    /**
     * @brief Flushes the cache content into the main document.
     * 
     * Does **NOT** clear the cache! This just ruins speed with no benefit.
     * If any value set is ever not supported for cache, it is autoremoved
     * via `Nebulite::Utility::JSON::set()`.
     */
    void flush();

    /**
     * @brief Empties the main document and cache.
     */
    void empty();

private:
    //------------------------------------------
    // Templated helpers

    /**
     * @brief Template for supported cache storages.
     * 
     * This template is used to determine if a type is a simple value that can be cached.
     * 
     * @note Make sure to update this template and Nebulite::Utility::JSON::CacheEntry if the list of supported types changes.
     */
    template <typename T>
    struct is_simple_value : std::disjunction<
        std::is_same<T, int32_t>,
        std::is_same<T, int64_t>,
        std::is_same<T, uint32_t>,
        std::is_same<T, uint64_t>,
        std::is_same<T, double>,
        std::is_same<T, std::string>,
        std::is_same<T, bool>
    > {};

    /**
     * @brief Helper variable template for checking if a type is a simple value.
     */
    template <typename T>
    static inline constexpr bool is_simple_value_v = is_simple_value<T>::value;

    //------------------------------------------
    // Value storage

    // main doc
    rapidjson::Document doc;

    //------------------------------------------
    // Locking system

    /**
     * @brief Mutex for thread-safe access.
     */
    mutable std::recursive_mutex mtx;

    //------------------------------------------
    // Caching

    /**
     * @brief Variant type for representing simple JSON values.
     */
    
    using SimpleJSONValue = std::variant<
        int32_t, int64_t, 
        uint32_t, uint64_t,
        double, 
        std::string, 
        bool
    >;

    /**
     * @brief Cache entry for storing JSON values.
     * 
     * Holds a main value that represents the last set value, along with any derived values via type casting.
     */
    struct CacheEntry {
        SimpleJSONValue main_value;
        absl::flat_hash_map<std::type_index, SimpleJSONValue> derived_values;
    };

    /**
     * @brief Cache for storing simple JSON values.
     */
    absl::flat_hash_map<std::string, CacheEntry> cache;

    /**
     * @brief Pointer-Direct-Access-Cache for double values
     * 
     * Used in Nebulites Expression System in combination with tinyexpr for quick evaluations.
     */
    absl::flat_hash_map<std::string, double*> quick_expr_double_cache;

    //------------------------------------------
    // Helper functions for get/set:
    // they are called once it is validated that cache[key] exists

    /**
     * @brief Sets a value in the cache, clears the derived cache and sets the double pointer cache.
     * 
     * @todo For consistency with get_type, maybe changing std::string key to a CacheEntry reference?
     */
    template <typename T> void set_type(std::string key, const T& value);

    /**
     * @brief Retrieves a value from the cache or derived cache, converting if necessary.
     * 
     * - if T does not match main value, get from derived_values
     * - if derived_values does not contain value, convert from cache to T, store and return
     * 
     * @tparam T The type of the value to retrieve.
     * @param entry The cache entry to retrieve the value from.
     * @param defaultValue The default value to return if no conversion is possible.
     * @return The retrieved value or the default value.
     */
    template <typename T> T get_type(CacheEntry& entry, const T& defaultValue);

    /**
     * @brief Converts the main_value to the specified type.
     * 
     * @tparam T The type to convert to.
     * @param val The struct containing main_value and its derived_values.
     * @param defaultValue The default value to return if no conversion is possible.
     * @return The converted value or the default value.
     */
    template <typename T>
    T convert_variant(const SimpleJSONValue& val, const T& defaultValue = T());

    //------------------------------------------
    /**
     * @brief Helper functions for working with rapidjson.
     */
    class DirectAccess{
    public:
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
    };
};
} // namespace Utility
} // namespace Nebulite

template <typename T>
T Nebulite::Utility::JSON::convert_variant(const SimpleJSONValue& val, const T& defaultValue) {
    /*
    Use std::visit to handle the variant type.
    Conceptually, it works like a type-based switch statement:

    switch(type):
            case Type::INT:    return ...;
            case Type::FLOAT:  return ...;
            case Type::STRING: return ...;

    However, instead of a switch, std::visit applies the provided callable
    to the value stored in the std::variant, resolving the type at runtime.
    */
    return std::visit([&](const auto& stored) -> T 
    {
        // Removing all qualifiers (const, volatile, references, etc.)
        using StoredT = std::decay_t<decltype(stored)>;

        // [DIRECT]
        // Directly cast if types are convertible
        if constexpr (std::is_convertible_v<StoredT, T>) {
            return static_cast<T>(stored);
        }

        // [STRING] -> [BOOL]
        // Handle string to bool
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<T, bool>) {
            return stored == "true" || stored == "1";
        }

        // [STRING] -> [INT]
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<T, int>) {
            try {
                return static_cast<T>(std::stoi(stored));
            } catch (...) {
                return defaultValue;
            }
        }

        // [STRING] -> [DOUBLE]
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<T, double>) {
            try {
                return static_cast<T>(std::stod(stored));
            } catch (...) {
                return defaultValue;
            }
        }

        // [STRING] -> [UNSIGNED LONG]
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<T, unsigned long>) {
            try {
                return static_cast<T>(std::stoul(stored));
            } catch (...) {
                return defaultValue;
            }
        }

        // [STRING] -> [UNSIGNED LONG LONG]
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<T, unsigned long long>) {
            try {
                return static_cast<T>(std::stoull(stored));
            } catch (...) {
                return defaultValue;
            }
        }

        // [ARITHMETIC] -> [STRING]
        if constexpr (std::is_arithmetic_v<StoredT> && std::is_same_v<T, std::string>) {
            return std::to_string(stored);
        }

        // [STRING] -> [INTEGRAL]
        // for completion, we may wish to include all other integral types
        // Not in use anymore, rather specifying types directly!
        /*
        Handle string to integral type.

        From cppreference.com: https://en.cppreference.com/w/cpp/types/is_integral.html
        if T is the type bool, char, char8_t(since C++20), char16_t, char32_t, wchar_t, short, int, 
        long, long long, or any implementation-defined extended integer types, including any signed, 
        unsigned, and cv-qualified variants. Otherwise, value is equal to false.

        
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_integral_v<T>) {
            // Using a long long conversion
            try {
                if constexpr (std::is_signed_v<T>) {
                    return static_cast<T>(std::stoll(stored));
                } else {
                    return static_cast<T>(std::stoull(stored));
                }
            } catch (...) {
                return defaultValue;
            }
        }
        */

        //------------------------------------------
        // [FALLBACK]
        std::cerr << "[ERROR] Nebulite::Utility::JSON::convert_variant - Unsupported conversion from " 
                  << abi::__cxa_demangle(typeid(stored).name(), nullptr, nullptr, nullptr) 
                  << " to " << abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr) << ".\n"
                  << "Please add the required conversion.\n"
                  << "Fallback conversion from String to any Integral type was disabled due to potential lossy data conversion.\n"
                  << "Rather, it is recommended to add one explicit conversion path per datatype.\n"
                  << "Returning default value." << std::endl;
        return defaultValue;
    }, 
    val);
}

template <typename T>
void Nebulite::Utility::JSON::set_type(std::string key, const T& value) {
    CacheEntry& entry = cache[key];     // creates or updates entry
    entry.main_value = value;
    entry.derived_values.clear();       // reset derived types

    // Set value of its pointer cache
    double* ptr = getDoublePointerOf(key);
    if (ptr) {
        try {
            *ptr = convert_variant<double>(value, std::numeric_limits<double>::quiet_NaN());
        } catch (...) {
            std::cerr << "Failed to convert value to double for key: " << key << ". Setting to NaN" << std::endl;
            *ptr = std::numeric_limits<double>::quiet_NaN();
        }
    }
}

template <typename T>
T Nebulite::Utility::JSON::get_type(CacheEntry& entry, const T& defaultValue) {
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
T Nebulite::Utility::JSON::get(const char* key, const T defaultValue) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    if constexpr (is_simple_value_v<T> || std::is_same_v<T, const char*>) {
        // Find cached value
        auto it = cache.find(key);
        if (it != cache.end()) {
            return get_type<T>(it->second, defaultValue);
        }

        // if not found in cache, access actual doc through DirectAccess
        T tmp = Nebulite::Utility::JSON::DirectAccess::get<T>(key, defaultValue, doc);
        
        // Store in cache, return
        set(key,tmp);
        return get<T>(key, defaultValue);
    }
    // Fallback to doc
    return Nebulite::Utility::JSON::DirectAccess::get<T>(key, defaultValue, doc);
}

template <typename T>
void Nebulite::Utility::JSON::set(const char* key, const T& value) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // 1.) Check if the key is a simple value
    if constexpr (is_simple_value_v<T>) {
        set_type(key,value);
    } 
    // 2.) Make sure to allow for char arrays
    else if constexpr (std::is_same_v<T, const char*> || (std::is_array_v<T> && std::is_same_v<std::remove_extent_t<T>, char>)) {
        // Convert char arrays and const char* to std::string
        set_type(key,std::string(value));
    } 
    // 3.) Handle unsupported types
    else {
        // Remove from cache to prevent type mismatch
        cache.erase(key);
        Nebulite::Utility::JSON::DirectAccess::set<T>(key, value, doc, doc.GetAllocator());
    }
}

//------------------------------------------
// Direct access get/set

template <typename T>
T Nebulite::Utility::JSON::DirectAccess::get(const char* key, const T defaultValue, rapidjson::Value& val) {
    rapidjson::Value* keyVal = Nebulite::Utility::JSON::DirectAccess::traverse_path(key,val);
    if(keyVal == nullptr){
        // Value doesnt exist in doc, return default
        return defaultValue;
    }
    else{
        // Base case: convert currentVal to T using JSONHandler
        T tmp;
        Nebulite::Utility::JSON::DirectAccess::ConvertFromJSONValue<T>(*keyVal, tmp, defaultValue);
        return tmp;
    }
}

template <typename T>
void Nebulite::Utility::JSON::DirectAccess::set(const char* key, const T& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
    // Ensure key path exists
    rapidjson::Value* keyVal = Nebulite::Utility::JSON::DirectAccess::ensure_path(key, val, allocator);
    if (keyVal != nullptr) {
        Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<T>(value, *keyVal, allocator);
    } else {
        std::cerr << "Failed to create or access path: " << key << std::endl;
    }
}

//------------------------------------------
// All conversion variants from/to rapidjson values

//------------------------------------------
// 1.) to JSON value
//------------------------------------------

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<bool>(const bool& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)              {jsonValue.SetBool(data);}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<int>(const int& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)                {jsonValue.SetInt(data);}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<uint32_t>(const uint32_t& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)      {jsonValue.SetUint(data);}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<uint64_t>(const uint64_t& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)      {jsonValue.SetUint64(data);}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<double>(const double& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)          {jsonValue.SetDouble(data);}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<long>(const long& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)              {jsonValue.SetInt64(data);}
template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<long long>(const long long& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)    {jsonValue.SetInt64(data);}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<std::string>(const std::string& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    jsonValue.SetString(
        data.c_str(), 
        static_cast<rapidjson::SizeType>(data.length()), allocator
    );
}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<const char*>(const char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator){
    if (data) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<char*>(char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<rapidjson::Value*>(rapidjson::Value* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)         {jsonValue.CopyFrom(*data, allocator);}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<rapidjson::Document*>(rapidjson::Document* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator)   {jsonValue.CopyFrom(*data, allocator);}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<rapidjson::Document>(const rapidjson::Document& data,rapidjson::Value& jsonValue,rapidjson::Document::AllocatorType& allocator)       {jsonValue.CopyFrom(data, allocator);}

//------------------------------------------
// 2.) from JSON Value
//------------------------------------------

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, bool& result, const bool& defaultvalue){result = jsonValue.GetBool();}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, int& result, const int& defaultvalue) {
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

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, uint32_t& result, const uint32_t& defaultvalue){result = jsonValue.GetUint();}

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, uint64_t& result, const uint64_t& defaultvalue){
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

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, double& result, const double& defaultvalue){
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

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, std::string& result, const std::string& defaultvalue){
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

template <> inline void Nebulite::Utility::JSON::DirectAccess::ConvertFromJSONValue(const rapidjson::Value& jsonValue, rapidjson::Document& result, const rapidjson::Document& defaultvalue){
    result.CopyFrom(jsonValue, result.GetAllocator());
}