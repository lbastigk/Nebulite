#include "JSON.h"


Nebulite::JSON::JSON()
    : jsonTree(this)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    doc.SetObject();
}

Nebulite::JSON::~JSON(){
    std::scoped_lock lock(mtx);
    cache.clear();
    Helper::empty(doc);
}

std::string Nebulite::JSON::reservedCharacters = "[]{}.,";

void Nebulite::JSON::set_subdoc(const char* key, Nebulite::JSON& child){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    
    // Ensure key path exists
    rapidjson::Value* keyVal = ensure_path(key, doc, doc.GetAllocator());
    if (keyVal != nullptr) {
        child.flush();
        Nebulite::JSON::Helper::ConvertToJSONValue<rapidjson::Document>(child.doc, *keyVal, doc.GetAllocator());
    } else {
        std::cerr << "Failed to create or access path: " << key << std::endl;
    }
}

Nebulite::JSON Nebulite::JSON::get_subdoc(const char* key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    rapidjson::Value* keyVal = traverseKey(key,doc);
    if(keyVal != nullptr){
        // turn keyVal to doc
        Nebulite::JSON json;
        json.doc.CopyFrom(*keyVal,json.doc.GetAllocator());
        return json;
    }
    else{
        // Return empty doc
        return Nebulite::JSON(); 
    }
}

void Nebulite::JSON::set_empty_array(const char* key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    rapidjson::Value* val = ensure_path(key,doc,doc.GetAllocator());
    val->SetArray();
}

void Nebulite::JSON::remove_key(const char* key) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // Ensure key exists
    rapidjson::Value* keyVal = traverseKey(key, doc);
    if (keyVal != nullptr) {
        // Remove the key from the document
        rapidjson::Value keyName(key, doc.GetAllocator());
        doc.RemoveMember(keyName);

        // Also remove from cache if it exists
        auto it = cache.find(key);
        if (it != cache.end()) {
            cache.erase(it);
        }

        // For security, flush the cache
        flush();
    }
}

//template <typename T>
Nebulite::JSON::KeyType Nebulite::JSON::memberCheck(std::string key) {

    // 1. Check if key is empty -> represents the whole document
    if (key.empty()) {
        return KeyType::document;
    }

    // For all other cases, lock this object
    std::lock_guard<std::recursive_mutex> lock(mtx);

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

uint32_t Nebulite::JSON::memberSize(std::string key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

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
    std::lock_guard<std::recursive_mutex> lock(mtx);

    flush();
    if(key.size() == 0){
        // Serialize entire doc
        return Nebulite::JSON::Helper::serialize(doc);
    } 
    else{
        Nebulite::JSON sub = get_subdoc(key.c_str());
        return sub.serialize();
    }
}
void Nebulite::JSON::deserialize(std::string serial_or_link){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // remove cache
    for (auto it = cache.begin(); it != cache.end(); ) {
        cache.erase(it++);
    }

    //----------------------------------------------------------
    // Split the input into tokens
    std::vector<std::string> tokens = StringHandler::split(serial_or_link, '|');

    //----------------------------------------------------------
    // Validity check
    if (tokens.empty()) {
        // Error: No file path given
        return; // or handle error properly
    }

    //----------------------------------------------------------
    // Load the JSON file
    Nebulite::JSON::Helper::deserialize(doc,tokens[0]);

    //----------------------------------------------------------
    // Now apply modifications
    tokens.erase(tokens.begin()); // Remove the first token (path or serialized JSON)
    for(const auto& token : tokens) {
        if (token.empty()) continue; // Skip empty tokens

        // Legacy: Handle key=value pairs
        if (token.find('=') != std::string::npos) {
            // Handle modifier (key=value)
            auto pos = token.find('=');
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);

            // Old implementation via direct set
            //Nebulite::JSON::Helper::Set<std::string>(doc, key, value);    

            // New implementation through functioncall
            parseStr("Nebulite::JSON::Helper::deserialize set " + key + " " + value);
        }
        else{
            // Forward to FunctionTree for resolution
            parseStr("Nebulite::JSON::Helper::deserialize " + token);
        }
    }
}

void Nebulite::JSON::flush() {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    for (auto it = cache.begin(); it != cache.end(); ) {
        const std::string& key = it->first;
        const SimpleJSONValue& val = it->second.main_value;

        // Visit the variant and call set_into_doc with correct type
        std::visit([this, &key](auto&& arg) {
            fallback_set(key.c_str(), arg, doc);
        }, val);

        // Increment iterator before erasing current element
        auto to_delete = it++;
        cache.erase(to_delete);
    }
}

void Nebulite::JSON::empty(){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    Nebulite::JSON::Helper::empty(doc);
    for (auto it = cache.begin(); it != cache.end(); ) {
        cache.erase(it++);
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


rapidjson::Value* Nebulite::JSON::ensure_path(const char* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
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


//---------------------------------------------------------------------
// Static Helper Functions
rapidjson::Value Nebulite::JSON::Helper::sortRecursive(const rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
    if (value.IsObject()) {
        // Sort object keys
        std::vector<std::pair<std::string, const rapidjson::Value*>> members;
        for (auto itr = value.MemberBegin(); itr != value.MemberEnd(); ++itr) {
            members.emplace_back(itr->name.GetString(), &itr->value);
        }

        std::sort(members.begin(), members.end(),
                  [](const auto& a, const auto& b) {
                      return a.first < b.first;
                  });

        rapidjson::Value sortedObj(rapidjson::kObjectType);
        for (const auto& pair : members) {
            rapidjson::Value name(pair.first.c_str(), allocator);
            rapidjson::Value sortedVal = sortRecursive(*pair.second, allocator);
            sortedObj.AddMember(name, sortedVal, allocator);
        }
        return sortedObj;

    } else if (value.IsArray()) {
        // Preserve array order; sort internal objects if any
        rapidjson::Value newArr(rapidjson::kArrayType);
        for (const auto& v : value.GetArray()) {
            newArr.PushBack(sortRecursive(v, allocator), allocator);
        }
        return newArr;

    } else {
        // Primitive value: return a deep copy
        return rapidjson::Value(value, allocator);
    }
}
std::string Nebulite::JSON::Helper::serialize(const rapidjson::Document& doc) {
    if (!doc.IsObject() && !doc.IsArray()) {
        std::cerr << "Serialization only supports JSON objects or arrays!" << std::endl;
        return "{}";
    }

    rapidjson::Document sortedDoc;
    sortedDoc.SetObject(); // Required before Swap or adding values

    rapidjson::Value sortedRoot = sortRecursive(doc, sortedDoc.GetAllocator());
    sortedDoc.Swap(sortedRoot); // Efficiently replace contents

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    sortedDoc.Accept(writer);
    return buffer.GetString();
}


void Nebulite::JSON::Helper::deserialize(rapidjson::Document& doc, std::string serialOrLink) {

    // Check if the input is already a serialized JSON string
    if (serialOrLink.starts_with("{")) {
        rapidjson::ParseResult res = doc.Parse(serialOrLink.c_str());
    } 
    // If not, treat it as a file path
    else {
        //----------------------------------------------------------
        // Load the JSON file
        // First token is the path or serialized JSON
        std::string JSONString = FileManagement::LoadFile(serialOrLink.c_str());
        doc.Parse(JSONString.c_str());
    }
}

void Nebulite::JSON::Helper::empty(rapidjson::Document &doc) {
    doc.SetNull();
    //doc.GetAllocator().Clear();
    //doc.Swap(rapidjson::Value(rapidjson::kObjectType).Move());
}

//-------------------------------------------
// Threadsafe sets
void Nebulite::JSON::set_add(const char* key, const char* valStr) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    double current = get<double>(key, 0.0);
    double addVal = std::stod(valStr);
    set<double>(key, current + addVal);
}

void Nebulite::JSON::set_multiply(const char* key, const char* valStr) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    double current = get<double>(key, 0.0);
    double mulVal = std::stod(valStr);
    set<double>(key, current * mulVal);
}

void Nebulite::JSON::set_concat(const char* key, const char* valStr) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    std::string current = get<std::string>(key, "");
    set<std::string>(key, current + valStr);
}

FuncTree<Nebulite::ERROR_TYPE>* Nebulite::JSON::getJSONTree() {
    return &jsonTree;
}