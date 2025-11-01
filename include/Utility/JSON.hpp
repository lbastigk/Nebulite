/**
 * @file JSON.hpp
 * @brief A wrapper around rapidjson to simplify JSON manipulation in Nebulite.
 */

#ifndef NEBULITE_UTILITY_JSON_HPP
#define NEBULITE_UTILITY_JSON_HPP

//------------------------------------------
// Includes

// Standard library
#include <mutex>
#include <cmath>
#include <typeinfo>
#include <cxxabi.h>
#include <string>
#include <variant>
#include <type_traits>
#include <thread>

// External
#include <absl/container/flat_hash_map.h>

// Nebulite
#include "Constants/ThreadSettings.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Utility/RjDirectAccess.hpp"
#include "Utility/OrderedDoublePointers.hpp"


#define JSON_UID_QUICKCACHE_SIZE 30 // First 30 keys get a quickcache entry for double pointers!

namespace Nebulite::Utility {
NEBULITE_DOMAIN(JSON){
private:
    /**
     * @enum EntryState
     * @brief Represents the state of a cached entry in the JSON document.
     * 
     * CLEAN: The entry is synchronized with the RapidJSON document and holds a real value.
     * DIRTY: The entry has been modified in the cache and needs to be flushed to the RapidJSON document.
     * VIRTUAL: The entry exists for pointer stability but does not have a real value set (defaults to 0).
     * If we access old pointers to deleted entries, we re-sync them from the document.
     * DELETED: The entry is marked for deletion but is not removed from the cache to maintain pointer stability.
     * 
     * The basic idea is: On reloading a full document, all entries become DELETED. If we access a double pointer,
     * we mark the entry as VIRTUAL, as its a resurrected entry, but its potentially not the real value due to casting.
     * A value becomes DIRTY if it was previously CLEAN and we notice a change in its double value.
     * On flushing, all DIRTY entries become CLEAN again. VIRTUAL entries remain VIRTUAL as they are not flushed.
     */
    enum class EntryState {
        CLEAN,   // Synchronized with RapidJSON document, real value
        DIRTY,   // Modified in cache, needs flushing to RapidJSON, real value  
        VIRTUAL, // Deleted entry that was re-synced, but may not be the real value due to casting
        DELETED  // Deleted entry due to deserialization, inner value is invalid
    };

    /**
     * @struct CacheEntry
     * @brief Represents a cached entry in the JSON document, including its value, state, and stable pointer for double values.
     */
    struct CacheEntry {
        // No copying or moving
        CacheEntry(CacheEntry const&) = delete;
        CacheEntry& operator=(CacheEntry const&) = delete;
        CacheEntry(CacheEntry&&) = delete;
        CacheEntry& operator=(CacheEntry&&) = delete;

        RjDirectAccess::simpleValue value = 0.0;        // Default virtual entries to 0
        double last_double_value = 0.0;                 // For change detection
        double* stable_double_ptr = new double(0.0);    // Never deleted.

        EntryState state = EntryState::DIRTY;       // Default to dirty
        
        CacheEntry(){}
    };

    /**
     * @brief Inserts a rapidjson value into the cache, converting it to the appropriate C++ type.
     * 
     * @param key The key of the value to cache.
     * @param val The rapidjson value to cache.
     * @param defaultValue The default value to use if conversion fails.
     */
    template<typename T>
    T jsonValueToCache(std::string const& key, rapidjson::Value const* val, T const& defaultValue);


    /**
     * @brief Invalidate all child keys of a given parent key.
     * 
     * For example, if parent_key is "config", it will invalidate
     * "config.option1", "config.option2.suboption", etc.
     * as well as "config[0]", "config[1].suboption", etc.
     */
    void invalidate_child_keys(std::string const& parent_key);

    /**
     * @brief The Caching system used for fast access to frequently used values.
     */
    absl::flat_hash_map<std::string, std::unique_ptr<CacheEntry>> cache;

    // Callback type for cache invalidation notifications
    using InvalidationCallback = std::function<void(std::string const& key, double* old_ptr, double* new_ptr)>;

    // Actual RapidJSON document
    rapidjson::Document doc;

    // Mutex for thread safety
    std::recursive_mutex mtx;

    /**
     * @brief Helper function to convert any type from cache into another type.
     * 
     * @param var The variant value stored in the cache.
     * @param defaultValue The default value to return if conversion fails.
     * @return The converted value of type newType, or defaultValue on failure.
     * 
     * @todo The if-branches for string "true"/"false" to int/double conversions are currently commented out,
     * as they cause issues when compiling on macOS. Reason unknown, says it can't convert to the respective type,
     * even though it is explicitly the correct type. Needs further investigation.
     * Most likely reason is that the compiler doesn't like the nested if-branch inside the constexpr if?
     * Even a redundant static_cast to the return didn't help.
     */
    template<typename newType>
    newType convertVariant(RjDirectAccess::simpleValue const& var, newType const& defaultValue = newType{});

    /**
     * @brief Flush all DIRTY entries in the cache back to the RapidJSON document.
     * 
     * This ensures that the RapidJSON document is always structurally valid
     * and up-to-date with the cached values.
     */
    void flush();

    /**
     * @brief Structure to hold multiple maps for expression references.
     * 
     * Currently, only reference "other" is used, but later on references like "parent" or "child" could be added.
     */
	struct ExpressionRef {
		Nebulite::Utility::MappedOrderedDoublePointers as_other;
	} expressionRefs[ORDERED_DOUBLE_POINTERS_MAPS];

    /**
     * @brief Super quick double cache based on unique IDs, no hash lookup.
     */
    double* uidDoubleCache[JSON_UID_QUICKCACHE_SIZE] = {nullptr};

public:
    explicit JSON(Nebulite::Core::GlobalSpace* globalSpace);

    virtual ~JSON();

    //------------------------------------------
    // Overload of assign operators

    JSON(JSON const&) = delete;
    JSON& operator=(JSON const&) = delete;
    JSON(JSON&& other) noexcept;
    JSON& operator=(JSON&& other) noexcept;

    /**
     * @brief A list of reserved characters that cannot be used in key names.
     */
    const static std::string reservedCharacters;

    //------------------------------------------
    // Domain-specific methods

    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Validity check

    /**
     * @brief Checks if a string is in JSON or JSONC format.
     * 
     * @param str The string to check.
     * @return true if the string is JSON or JSONC, false otherwise.
     */
    static bool is_json_or_jsonc(std::string const& str){
        return RjDirectAccess::is_json_or_jsonc(str);
    }

    //------------------------------------------
    // Set methods

    /**
     * @brief Sets a value in the JSON document.
     * 
     * This function sets a value of the specified type in the JSON document.
     * If the key already exists, the value is updated.
     * 
     * @tparam T The type of the value to set.
     * @param key The key of the value to set.
     * @param value The value to set.
     */
    template <typename T>
    void set(std::string const& key, T const& val);

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
    void set_subdoc(char const* key, Nebulite::Utility::JSON* child);

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
    void set_empty_array(char const* key);

    //------------------------------------------
    // Special sets for threadsafe maths operations

    /**
     * @brief Performs an addition operation on a numeric value in the JSON document.
     */
    void set_add     (char const* key, double val);

    /**
     * @brief Performs a multiplication operation on a numeric value in the JSON document.
     */
    void set_multiply(char const* key, double val);

    /**
     * @brief Performs a concatenation operation on a string value in the JSON document.
     */
    void set_concat  (char const* key, char const* valStr);

    //------------------------------------------
    // Get methods

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
    template <typename T>
    T get(std::string const& key, T const& defaultValue = T());

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
    Nebulite::Utility::JSON get_subdoc(std::string const& key);

    /**
     * @brief Provides access to the internal mutex for thread-safe operations.
     * Allowing modules to lock the JSON document.
     */
    std::lock_guard<std::recursive_mutex> lock(){return std::lock_guard<std::recursive_mutex>(mtx);}

    /**
     * @brief Gets a pointer to a to a double value pointer in the JSON document.
     */
    double* getStableDoublePointer(std::string const& key);

    /**
     * @brief Gets a pointer to a to a double value pointer in the JSON document based on a unique ID.
     * 
     * @param uid The unique ID of the key, must be smaller than JSON_UID_QUICKCACHE_SIZE !
     */
    double* get_uid_double_ptr(uint64_t uid, std::string const& key){
        if(uidDoubleCache[uid] == nullptr){
            // Need to create new entry
            uidDoubleCache[uid] = getStableDoublePointer(key);
        }
        return uidDoubleCache[uid];
    }

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
     */
    KeyType memberCheck(std::string const& key);

    /**
     * @brief Checks the size of a key in the JSON document.
     * 
     * This function checks the size of a key in the JSON document.
     * 
     * If the key does not exist, the size is considered 0.
     * 
     * If the key represents a document, the size is considered 1.
     * 
     * @param key The key to check.
     * @return The size of the key.
     */
    size_t memberSize(std::string const& key);

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
    void remove_key(char const* key);

    //------------------------------------------
    // Serialize/Deserialize

    /**
     * @brief Serializes the entire document or a portion of the document
     * 
     * @param key The key to serialize. (Optional: leave empty to serialize entire document)
     * @return The serialized JSON string.
     */
    std::string serialize(std::string const& key = "");

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
     * See `JSDM_*.hpp` files for available functioncalls.
     */
    void deserialize(std::string const& serial_or_link);

    //------------------------------------------
    // Assorted list of double pointers

	Nebulite::Utility::MappedOrderedDoublePointers* getExpressionRefsAsOther(){
        #if ORDERED_DOUBLE_POINTERS_MAPS == 1
            return &expressionRefs[0].as_other;
        #else
            // Each thread gets a unique starting position based on thread ID
            static thread_local size_t thread_offset = std::hash<std::thread::id>{}(std::this_thread::get_id());
            static thread_local size_t counter = 0;
            
            // Rotate through pool entries starting from thread's unique offset
            size_t idx = (thread_offset + counter++) % ORDERED_DOUBLE_POINTERS_MAPS;
            return &expressionRefs[idx].as_other;
        #endif
	}
};
}

template<typename T>
void Nebulite::Utility::JSON::set(std::string const& key, T const& value){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    // Check if key is valid
    if (!RjDirectAccess::isValidKey(key)){
        Nebulite::Utility::Capture::cerr() << "Invalid key: " << key << Nebulite::Utility::Capture::endl;
        return;
    }

    auto it = cache.find(key);
    if (it != cache.end()){
        // Existing cache value, structure validity guaranteed

        // Update the entry, mark as dirty
        it->second->value = value;
        it->second->state = EntryState::DIRTY;
        
        // Update double pointer value
        *(it->second->stable_double_ptr) = convertVariant<double>(value);
        it->second->last_double_value = *(it->second->stable_double_ptr);
    } else {
        // New cache value, structural validity is not guaranteed

        // Remove any child keys to synchronize the structure
        invalidate_child_keys(key);

        // Create new entry directly in DIRTY state
        std::unique_ptr<CacheEntry> new_entry = std::make_unique<CacheEntry>();

        // Set entry values
        new_entry->value = value;
        // Pointer was created in constructor, no need to redo make_shared
        *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value, 0.0);
        new_entry->last_double_value = *(new_entry->stable_double_ptr);
        new_entry->state = EntryState::DIRTY;
        
        // Insert into cache
        cache[key] = std::move(new_entry);

        // Flush to RapidJSON document for structural integrity
        flush();
    }
}

template<typename T>
T Nebulite::Utility::JSON::get(std::string const& key, T const& defaultValue){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);
    // Check cache first
    auto it = cache.find(key);
    if (it != cache.end() && it->second->state != EntryState::DELETED){
        // Entry exists and is not deleted
        
        // Check its double value for change detection using an epsilon to avoid unsafe direct comparison
        {
            const double eps = 1e-9;
            if(std::fabs(*it->second->stable_double_ptr - it->second->last_double_value) > eps){
                // Value changed since last check
                // We update the actual value with the new double value
                // Then we convert the double to the requested type
                it->second->last_double_value = *it->second->stable_double_ptr;
                it->second->value = it->second->last_double_value;
                it->second->state = EntryState::DIRTY; // Mark as dirty to sync back
                return convertVariant<T>(it->second->value, defaultValue);
            }
        }
        return convertVariant<T>(it->second->value, defaultValue);
    }

    // Check document, if not in cache
    rapidjson::Value* val = Nebulite::Utility::RjDirectAccess::traverse_path(key.c_str(), doc);
    if(val != nullptr){
        if(it != cache.end()){
            // Modify existing entry
            if(!Nebulite::Utility::RjDirectAccess::getSimpleValue(&it->second->value, val)){
                return defaultValue;
            }
            
            // Mark as clean
            it->second->state = EntryState::CLEAN;

            // Set stable double pointer
            *it->second->stable_double_ptr = convertVariant<double>(it->second->value, 0.0);
            it->second->last_double_value = *(it->second->stable_double_ptr);

            // Return converted value
            return convertVariant<T>(it->second->value, defaultValue);
        }
        else{
            // Create new cache entry
            return jsonValueToCache<T>(key, val, defaultValue);
        }
    }

    // Value could not be created, return default
    return defaultValue;
}

template<typename T>
T Nebulite::Utility::JSON::jsonValueToCache(std::string const& key, rapidjson::Value const* val, T const& defaultValue){
    // Create a new cache entry
    std::unique_ptr<CacheEntry> new_entry = std::make_unique<CacheEntry>();
    
    // Get supported types
    if(!Nebulite::Utility::RjDirectAccess::getSimpleValue(&new_entry->value, val)){
        return defaultValue;
    }

    // Mark as clean
    new_entry->state = EntryState::CLEAN;

    // Set stable double pointer
    *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value, 0.0);
    new_entry->last_double_value = *(new_entry->stable_double_ptr);

    // Insert into cache
    cache[key] = std::move(new_entry);

    // Return converted value
    return convertVariant<T>(cache[key]->value, defaultValue);
}

// Converter helper functions for convertVariant
namespace{
    inline bool stringToBool(std::string const& stored, bool defaultValue){
        // Handle numeric strings and "true"
        if(Nebulite::Utility::StringHandler::isNumber(stored)){
            try {
                return static_cast<bool>(std::stoi(stored) != 0);
            } catch (...){
                return defaultValue;
            }
        }
        return stored == "true";
    }

    inline int stringToInt(std::string const& stored, int defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<int>(std::stoi(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline double stringToDouble(std::string const& stored, double defaultValue){
        //if (stored == "true") return 1.0;
        //if (stored == "false") return 0.0;
        try {
            return static_cast<double>(std::stod(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline unsigned long stringToUnsignedLong(std::string const& stored, unsigned long defaultValue){
        //if (stored == "true") return 1.0;
        //if (stored == "false") return 0.0;
        try {
            return static_cast<unsigned long>(std::stoul(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline void convertVariantErrorMessage(std::string const& oldType, std::string const& newType){
        Nebulite::Utility::Capture::cerr() << "[ERROR] Nebulite::Utility::JSON::convert_variant - Unsupported conversion from " 
                  << oldType
                  << " to " << newType << ".\n"
                  << "Please add the required conversion.\n"
                  << "Fallback conversion from String to any Integral type was disabled due to potential lossy data conversion.\n"
                  << "Rather, it is recommended to add one explicit conversion path per datatype.\n"
                  << "Returning default value." << Nebulite::Utility::Capture::endl;
        // Exit program
        std::exit(EXIT_FAILURE);
    }
}

template<typename newType>
newType Nebulite::Utility::JSON::convertVariant(RjDirectAccess::simpleValue const& var, newType const& defaultValue){
    return std::visit([&](auto const& stored) -> newType 
    {
        // Removing all qualifiers (const, volatile, references, etc.)
        using StoredT = std::decay_t<decltype(stored)>;

        // [DIRECT]
        // Directly cast if types are convertible
        if constexpr (std::is_convertible_v<StoredT, newType>){
            return static_cast<newType>(stored);
        }

        // [STRING] -> [BOOL]
        // Handle string to bool
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, bool>){
            return stringToBool(stored, defaultValue);
        }

        // [STRING] -> [DOUBLE]
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, double>){
            return stringToDouble(stored, defaultValue);
        }
        

        // [ARITHMETIC] -> [STRING]
        if constexpr (std::is_arithmetic_v<StoredT> && std::is_same_v<newType, std::string>){
            return std::to_string(stored);
        }

        //------------------------------------------
        // [ERROR] Unsupported conversion
        std::string const oldTypeName = abi::__cxa_demangle(typeid(stored).name(), nullptr, nullptr, nullptr);
        std::string const newTypeName = abi::__cxa_demangle(typeid(newType).name(), nullptr, nullptr, nullptr);
        convertVariantErrorMessage(oldTypeName, newTypeName);
        return defaultValue;
    }, 
    var);
}

#endif // NEBULITE_UTILITY_JSON_HPP