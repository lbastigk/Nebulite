#include "Utility/JSON.hpp"

#include "Core/GlobalSpace.hpp"
#include "Constants/ErrorTypes.hpp"
#include "DomainModule/JSDM.hpp"

Nebulite::Utility::JSON::JSON(Nebulite::Core::GlobalSpace* globalSpace)
: Nebulite::Interaction::Execution::Domain<Nebulite::Utility::JSON>("JSON", this, this, globalSpace)
{
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);
    doc.SetObject();
    Nebulite::DomainModule::JSDM_init(this);
}

Nebulite::Utility::JSON::~JSON(){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);
    cache.clear();
    doc.SetObject();
}

Nebulite::Utility::JSON::JSON(JSON&& other) noexcept
: Nebulite::Interaction::Execution::Domain<Nebulite::Utility::JSON>("JSON", this, this, other.getGlobalSpace())
{
    std::scoped_lock lockGuard(mtx, other.mtx); // Locks both, deadlock-free
    doc = std::move(other.doc);
    cache = std::move(other.cache);
}

Nebulite::Utility::JSON& Nebulite::Utility::JSON::operator=(JSON&& other) noexcept{
    if (this != &other) {
        std::scoped_lock lockGuard(mtx, other.mtx);
        doc = std::move(other.doc);
        cache = std::move(other.cache);
        for(size_t idx = 0; idx < JSON_UID_QUICKCACHE_SIZE; idx++){
            uidDoubleCache[idx] = other.uidDoubleCache[idx];
            other.uidDoubleCache[idx] = nullptr;
        }
    }
    return *this;
}

//------------------------------------------
// Private methods

// Mark all child keys as virtual
// e.g.: "parent.child1", "parent.child2.subchild", "parent[0]", etc.
void Nebulite::Utility::JSON::invalidate_child_keys(std::string const& parent_key) {
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    // Find all child keys and invalidate them
    for (auto& [key, entry] : cache) {
        if (key.starts_with(parent_key + ".") || key.starts_with(parent_key + "[")) {
            entry->state = EntryState::DELETED; // Mark as deleted
            entry->value = 0.0; // Reset value to default
            *(entry->stable_double_ptr) = 0.0;
            entry->last_double_value = 0.0;
        }
    }
}

void Nebulite::Utility::JSON::flush(){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);
    for (auto& [key, entry] : cache) {
        // If double values changed, mark dirty
        if(entry->last_double_value != *(entry->stable_double_ptr)) {
            entry->state = EntryState::DIRTY;
            entry->last_double_value = *(entry->stable_double_ptr);
            entry->value = *(entry->stable_double_ptr);
        }

        if (entry->state == EntryState::DIRTY) {
            (void)Nebulite::Utility::RjDirectAccess::set(key.c_str(), entry->value, doc, doc.GetAllocator());
            entry->state = EntryState::CLEAN;
        }
    }
}

const std::string Nebulite::Utility::JSON::reservedCharacters = "[]{}.,";


//------------------------------------------
// Domain-specific methods


Nebulite::Constants::Error Nebulite::Utility::JSON::update(){
    // Used once domain is fully set up
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);
    updateModules();
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Get methods

Nebulite::Utility::JSON Nebulite::Utility::JSON::get_subdoc(std::string const& key){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    // Flush own contents
    flush();

    rapidjson::Value* keyVal = Nebulite::Utility::RjDirectAccess::traverse_path(key.c_str(),doc);
    if(keyVal != nullptr){
        // turn keyVal to doc
        Nebulite::Utility::JSON json(getGlobalSpace());
        json.doc.CopyFrom(*keyVal,json.doc.GetAllocator());
        return json;
    }
    else{
        // Return empty doc
        return Nebulite::Utility::JSON(getGlobalSpace());
    }
}

double* Nebulite::Utility::JSON::getStableDoublePointer(std::string const& key){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    // Check cache first
    auto it = cache.find(key);
    if (it != cache.end()) {
        // If the entry is deleted, we need to update its value from the document
        if(it->second->state == EntryState::DELETED){
            *it->second->stable_double_ptr = get<double>(key, 0.0);
            it->second->last_double_value = *(it->second->stable_double_ptr);
            it->second->state = EntryState::VIRTUAL; // Now it's virtual
        }
        return it->second->stable_double_ptr;
    }

    // Not in cache
    // Instead of repeating code, we just call get to create a cache entry
    /**
     * @todo Instead of a dummy-get, we should refactor get to have a helper that creates the cache entry
     */
    volatile double dummy = get<double>(key, 0.0);
    it = cache.find(key);
    if(it != cache.end()){
        return it->second->stable_double_ptr;
    }

    // If get failed, we create a new virtual entry manually
    // And return its stable pointer
    std::unique_ptr<CacheEntry> new_entry = std::make_unique<CacheEntry>();
    new_entry->value = 0.0;
    // Pointer already created in constructor, no need to redo make_shared
    *new_entry->stable_double_ptr = 0.0;
    new_entry->last_double_value = 0.0;
    new_entry->state = EntryState::VIRTUAL;
    cache[key] = std::move(new_entry);
    return cache[key]->stable_double_ptr;
}

//------------------------------------------
// Set methods

void Nebulite::Utility::JSON::set_subdoc(const char* key, Nebulite::Utility::JSON* child){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

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
        Nebulite::Utility::Capture::cerr() << "Failed to create or access path: " << key << Nebulite::Utility::Capture::endl;
    }
}

void Nebulite::Utility::JSON::set_empty_array(const char* key){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);
    flush();
    rapidjson::Value* val = Nebulite::Utility::RjDirectAccess::ensure_path(key,doc,doc.GetAllocator());
    val->SetArray();
}

//------------------------------------------
// Serialize/Deserialize

std::string Nebulite::Utility::JSON::serialize(std::string const& key) {
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);
    flush(); // Ensure all changes are reflected in the document
    if(key.size() == 0){
        // Serialize entire doc
        return Nebulite::Utility::RjDirectAccess::serialize(doc);
    } 
    else{
        Nebulite::Utility::JSON sub = get_subdoc(key);
        return sub.serialize();
    }
}

void Nebulite::Utility::JSON::deserialize(std::string const& serial_or_link){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    // Reset document and cache
    flush();
    doc.SetObject();
    for(auto& [key, entry] : cache) {
        entry->state = EntryState::DELETED; // Mark as deleted
        entry->value = 0.0; // Reset value to default
        *(entry->stable_double_ptr) = 0.0;
        entry->last_double_value = 0.0;
    }

    //------------------------------------------
    // Split the input into tokens
    std::vector<std::string> tokens;
    if(is_json_or_jsonc(serial_or_link)){
        // Direct JSON string, no splitting
        tokens.push_back(serial_or_link);
    }
    else{
        // Split based on modifiers, indicated by '|'
        tokens = Nebulite::Utility::StringHandler::split(serial_or_link, '|');
    }

    //------------------------------------------
    // Validity check
    if (tokens.empty()) {
        // Error: No file path given
        return; // or handle error properly
    }

    //------------------------------------------
    // Load the JSON file
    Nebulite::Utility::RjDirectAccess::deserialize(doc,tokens[0], getGlobalSpace());

    //------------------------------------------
    // Delete all cache entries
    for(auto& entry : cache){
        // Mark all as deleted
        entry.second->state = EntryState::DELETED;

        // Set its double pointer value
        *entry.second->stable_double_ptr = Nebulite::Utility::RjDirectAccess::get<double>(entry.first.c_str(), 0.0, doc);
        entry.second->last_double_value = *(entry.second->stable_double_ptr);
    }

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

Nebulite::Utility::JSON::KeyType Nebulite::Utility::JSON::memberCheck(std::string const& key) {
    // 1. Check if key is empty -> represents the whole document
    if (key.empty()) {
        return KeyType::document;
    }

    // For all other cases, lock this object
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

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

uint32_t Nebulite::Utility::JSON::memberSize(std::string const& key){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    auto kt = memberCheck(key);
    if(kt == KeyType::null){
        return 0;
    }
    else if(kt == KeyType::value || kt == KeyType::document){
        return 1;
    }
    else{
        // Is array, get size
        flush(); // Ensure cache is flushed before accessing doc
        auto val = Nebulite::Utility::RjDirectAccess::traverse_path(key.c_str(),doc);
        return val->Size();
    }
}

void Nebulite::Utility::JSON::remove_key(const char* key){
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

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

// TODO: optimize by avoiding double cache lookups
// special get-function that returns the cache pointer instead of value

void Nebulite::Utility::JSON::set_add(const char* key, double val) {
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    // Get current value
    double current = get<double>(key, 0.0);
    double newValue = current + val;

    // Update double pointer value
    auto it = cache.find(key);
    if (it != cache.end()) {
        *(it->second->stable_double_ptr) = newValue;
    }
    else{
        set<double>(key, newValue);
        it = cache.find(key);
        if(it != cache.end()){
            *(it->second->stable_double_ptr) = newValue;
            it->second->last_double_value = newValue;
        }
    }
}

void Nebulite::Utility::JSON::set_multiply(const char* key, double val) {
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    // Get current value
    double current = get<double>(key, 0.0);
    double newValue = current * val;

    // Update double pointer value
    auto it = cache.find(key);
    if (it != cache.end()) {
        *(it->second->stable_double_ptr) = newValue;
    }
    else{
        set<double>(key, newValue);
        it = cache.find(key);
        if(it != cache.end()){
            *(it->second->stable_double_ptr) = newValue;
            it->second->last_double_value = newValue;
        }
    }
}

void Nebulite::Utility::JSON::set_concat(const char* key, const char* valStr) {
    std::lock_guard<std::recursive_mutex> lockGuard(mtx);

    std::string current = get<std::string>(key, "");
    set<std::string>(key, current + valStr);

    // Update double pointer value to default 0.0
    auto it = cache.find(key);
    if (it != cache.end()) {
        // Strings default to 0.0
        *(it->second->stable_double_ptr) = 0.0;
        it->second->last_double_value = 0.0;
    }
}