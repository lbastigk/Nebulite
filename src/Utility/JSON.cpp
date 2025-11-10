#include "Utility/JSON.hpp"

#include <cfloat>

#include "Core/GlobalSpace.hpp"
#include "Constants/ErrorTypes.hpp"
#include "DomainModule/Initializer.hpp"

namespace Nebulite::Utility {

std::string const JSON::reservedCharacters = "[]{}.,";

JSON::JSON(Core::GlobalSpace* globalSpace)
: Domain("JSON", this, this, globalSpace)
{
    std::scoped_lock const lockGuard(mtx);
    doc.SetObject();
    DomainModule::Initializer::initJSON(this);
}

JSON::~JSON(){
    std::scoped_lock const lockGuard(mtx);
    doc.SetObject();

    // Delete all cached stable pointers
    // We assume that noone will use them after JSON is destroyed

    // 1.) UID quickcache
    for (auto const& ptr : uidDoubleCache){
        // This is not necessary, probably because cache
        // itself will delete them...
        //delete ptr;
    }

    // 2.) Delete cache and its pointers
    cache.clear();
}

JSON::JSON(JSON&& other) noexcept
: Domain("JSON", this, this, other.getGlobalSpace())
{
    std::scoped_lock lockGuard(mtx, other.mtx); // Locks both, deadlock-free
    doc = std::move(other.doc);
    cache = std::move(other.cache);
}

JSON& JSON::operator=(JSON&& other) noexcept{
    if (this != &other){
        std::scoped_lock lockGuard(mtx, other.mtx);
        doc = std::move(other.doc);
        cache = std::move(other.cache);
        for(size_t idx = 0; idx < uidQuickCacheSize; idx++){
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
void JSON::invalidate_child_keys(std::string const& parent_key){
    std::scoped_lock const lockGuard(mtx);

    // Find all child keys and invalidate them
    for (auto& [key, entry] : cache){
        if (key.starts_with(parent_key + ".") || key.starts_with(parent_key + "[")){
            entry->state = EntryState::DELETED; // Mark as deleted
            entry->value = 0.0; // Reset value to default
            *entry->stable_double_ptr = 0.0;
            entry->last_double_value = 0.0;
        }
    }
}

void JSON::flush(){
    std::scoped_lock const lockGuard(mtx);
    for (auto& [key, entry] : cache){
        // If double values changed, mark dirty
        if(std::abs(entry->last_double_value - *entry->stable_double_ptr) > DBL_EPSILON){
            entry->state = EntryState::DIRTY;
            entry->last_double_value = *entry->stable_double_ptr;
            entry->value = *entry->stable_double_ptr;
        }

        if (entry->state == EntryState::DIRTY){
            (void)RjDirectAccess::set(key.c_str(), entry->value, doc, doc.GetAllocator());
            entry->state = EntryState::CLEAN;
        }
    }
}


//------------------------------------------
// Domain-specific methods

Constants::Error JSON::update(){
    // Used once domain is fully set up
    std::scoped_lock const lockGuard(mtx);
    updateModules();
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Get methods

JSON JSON::getSubDoc(std::string const& key){
    std::scoped_lock const lockGuard(mtx);
    flush();
    if(rapidjson::Value const* keyVal = RjDirectAccess::traverse_path(key.c_str(),doc); keyVal != nullptr){
        // turn keyVal to doc
        JSON json(getGlobalSpace());
        json.doc.CopyFrom(*keyVal,json.doc.GetAllocator());
        return json;
    }
    return JSON(getGlobalSpace());
}

double* JSON::getStableDoublePointer(std::string const& key){
    std::scoped_lock const lockGuard(mtx);

    // Check cache first
    auto it = cache.find(key);
    if (it != cache.end()){
        // If the entry is deleted, we need to update its value from the document
        if(it->second->state == EntryState::DELETED){
            *it->second->stable_double_ptr = get<double>(key, 0.0);
            it->second->last_double_value = *it->second->stable_double_ptr;
            it->second->state = EntryState::VIRTUAL; // Now it's virtual
        }
        return it->second->stable_double_ptr;
    }

    // Try loading from document into cache
    if(rapidjson::Value const* val = RjDirectAccess::traverse_path(key.c_str(), doc); val != nullptr){
        jsonValueToCache<double>(key, val, 0.0);
    }

    // Check cache again
    it = cache.find(key);
    if(it != cache.end()){
        return it->second->stable_double_ptr;
    }

    // If loading from document failed, create a new virtual entry
    auto new_entry = std::make_unique<CacheEntry>();
    new_entry->value = 0.0;
    *new_entry->stable_double_ptr = 0.0;
    new_entry->last_double_value = 0.0;
    new_entry->state = EntryState::VIRTUAL;
    cache[key] = std::move(new_entry);
    return cache[key]->stable_double_ptr;
}

//------------------------------------------
// Set methods

void JSON::setSubDoc(char const* key, JSON* child){
    std::scoped_lock const lockGuard(mtx);

    // Flush own contents
    flush();

    // Ensure key path exists
    // Insert child document
    if (rapidjson::Value* keyVal = RjDirectAccess::ensure_path(key, doc, doc.GetAllocator()); keyVal != nullptr){
        child->flush();
        RjDirectAccess::ConvertToJSONValue<rapidjson::Document>(child->doc, *keyVal, doc.GetAllocator());
        
        // Since we inserted an entire document, we need to invalidate its child keys:
        invalidate_child_keys(key);
    } else {
        Capture::cerr() << "Failed to create or access path: " << key << Capture::endl;
    }
}

void JSON::set_empty_array(char const* key){
    std::scoped_lock const lockGuard(mtx);
    flush();
    rapidjson::Value* val = RjDirectAccess::ensure_path(key,doc,doc.GetAllocator());
    val->SetArray();
}

//------------------------------------------
// Serialize/Deserialize

std::string JSON::serialize(std::string const& key){
    std::scoped_lock const lockGuard(mtx);
    flush(); // Ensure all changes are reflected in the document
    if(key.empty()){
        // Serialize entire doc
        return RjDirectAccess::serialize(doc);
    }
    JSON sub = getSubDoc(key);
    return sub.serialize();
}

void JSON::deserialize(std::string const& serial_or_link){
    std::scoped_lock const lockGuard(mtx);

    // Reset document and cache
    flush();
    doc.SetObject();
    for(auto const& entry : std::views::values(cache)){
        entry->state = EntryState::DELETED; // Mark as deleted
        entry->value = 0.0; // Reset value to default
        *entry->stable_double_ptr = 0.0;
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
        tokens = StringHandler::split(serial_or_link, '|');
    }

    //------------------------------------------
    // Validity check
    if (tokens.empty()){
        // Error: No file path given
        return; // or handle error properly
    }

    //------------------------------------------
    // Load the JSON file
    RjDirectAccess::deserialize(doc,tokens[0], getGlobalSpace());

    //------------------------------------------
    // Delete all cache entries
    for(auto& [key, entry] : cache){
        // Mark all as deleted
        entry->state = EntryState::DELETED;

        // Set its double pointer value
        *entry->stable_double_ptr = RjDirectAccess::get<double>(key.c_str(), 0.0, doc);
        entry->last_double_value = *entry->stable_double_ptr;
    }

    //------------------------------------------
    // Now apply modifications
    tokens.erase(tokens.begin()); // Remove the first token (path or serialized JSON)
    for(auto const& token : tokens){
        if (token.empty()) continue; // Skip empty tokens

        // Legacy: Handle key=value pairs
        if (auto const pos = token.find('='); pos != std::string::npos){
            // Handle modifier (key=value)
            std::string keyAndValue = token;
            if (pos != std::string::npos) keyAndValue[pos] = ' ';

            // New implementation through functioncall
            if (std::string const callStr = std::string(__FUNCTION__) + " set " + keyAndValue; parseStr(callStr) != Constants::ErrorTable::NONE()){
                Capture::cerr() << "Failed to apply deserialize modifier: " << callStr << Capture::endl;
            }
        }
        else{
            // Forward to FunctionTree for resolution
            if (std::string const callStr = std::string(__FUNCTION__) + " " + token; parseStr(callStr) != Constants::ErrorTable::NONE()){
                Capture::cerr() << "Failed to apply deserialize modifier: " << callStr << Capture::endl;
            }
        }
    }
}

//------------------------------------------
// Key Types, Sizes

JSON::KeyType JSON::memberCheck(std::string const& key){
    // 1. Check if key is empty -> represents the whole document
    if (key.empty()){
        return KeyType::document;
    }

    // For all other cases, lock this object
    std::scoped_lock const lockGuard(mtx);

    // 2. Check cache first
    if (cache.find(key) != cache.end()){
        // Key is cached, return its type
        return KeyType::value;
    }

    // Not directly in cache, flush before accessing the document
    flush();

    // 4. If not cached, check rapidjson doc
    auto const val = RjDirectAccess::traverse_path(key.c_str(),doc);
    if(val == nullptr){
        return KeyType::null;
    }
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

size_t JSON::memberSize(std::string const& key){
    std::scoped_lock const lockGuard(mtx);

    auto const kt = memberCheck(key);
    if(kt == KeyType::null){
        return 0;
    }
    if(kt == KeyType::value || kt == KeyType::document){
        return 1;
    }
    // Is array, get size
    flush(); // Ensure cache is flushed before accessing doc
    auto const val = RjDirectAccess::traverse_path(key.c_str(),doc);
    return val->Size();
}

void JSON::remove_key(char const* key){
    std::scoped_lock const lockGuard(mtx);

    // Ensure cache is flushed before removing key
    flush(); 

    // Remove member from cache
    cache.erase(key);
    invalidate_child_keys(key);

    // Find in RapidJSON document
    RjDirectAccess::remove_member(key, doc);
}

//------------------------------------------
// Threadsafe sets

// TODO: optimize by avoiding double cache lookups
// special get-function that returns the cache pointer instead of value

void JSON::set_add(std::string const& key, double const& val){
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key, 0.0);
    double const newValue = current + val;

    // Update double pointer value
    if (auto it = cache.find(key); it != cache.end()){
        *it->second->stable_double_ptr = newValue;
    }
    else{
        set<double>(key, newValue);
        it = cache.find(key);
        if(it != cache.end()){
            *it->second->stable_double_ptr = newValue;
            it->second->last_double_value = newValue;
        }
    }
}

void JSON::set_multiply(std::string const& key, double const& val){
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key, 0.0);
    double const newValue = current * val;

    // Update double pointer value
    if (auto it = cache.find(key); it != cache.end()){
        *it->second->stable_double_ptr = newValue;
    }
    else{
        set<double>(key, newValue);
        it = cache.find(key);
        if(it != cache.end()){
            *it->second->stable_double_ptr = newValue;
            it->second->last_double_value = newValue;
        }
    }
}

void JSON::set_concat(std::string const& key, std::string const& valStr){
    std::scoped_lock const lockGuard(mtx);

    auto const current = get<std::string>(key, "");
    set<std::string>(key, current + valStr);

    // Update double pointer value to default 0.0
    if (auto const it = cache.find(key); it != cache.end()){
        // Strings default to 0.0
        *it->second->stable_double_ptr = 0.0;
        it->second->last_double_value = 0.0;
    }
}

} // namespace Nebulite::Utility