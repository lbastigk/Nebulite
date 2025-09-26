#include "Utility/JSON.hpp"
#include "DomainModule/JDM.hpp"

/**
 * @def FLUSH_DEBUG
 * 
 * @brief If defined, debug information about cache flushing will be printed to the console.
 */
#define FLUSH_DEBUG 0

Nebulite::Utility::JSON::JSON()
: Nebulite::Interaction::Execution::Domain<Nebulite::Utility::JSON>("JSON", this, this)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    doc.SetObject();
    Nebulite::DomainModule::JDM_init(this);
}

Nebulite::Utility::JSON::JSON(JSON&& other) noexcept
: Nebulite::Interaction::Execution::Domain<Nebulite::Utility::JSON>("JSON", this, this)
{
    std::scoped_lock lock(mtx, other.mtx); // Locks both, deadlock-free
    doc = std::move(other.doc);
    cache = std::move(other.cache);
}

Nebulite::Utility::JSON::~JSON(){
    std::scoped_lock lock(mtx);
    cache.clear();
    Nebulite::Utility::JSON::DirectAccess::empty(doc);
}

const std::string Nebulite::Utility::JSON::reservedCharacters = "[]{}.,";

void Nebulite::Utility::JSON::set_subdoc(const char* key, Nebulite::Utility::JSON& child){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // Flush own contents
    #if FLUSH_DEBUG
        std::cout << "[DEBUG] Nebulite::Utility::JSON::flush() called from " << __FUNCTION__ << std::endl;
    #endif
    flush();

    // Ensure key path exists
    rapidjson::Value* keyVal = Nebulite::Utility::JSON::DirectAccess::ensure_path(key, doc, doc.GetAllocator());

    // Insert child document
    if (keyVal != nullptr) {
        child.flush();
        Nebulite::Utility::JSON::DirectAccess::ConvertToJSONValue<rapidjson::Document>(child.doc, *keyVal, doc.GetAllocator());
    } else {
        std::cerr << "Failed to create or access path: " << key << std::endl;
    }
}

Nebulite::Utility::JSON Nebulite::Utility::JSON::get_subdoc(const char* key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    #if FLUSH_DEBUG
        std::cout << "[DEBUG] Nebulite::Utility::JSON::flush() called from " << __FUNCTION__ << std::endl;
    #endif
    flush();

    rapidjson::Value* keyVal = Nebulite::Utility::JSON::DirectAccess::traverse_path(key,doc);
    if(keyVal != nullptr){
        // turn keyVal to doc
        Nebulite::Utility::JSON json;
        json.doc.CopyFrom(*keyVal,json.doc.GetAllocator());
        return json;
    }
    else{
        // Return empty doc
        return Nebulite::Utility::JSON(); 
    }
}

void Nebulite::Utility::JSON::set_empty_array(const char* key){
    #if FLUSH_DEBUG
        std::cout << "[DEBUG] Nebulite::Utility::JSON::flush() called from " << __FUNCTION__ << std::endl;
    #endif
    flush();
    std::lock_guard<std::recursive_mutex> lock(mtx);

    rapidjson::Value* val = Nebulite::Utility::JSON::DirectAccess::ensure_path(key,doc,doc.GetAllocator());
    val->SetArray();
}

void Nebulite::Utility::JSON::remove_key(const char* key) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    #if FLUSH_DEBUG
        std::cout << "[DEBUG] Nebulite::Utility::JSON::flush() called from " << __FUNCTION__ << std::endl;
    #endif
    flush(); // Ensure cache is flushed before removing key

    std::string keyStr(key);
    
    // Handle simple case: direct member of root document
    if (keyStr.find('.') == std::string::npos && keyStr.find('[') == std::string::npos) {
        if (doc.HasMember(key)) {
            doc.RemoveMember(key);
        }
        return;
    }
    
    // For complex paths, find the parent and final key/index
    
    
    // Now we find the parent value to delete the child from:
    // - parent.child
    // - parent[index]
    // - parent.child[index]
    std::string finalKey;
    int arrayIndex = -1;
    rapidjson::Value* parent = Nebulite::Utility::JSON::DirectAccess::traverse_to_parent(key, doc, finalKey, arrayIndex);
    
    // Remove the final key/index from parent
    if (parent != nullptr) {
        if (arrayIndex >= 0) {
            // Remove an array element
            if(finalKey != ""){
                parent[arrayIndex].RemoveMember(finalKey.c_str());
            }
            else if (parent->IsArray() && arrayIndex < static_cast<int>(parent->Size())) {
                parent->Erase(parent->Begin() + arrayIndex);
            }
        } else if (!finalKey.empty()) {
            // Remove object member
            if (parent->IsObject() && parent->HasMember(finalKey.c_str())) {
                parent->RemoveMember(finalKey.c_str());
            }
        }
    }
}

Nebulite::Utility::JSON::KeyType Nebulite::Utility::JSON::memberCheck(std::string key) {
    // 1. Check if key is empty -> represents the whole document
    if (key.empty()) {
        return KeyType::document;
    }

    // For all other cases, lock this object
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // 2. Check cache first
    auto it = cache.find(key);
    if (it != cache.end()) {
        // Key is cached, return its type
        return KeyType::value;
    }

    // Not directly in cache, flush before accessing the document
    #if FLUSH_DEBUG
        std::cout << "[DEBUG] Nebulite::Utility::JSON::flush() called from " << __FUNCTION__ << std::endl;
    #endif
    flush();

    // 4. If not cached, check rapidjson doc
    auto val = Nebulite::Utility::JSON::DirectAccess::traverse_path(key.c_str(),doc);
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

uint32_t Nebulite::Utility::JSON::memberSize(std::string key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    auto kt = memberCheck(key);
    if(kt != KeyType::array){
        return kt;
    }
    else{
        // Is array, get size
        #if FLUSH_DEBUG
            std::cout << "[DEBUG] Nebulite::Utility::JSON::flush() called from " << __FUNCTION__ << std::endl;
        #endif
        flush(); // Ensure cache is flushed before accessing doc
        auto val = Nebulite::Utility::JSON::DirectAccess::traverse_path(key.c_str(),doc);
        return val->Size();
    }
}

std::string Nebulite::Utility::JSON::serialize(std::string key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    #if FLUSH_DEBUG
        std::cout << "[DEBUG] Nebulite::Utility::JSON::flush() called from " << __FUNCTION__ << std::endl;
    #endif
    flush();
    if(key.size() == 0){
        // Serialize entire doc
        return Nebulite::Utility::JSON::DirectAccess::serialize(doc);
    } 
    else{
        Nebulite::Utility::JSON sub = get_subdoc(key.c_str());
        return sub.serialize();
    }
}

void Nebulite::Utility::JSON::deserialize(std::string serial_or_link){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // remove cache
    for (auto it = cache.begin(); it != cache.end(); ) {
        cache.erase(it++);
    }

    //------------------------------------------
    // Split the input into tokens
    std::vector<std::string> tokens = Nebulite::Utility::StringHandler::split(serial_or_link, '|');

    //------------------------------------------
    // Validity check
    if (tokens.empty()) {
        // Error: No file path given
        return; // or handle error properly
    }

    //------------------------------------------
    // Load the JSON file
    Nebulite::Utility::JSON::DirectAccess::deserialize(doc,tokens[0]);

    //------------------------------------------
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

            // New implementation through functioncall
            parseStr("Nebulite::Utility::JSON::DirectAccess::deserialize set " + key + " " + value);
        }
        else{
            // Forward to FunctionTree for resolution
            parseStr("Nebulite::Utility::JSON::DirectAccess::deserialize " + token);
        }
    }
}

void Nebulite::Utility::JSON::flush() {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // [DEBUG] Inform about cache flushing
    #if FLUSH_DEBUG
        std::cout << "[DEBUG] Flushing JSON cache..." << std::endl;
    #endif
    

    // Inserting all cached values into the rapidjson document
    for (auto it = cache.begin(); it != cache.end(); it++) {
        const std::string& key = it->first;
        const SimpleJSONValue& val = it->second.main_value;

        // Visit the variant and call set with correct type
        std::visit([this, &key](auto&& arg) {
            Nebulite::Utility::JSON::DirectAccess::set(key.c_str(), arg, doc, doc.GetAllocator());
        }, val);
    }
}

void Nebulite::Utility::JSON::empty(){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    Nebulite::Utility::JSON::DirectAccess::empty(doc);
    for (auto it = cache.begin(); it != cache.end(); ) {
        cache.erase(it++);
    }
}

rapidjson::Value* Nebulite::Utility::JSON::DirectAccess::traverse_path(const char* key, rapidjson::Value& val){
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

rapidjson::Value* Nebulite::Utility::JSON::DirectAccess::ensure_path(const char* key, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
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

//------------------------------------------
// Static Helper Functions
rapidjson::Value Nebulite::Utility::JSON::DirectAccess::sortRecursive(const rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) {
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

std::string Nebulite::Utility::JSON::DirectAccess::serialize(const rapidjson::Document& doc) {
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

void Nebulite::Utility::JSON::DirectAccess::deserialize(rapidjson::Document& doc, std::string serialOrLink) {

    std::string jsonString;
    
    // Check if the input is already a serialized JSON string
    if (serialOrLink.starts_with("{") || serialOrLink.starts_with("//") || serialOrLink.starts_with("/*") || serialOrLink.starts_with("\n")) {
        jsonString = serialOrLink;
    } 
    // If not, treat it as a file path
    else {
        //------------------------------------------
        // Load the JSON file
        // First token is the path or serialized JSON
        jsonString = Nebulite::Utility::FileManagement::LoadFile(serialOrLink.c_str());
    }
    
    // Strip JSONC comments before parsing
    std::string cleanJson = stripComments(jsonString);
    
    rapidjson::ParseResult res = doc.Parse(cleanJson.c_str());
    if (!res) {
        std::cerr << "JSON Parse Error at offset " << res.Offset() << std::endl;
    }
}

void Nebulite::Utility::JSON::DirectAccess::empty(rapidjson::Document &doc) {
    doc.SetNull();
    //doc.GetAllocator().Clear();
    //doc.Swap(rapidjson::Value(rapidjson::kObjectType).Move());
}

std::string Nebulite::Utility::JSON::DirectAccess::stripComments(const std::string& jsonc) {
    std::string result;
    result.reserve(jsonc.size());
    
    bool inString = false;
    bool inSingleComment = false;
    bool inMultiComment = false;
    bool escaped = false;
    
    for (size_t i = 0; i < jsonc.size(); ++i) {
        char c = jsonc[i];
        char next = (i + 1 < jsonc.size()) ? jsonc[i + 1] : '\0';
        
        if (inSingleComment) {
            if (c == '\n') {
                inSingleComment = false;
                result += c; // Preserve newline for line counting
            }
            continue;
        }
        
        if (inMultiComment) {
            if (c == '*' && next == '/') {
                inMultiComment = false;
                ++i; // Skip the '/'
            }
            continue;
        }
        
        if (inString) {
            result += c;
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                inString = false;
            }
            continue;
        }
        
        // Not in string or comment
        if (c == '"') {
            inString = true;
            result += c;
        } else if (c == '/' && next == '/') {
            inSingleComment = true;
            ++i; // Skip the second '/'
        } else if (c == '/' && next == '*') {
            inMultiComment = true;
            ++i; // Skip the '*'
        } else {
            result += c;
        }
    }
    
    return result;
}

rapidjson::Value* Nebulite::Utility::JSON::DirectAccess::traverse_to_parent(const char* fullKey, rapidjson::Value& root, std::string& finalKey, int& arrayIndex) {
    std::string keyStr(fullKey);
    size_t lastDot = keyStr.find_last_of('.');
    size_t lastBracket = keyStr.find_last_of('[');

    rapidjson::Value* parent = nullptr;
    if (lastBracket != std::string::npos && (lastDot == std::string::npos || lastBracket > lastDot)) {
        // Last access is array index: var.subvar[2] or var[2]
        size_t openBracket = keyStr.find_last_of('[');
        size_t closeBracket = keyStr.find_last_of(']');
        
        if (openBracket != std::string::npos && closeBracket != std::string::npos && closeBracket > openBracket) {
            std::string parentPath = keyStr.substr(0, openBracket);
            std::string indexStr = keyStr.substr(openBracket + 1, closeBracket - openBracket - 1);
            
            try {
                arrayIndex = std::stoi(indexStr);
                if (parentPath.empty()) {
                    parent = &root;
                } else {
                    parent = Nebulite::Utility::JSON::DirectAccess::traverse_path(parentPath.c_str(), root);
                }
            } catch (...) {
                return nullptr; // Invalid index
            }
        }
    } else if (lastDot != std::string::npos) {
        // Last access is object member: var.subvar.finalkey
        std::string parentPath = keyStr.substr(0, lastDot);
        finalKey = keyStr.substr(lastDot + 1);
        parent = Nebulite::Utility::JSON::DirectAccess::traverse_path(parentPath.c_str(), root);
    }
    return parent;
}

//------------------------------------------
// Threadsafe sets
void Nebulite::Utility::JSON::set_add(const char* key, double val) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    double current = get<double>(key, 0.0);
    set<double>(key, current + val);
}

void Nebulite::Utility::JSON::set_multiply(const char* key, double val) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    double current = get<double>(key, 0.0);
    set<double>(key, current * val);
}

void Nebulite::Utility::JSON::set_concat(const char* key, const char* valStr) {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    std::string current = get<std::string>(key, "");
    set<std::string>(key, current + valStr);
}