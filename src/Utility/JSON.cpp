#include "Utility/JSON.hpp"

#include "DomainModule/JDM.hpp"
#include <vector>

Nebulite::Utility::JSON::JSON()
: Nebulite::Interaction::Execution::Domain<Nebulite::Utility::JSON>("JSON", this, this)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    doc.SetObject();
    Nebulite::DomainModule::JDM_init(this);
}

Nebulite::Utility::JSON::~JSON(){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    cache.clear();
    doc.SetObject();
}

Nebulite::Utility::JSON::JSON(JSON&& other) noexcept
: Nebulite::Interaction::Execution::Domain<Nebulite::Utility::JSON>("JSON", this, this)
{
    std::scoped_lock lock(mtx, other.mtx); // Locks both, deadlock-free
    doc = std::move(other.doc);
    cache = std::move(other.cache);
}

Nebulite::Utility::JSON& Nebulite::Utility::JSON::operator=(JSON&& other) noexcept{
    if (this != &other) {
        std::scoped_lock lock(mtx, other.mtx);
        doc = std::move(other.doc);
        cache = std::move(other.cache);
    }
    return *this;
}

//------------------------------------------
// Private methods

// Mark all child keys as virtual
// e.g.: "parent.child1", "parent.child2.subchild", "parent[0]", etc.
void Nebulite::Utility::JSON::invalidate_child_keys(const std::string& parent_key) {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    std::vector<std::string> keys_to_remove;

    // Find all child keys
    for (auto& [key, entry] : cache) {
        if (key.starts_with(parent_key + ".") || key.starts_with(parent_key + "[")) {
            entry.state = EntryState::VIRTUAL;
            entry.value = 0.0; // Reset value to default
            *(entry.stable_double_ptr) = 0.0;
            entry.last_double_value = 0.0;
        }
    }
}

void Nebulite::Utility::JSON::flush(){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    for (auto& [key, entry] : cache) {
        // If double values changed, mark dirty

        if (entry.state == EntryState::DIRTY) {
            Nebulite::Utility::RjDirectAccess::set(key.c_str(), entry.value, doc, doc.GetAllocator());
            entry.state = EntryState::CLEAN;
        }
    }
}

const std::string Nebulite::Utility::JSON::reservedCharacters = "[]{}.,";


//------------------------------------------
// Domain-specific methods


void Nebulite::Utility::JSON::update(){
    // Used once domain is fully set up
    std::lock_guard<std::recursive_mutex> lock(mtx);
}

//------------------------------------------
// Get methods

Nebulite::Utility::JSON Nebulite::Utility::JSON::get_subdoc(const char* key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // Flush own contents
    flush();

    rapidjson::Value* keyVal = Nebulite::Utility::RjDirectAccess::traverse_path(key,doc);
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

double* Nebulite::Utility::JSON::get_stable_double_ptr(const std::string& key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // Check cache first
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second.stable_double_ptr.get();
    }

    // Not in cache, create new entry
    // Instead of repeating code, we just call get to create a cache entry
    (void)get<double>(key, 0.0);
    it = cache.find(key);
    if(it != cache.end()){
        return it->second.stable_double_ptr.get();
    }

    // If get failed, we create a new virtual entry manually
    // And return its stable pointer
    CacheEntry new_entry;
    new_entry.value = 0.0;
    new_entry.stable_double_ptr = std::make_unique<double>(0.0);
    new_entry.last_double_value = 0.0;
    new_entry.state = EntryState::VIRTUAL;
    cache[key] = std::move(new_entry);
    return cache[key].stable_double_ptr.get();
}

//------------------------------------------
// Set methods

void Nebulite::Utility::JSON::set_subdoc(const char* key, Nebulite::Utility::JSON* child){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // Flush own contents
    flush();

    // Ensure key path exists
    rapidjson::Value* keyVal = Nebulite::Utility::RjDirectAccess::ensure_path(key, doc, doc.GetAllocator());

    // Insert child document
    if (keyVal != nullptr) {
        child->flush();
        Nebulite::Utility::RjDirectAccess::ConvertToJSONValue<rapidjson::Document>(child->doc, *keyVal, doc.GetAllocator());
        
        // Since we inserted an entire document, we need to invalidate its child keys:
        invalidate_child_keys(key);
    } else {
        std::cerr << "Failed to create or access path: " << key << std::endl;
    }
}

void Nebulite::Utility::JSON::set_empty_array(const char* key){
    std::lock_guard<std::recursive_mutex> lock(mtx);
    flush();
    rapidjson::Value* val = Nebulite::Utility::RjDirectAccess::ensure_path(key,doc,doc.GetAllocator());
    val->SetArray();
}

//------------------------------------------
// Serialize/Deserialize

std::string Nebulite::Utility::JSON::serialize(std::string key) {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    flush(); // Ensure all changes are reflected in the document
    if(key.size() == 0){
        // Serialize entire doc
        return Nebulite::Utility::RjDirectAccess::serialize(doc);
    } 
    else{
        Nebulite::Utility::JSON sub = get_subdoc(key.c_str());
        return sub.serialize();
    }
}

void Nebulite::Utility::JSON::deserialize(std::string serial_or_link){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // Reset document and cache
    doc.SetObject();
    for(auto& [key, entry] : cache) {
        entry.state = EntryState::VIRTUAL;
        entry.value = 0.0; // Reset value to default
        *(entry.stable_double_ptr) = 0.0;
        entry.last_double_value = 0.0;
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
    Nebulite::Utility::RjDirectAccess::deserialize(doc,tokens[0]);

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
            parseStr("Nebulite::Utility::JSON::deserialize set " + key + " " + value);
        }
        else{
            // Forward to FunctionTree for resolution
            parseStr("Nebulite::Utility::JSON::deserialize " + token);
        }
    }
}

//------------------------------------------
// Key Types, Sizes

const Nebulite::Utility::JSON::KeyType Nebulite::Utility::JSON::memberCheck(std::string key) {
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
    flush();

    // 4. If not cached, check rapidjson doc
    auto val = Nebulite::Utility::RjDirectAccess::traverse_path(key.c_str(),doc);
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

const uint32_t Nebulite::Utility::JSON::memberSize(std::string key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    auto kt = memberCheck(key);
    if(kt != KeyType::array){
        return kt;
    }
    else{
        // Is array, get size
        flush(); // Ensure cache is flushed before accessing doc
        auto val = Nebulite::Utility::RjDirectAccess::traverse_path(key.c_str(),doc);
        return val->Size();
    }
}

void Nebulite::Utility::JSON::remove_key(const char* key){
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // Ensure cache is flushed before removing key
    flush(); 

    // Remove member from cache
    cache.erase(key);
    invalidate_child_keys(key);

    // Find in RapidJSON document
    Nebulite::Utility::RjDirectAccess::remove_member(key, doc);
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