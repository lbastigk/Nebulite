/*
Nebulite::JSON is meant as a rapidjson wrapper to streamline the trivial json requests like get and set that Nebulite does

The Plan further is to:
- use maps for faster access to the doc contents if possible
    -> if a value is stored, instead of parsing into the rapidjson doc, store into a fast map
    -> this might be helpful for certain types like ints,strings etc for easier access
- allow for more complex keys allowing for array access: 
    -> JSON.get<int>('doc.arr[i]',0) <--- 0 being default if no value present

Current efforts are to replace the old JSONHandler class that didnt fully wrap the rapidjson::Document, meaning it had to be passed into it all the time.
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
#include <string>
#include <variant>
#include <type_traits>
#include <map>
#include <unordered_map>

#include "JSONHandler.h"


/*
Values to consider for get/set and eventually the cachemap:
- int32_t
- int64_t
- uint32_t
- uint64_t
- double
- std::string
- Nebulite::JSON

Work in progress, some more values might be needed!
However, the point of this wrapper is also for rapidjson arrays to not be an issue anymore. 
Instead of manually inserting an array, their values can simply be set by passing 'key[i]' as key
*/



namespace Nebulite{
    template <typename T>
        struct is_simple_value : std::disjunction<
            std::is_same<T, int32_t>,
            std::is_same<T, int64_t>,
            std::is_same<T, uint32_t>,
            std::is_same<T, uint64_t>,
            std::is_same<T, double>,
            std::is_same<T, std::string>
        > {};
        template <typename T>
        inline constexpr bool is_simple_value_v = is_simple_value<T>::value;
    class JSON{
    public:
        JSON();

        // Get any value
        template <typename T> T get(const char* key, const T& value, const T defaultValue = T());

        // Set any value
        template <typename T> void set(const char* key, const T& value);

        // Get type of key
        enum KeyType{
            document = -1,
            null = 0,
            value = 1,
            array = 2
        };
        //template <typename T>
        KeyType memberCheck(std::string key);

        // Get size of key
        // -1 - is document
        // 0  - key doesnt exist
        // 1  - standard key
        // >1 - array
        template <typename T>
        uint32_t size(std::string key);

        // Serializing/Deserializing
        std::string serialize(std::string key = "");
        void deserialize(std::string serial_or_link);              // if key is empty, deserialize entire doc

        // flushing map content into doc
        void flush();

        // Empty document
        void empty();
    private:
        // main doc
        rapidjson::Document doc;

        // caching Simple variables
        using SimpleJSONValue = std::variant<int32_t, int64_t, uint32_t, uint64_t,double, std::string, bool>;
        std::unordered_map<std::string, SimpleJSONValue> cache;

        // Get any value
        template <typename T> T get_from_doc(const char* key, const T& value, const T defaultValue, rapidjson::Value& val);

        // Set any value
        template <typename T> void set_into_doc(const char* key, const T& value, rapidjson::Value& val);

        rapidjson::Value* traverseKey(const char* key, rapidjson::Value& val);

        rapidjson::Value* makeKey(const char* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator);
    };
}

template <typename T>
T Nebulite::JSON::get(const char* key, const T& value, const T defaultValue) {
    if constexpr (is_simple_value_v<T>) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            return std::get<T>(it->second);
        }
    }

    // fallback for both simple and complex when not in cache
    return get_from_doc<T>(key, value, defaultValue, doc);
}

template <typename T>
void Nebulite::JSON::set(const char* key, const T& value) {
    if constexpr (is_simple_value_v<T>) {
        cache[key] = value;
    } else {
        // Importantly, remove old member from cache in case:
        // | key existed in cache before, supported cachetype
        // | but now, that key is overwritten with an unsupported type!
        if(cache.find(key) != cache.end()){
            cache.erase(key);
        }
        set_into_doc<T>(key, value, doc);
    }
}

template <typename T>
T Nebulite::JSON::get_from_doc(const char* key, const T& value, const T defaultValue, rapidjson::Value& val) {
    rapidjson::Value* keyVal = traverseKey(key,val);
    //rapidjson::Value* keyVal = makeKey(key,val,doc.GetAllocator());

    if(keyVal == nullptr){
        return defaultValue;
    }
    else{
        // Base case: convert currentVal to T using JSONHandler or your own conversion
        T tmp;
        JSONHandler::ConvertFromJSONValue<T>(*keyVal, tmp, defaultValue);
        return tmp;
    }
}


template <typename T>
void Nebulite::JSON::set_into_doc(const char* key, const T& value, rapidjson::Value& val) {
    // Ensure key path exists
    rapidjson::Value* keyVal = makeKey(key, val, doc.GetAllocator());
    if (keyVal != nullptr) {
        JSONHandler::ConvertToJSONValue<T>(value, *keyVal, doc.GetAllocator());
    } else {
        std::cout << "Failed to create or access path: " << key << std::endl;
    }
}






