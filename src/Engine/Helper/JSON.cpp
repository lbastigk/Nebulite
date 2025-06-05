#include "JSON.h"

template <typename T>
Nebulite::JSON::KeyType Nebulite::JSON::memberCheck(std::string key) {
    // 1. Check if key is empty or represents the whole document
    if (key.empty()) {
        return KeyType::document;
    }

    // 2. Check cache first
    auto it = cache.find(key);
    if (it != cache.end()) {
        // Cached simple values are always simple values, so:
        return KeyType::value;
    }

    // 3. If not cached, check rapidjson doc
    // TODO, not as easy to implement due to nested keys and array indexing
    else{
        return KeyType::null;
    }
}

template <typename T>
uint32_t Nebulite::JSON::size(std::string key){
    if(memberCheck(key) == keyType::null){
        return 0;
    }
    else{
        // TODO...
    }
}


std::string Nebulite::JSON::serialize(std::string key){
    if(key.size() == 0){
        // Serialize entire doc
        return JSONHandler::serialize(doc);
    } 
    else{
        // Key nesting, parse through ...
    }
}
void Nebulite::JSON::deserialize(std::string serial_or_link){
    doc = JSONHandler::deserialize(serial_or_link);
}

void Nebulite::JSON::flush(){
    
}

void Nebulite::JSON::empty(){
    
}

rapidjson::Value* Nebulite::JSON::parseKey(const char* key, rapidjson::Value& val){
    // Parse key segment (up to '.' or '[' or '\0')
    uint16_t key_end = 0;
    while (key[key_end] != '\0' && key[key_end] != '.' && key[key_end] != '[') {
        key_end++;
    }

    // Extract current key part as a string_view for lookup
    std::string_view currentKey(key, key_end);

    // Parse array index if present
    uint16_t enumerator = 0;  // Use zero-based indexing internally
    uint16_t next_key_start = key_end;
    if (key[next_key_start] == '[') {
        next_key_start++; // skip '['
        enumerator = 0;
        while (key[next_key_start] >= '0' && key[next_key_start] <= '9') {
            enumerator = enumerator * 10 + (key[next_key_start] - '0');
            next_key_start++;
        }
        if (key[next_key_start] != ']') {
            // Malformed key: missing ']'
            return nullptr;
        }
        next_key_start++; // skip ']'
    }

    // After parsing array index, check for '.' separator
    bool hasMore = (key[next_key_start] == '.');

    if (hasMore) {
        next_key_start++; // skip '.'
    }

    // Check if no size
    // This happens for nested array calls like val[2][3]
    rapidjson::Value* currentVal = &val; // use val directly if no size
    if(currentKey.size() != 0){
        // Look up currentKey in val (which should be an object)
        if (!val.IsObject()) {
            return nullptr; // Can't get a key from a non-object
        }

        if (!val.HasMember(currentKey.data())) {
            return nullptr; // Key not found
        }
        currentVal = &val[currentKey.data()];
    }
    

    // If there's an array index, check array bounds and update currentVal
    if (key[key_end] == '[') {
        if (!currentVal->IsArray() || currentVal->Size() <= enumerator) {
            return nullptr; // Not an array or out of bounds
        }
        currentVal = &currentVal[enumerator];
    }

    // If there are more keys, recurse into currentVal
    if (hasMore) {
        return Nebulite::JSON::parseKey(key + next_key_start, *currentVal);
    }
    return currentVal;
}