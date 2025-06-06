#include "JSON.h"



Nebulite::JSON::JSON(){
    doc.SetObject();
}

//template <typename T>
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
    else{
        auto val = traverseKey(key.c_str(),doc);
        if(val == nullptr){
            return KeyType::null;
        }
        else{
            if(val->IsArray()){
                return KeyType::array;
            }
            if(val->IsObject()){
                return KeyType::document;
            }
            if( val->IsNumber() || val->IsString() || val->IsBool()){
                return KeyType::value;
            }
            return KeyType::null;
        }
    }
}

template <typename T>
uint32_t Nebulite::JSON::size(std::string key){
    auto kt = memberCheck(key);
    if(kt != KeyType::array){
        return kt;
    }
    else{
        // Is array, get size
        auto val = traverseKey(key.c_str(),doc);
        return val->Size();
    }
}


std::string Nebulite::JSON::serialize(std::string key){
    flush();
    if(key.size() == 0){
        // Serialize entire doc
        return JSONHandler::serialize(doc);
    } 
    else{
        // Key nesting, parse through ...
        // TODO
        std::cout << "In-Doc serialization not implemented yet" << std::endl;
    }
}
void Nebulite::JSON::deserialize(std::string serial_or_link){
    doc = JSONHandler::deserialize(serial_or_link);
}

void Nebulite::JSON::flush() {
    for (auto it = cache.begin(); it != cache.end(); /* no increment here */) {
        const std::string& key = it->first;
        const SimpleJSONValue& val = it->second;

        // Visit the variant and call set_into_doc with correct type
        std::visit([this, &key](auto&& arg) {
            set_into_doc(key.c_str(), arg, doc);
        }, val);

        // Erase the cache entry and move iterator forward safely
        it = cache.erase(it);
    }
}

void Nebulite::JSON::empty(){
    JSONHandler::empty(doc);
    for (auto it = cache.begin(); it != cache.end(); /* no increment here */) {
        // Erase the cache entry and move iterator forward safely
        it = cache.erase(it);
    }
}

rapidjson::Value* Nebulite::JSON::traverseKey(const char* key, rapidjson::Value& val){
    rapidjson::Value* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()) {
        // Find '.' or '[' as next separators
        size_t dotPos = keyView.find('.');
        size_t bracketPos = keyView.find('[');

        size_t nextSep;
        if (dotPos == std::string_view::npos && bracketPos == std::string_view::npos) {
            nextSep = keyView.size();  // No separator - last key
        } else if (dotPos == std::string_view::npos) {
            nextSep = bracketPos;
        } else if (bracketPos == std::string_view::npos) {
            nextSep = dotPos;
        } else {
            nextSep = std::min(dotPos, bracketPos);
        }

        // Extract current key part (object key)
        std::string_view keyPart = keyView.substr(0, nextSep);

        // Handle object key part if non-empty
        if (!keyPart.empty()) {
            if (!current->IsObject()) {
                return nullptr;
            }

            if (!current->HasMember(std::string(keyPart).c_str())) {
                return nullptr;
            }
            current = &(*current)[std::string(keyPart).c_str()];
        }

        // Move keyView forward past current key
        keyView.remove_prefix(nextSep);

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == '[') {
            // Find closing ']'
            size_t closeBracket = keyView.find(']');
            if (closeBracket == std::string_view::npos) {
                // Malformed key - missing ']'
                return nullptr;
            }

            // Extract index string between '[' and ']'
            std::string_view idxStr = keyView.substr(1, closeBracket - 1);
            int index = 0;
            try {
                index = std::stoi(std::string(idxStr));
            } catch (...) {
                return nullptr; // invalid number
            }

            // Make sure current is array
            if (!current->IsArray()) {
                return nullptr;
            }

            // Check if array size is high enough
            if (current->Size() <= static_cast<rapidjson::SizeType>(index)) {
                return nullptr;
            }

            current = &(*current)[index];

            // Remove processed '[index]'
            keyView.remove_prefix(closeBracket + 1);
        }

        // If next character is '.', skip it and continue
        if (!keyView.empty() && keyView[0] == '.') {
            keyView.remove_prefix(1);
        }
    }

    return current;
}


rapidjson::Value* Nebulite::JSON::makeKey(const char* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value* current = &val;
    std::string_view keyView(key);

    while (!keyView.empty()) {
        // Find '.' or '[' as next separators
        size_t dotPos = keyView.find('.');
        size_t bracketPos = keyView.find('[');

        size_t nextSep;
        if (dotPos == std::string_view::npos && bracketPos == std::string_view::npos) {
            nextSep = keyView.size();  // No separator - last key
        } else if (dotPos == std::string_view::npos) {
            nextSep = bracketPos;
        } else if (bracketPos == std::string_view::npos) {
            nextSep = dotPos;
        } else {
            nextSep = std::min(dotPos, bracketPos);
        }

        // Extract current key part (object key)
        std::string_view keyPart = keyView.substr(0, nextSep);

        // Handle object key part if non-empty
        if (!keyPart.empty()) {
            if (!current->IsObject()) {
                current->SetObject();
            }

            if (!current->HasMember(std::string(keyPart).c_str())) {
                rapidjson::Value keyVal(std::string(keyPart).c_str(), allocator);
                rapidjson::Value newObj(rapidjson::kObjectType);
                current->AddMember(keyVal, newObj, allocator);
            }
            current = &(*current)[std::string(keyPart).c_str()];
        }

        // Move keyView forward past current key
        keyView.remove_prefix(nextSep);

        // Now handle zero or more array indices if they appear next
        while (!keyView.empty() && keyView[0] == '[') {
            // Find closing ']'
            size_t closeBracket = keyView.find(']');
            if (closeBracket == std::string_view::npos) {
                // Malformed key - missing ']'
                return nullptr;
            }

            // Extract index string between '[' and ']'
            std::string_view idxStr = keyView.substr(1, closeBracket - 1);
            int index = 0;
            try {
                index = std::stoi(std::string(idxStr));
            } catch (...) {
                return nullptr; // invalid number
            }

            // Make sure current is array
            if (!current->IsArray()) {
                current->SetArray();
            }

            // Expand array if needed
            while (current->Size() <= static_cast<rapidjson::SizeType>(index)) {
                rapidjson::Value emptyObj(rapidjson::kObjectType);
                current->PushBack(emptyObj, allocator);
            }

            current = &(*current)[index];

            // Remove processed '[index]'
            keyView.remove_prefix(closeBracket + 1);
        }

        // If next character is '.', skip it and continue
        if (!keyView.empty() && keyView[0] == '.') {
            keyView.remove_prefix(1);
        }
    }

    return current;
}
