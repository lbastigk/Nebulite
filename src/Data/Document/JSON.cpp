#include <cfloat>

#include "Nebulite.hpp"
#include "Constants/ErrorTypes.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonScope.hpp"
#include "DomainModule/Initializer.hpp"

namespace Nebulite::Data {

std::string const JSON::reservedCharacters = "[]{}.|\":";

JSON::JSON(std::string const& name)
    : Domain(name, *this, fullScope()) {
    std::scoped_lock const lockGuard(mtx);
    DomainModule::Initializer::initJSON(this);
}

JSON::~JSON() {
    std::scoped_lock const lockGuard(mtx);
    doc.SetObject();
    cache.clear();
}

//------------------------------------------
// Allow copy/move

JSON& JSON::operator=(JSON&& other) noexcept {
    if (this != &other) {
        std::scoped_lock lockGuard(mtx, other.mtx);
        doc = std::move(other.doc);
        cache = std::move(other.cache);
    }
    return *this;
}

JSON::JSON(JSON&& other) noexcept
    : Domain("JSON", *this, fullScope()) {
    std::scoped_lock lockGuard(mtx, other.mtx); // Locks both, deadlock-free
    doc = std::move(other.doc);
    cache = std::move(other.cache);
}

//------------------------------------------
// Scope sharing

JsonScope JSON::shareScope(std::string const& prefix) {
    std::scoped_lock const lockGuard(mtx);
    return JsonScope(*this, prefix, "Shared JSON Scope from " + getName());
}

JsonScope& JSON::shareManagedScope(std::string const& prefix) {
    std::scoped_lock const lockGuard(mtx);
    auto newScope = std::make_unique<JsonScope>(*this, prefix, "Managed Shared JSON Scope from " + getName());
    managedScopes.push_back(std::move(newScope));
    return *managedScopes.back();
}

//------------------------------------------
// Private methods

JsonScope& JSON::fullScope() {
    static JsonScope fullScopeInstance(*this, "");
    return fullScopeInstance;
}

// Mark all child keys as virtual
// e.g.: "parent.child1", "parent.child2.subchild", "parent[0]", etc.
void JSON::invalidate_child_keys(std::string const& parent_key) {
    std::scoped_lock const lockGuard(mtx);

    // Find all child keys and invalidate them
    for (auto& [key, entry] : cache) {
        if (key.starts_with(parent_key + ".") || key.starts_with(parent_key + "[")) {
            entry->state = CacheEntry::EntryState::DELETED; // Mark as deleted
            entry->value = 0.0; // Reset value to default
            *entry->stable_double_ptr = 0.0;
            entry->last_double_value = 0.0;
        }
    }
}

void JSON::flush() {
    std::scoped_lock const lockGuard(mtx);
    for (auto& [key, entry] : cache) {
        // Skip malformed entries
        if (entry->state == CacheEntry::EntryState::MALFORMED) {
            continue;
        }

        // If double values changed, mark dirty
        if (std::abs(entry->last_double_value - *entry->stable_double_ptr) > DBL_EPSILON) {
            entry->state = CacheEntry::EntryState::DIRTY;
            entry->last_double_value = *entry->stable_double_ptr;
            entry->value = *entry->stable_double_ptr;
        }

        // Every dirty entry is flushed back to the document and marked clean
        if (entry->state == CacheEntry::EntryState::DIRTY) {
            (void)RjDirectAccess::set(key.c_str(), entry->value, doc, doc.GetAllocator());
            entry->state = CacheEntry::EntryState::CLEAN;
        }
    }
}


//------------------------------------------
// Domain-specific methods

Constants::Error JSON::update() {
    // Used once domain is fully set up
    std::scoped_lock const lockGuard(mtx);
    updateModules();
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Get methods

std::optional<RjDirectAccess::simpleValue> JSON::getVariant(std::string const& key) {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.contains('|')) {
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.getVariant(JsonRvalueTransformer::valueKey);
        }
        return {};
    }

    // Check cache first
    auto it = cache.find(key);

    // Checking for malformed shouldn't be necessary, but just in case
    if (it != cache.end() && it->second->state == CacheEntry::EntryState::MALFORMED) {
        Nebulite::cerr() << "Warning: Attempted to access malformed key in getVariant(): " << key << Nebulite::endl;
        Nebulite::cerr() << "This is a serious logic issue, the malformed key check should have happened already. Please report to the developers!" << Nebulite::endl;
        return {};
    }

    if (it != cache.end() && it->second->state != CacheEntry::EntryState::DELETED) {
        // Entry exists and is not deleted

        // Check its double value for change detection using an epsilon to avoid unsafe direct comparison
        if (std::fabs(*it->second->stable_double_ptr - it->second->last_double_value) > DBL_EPSILON) {
            // Value changed since last check
            // We update the actual value with the new double value
            // Then we convert the double to the requested type
            it->second->last_double_value = *it->second->stable_double_ptr;
            it->second->value = it->second->last_double_value;
            it->second->state = CacheEntry::EntryState::DIRTY; // Mark as dirty to sync back
        }
        return it->second->value;
    }

    // Check document, if not in cache
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key.c_str(), doc); val != nullptr) {
        if (it == cache.end()) {
            // Create new cache entry and insert into cache
            auto new_entry = std::make_unique<CacheEntry>();
            cache[key] = std::move(new_entry);
            it = cache.find(key);
        }

        if (it != cache.end()) {
            // Modify existing entry
            if (!RjDirectAccess::getSimpleValue(&it->second->value, val)) {
                return {};
            }

            // Mark as clean
            it->second->state = CacheEntry::EntryState::CLEAN;

            // Set stable double pointer
            *it->second->stable_double_ptr = convertVariant<double>(it->second->value, 0.0);
            it->second->last_double_value = *it->second->stable_double_ptr;
            return it->second->value;
        }
    }

    // Value could not be found, return empty optional
    return {};
}

JSON JSON::getSubDoc(std::string const& key) {
    std::scoped_lock const lockGuard(mtx);

    // Handle integrity via flushing
    // Makes sure we don't have to worry about cache and double pointers here
    // Full access to the rapidjson document after this point
    flush();

    // Check if a transformation is present
    if (key.contains('|')) {
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp;
        }
        return JSON{};
    }

    if (rapidjson::Value const* keyVal = RjDirectAccess::traversePath(key.c_str(), doc); keyVal != nullptr) {
        // turn keyVal to doc
        JSON json;
        json.doc.CopyFrom(*keyVal, json.doc.GetAllocator());
        return json;
    }
    return JSON{};
}

bool JSON::getSubDocWithTransformations(std::string const& key, JSON& outDoc) {
    auto args = Utility::StringHandler::split(key, '|');
    std::string const baseKey = args[0];
    args.erase(args.begin());

    // Using getSubDoc to properly populate the tempDoc with the rapidjson::Value
    // Slower than a manual copy that handles types, but more secure and less error-prone
    outDoc = getSubDoc(baseKey);

    // Apply each transformation in sequence
    if (!transformer.parse(args, &outDoc)) {
        return false; // if any transformation fails, return default value
    }
    return true;
}

double* JSON::getStableDoublePointer(std::string const& key) {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.find('|') != std::string::npos) {
        Nebulite::Utility::Capture::cerr() << "Transformations are not supported in getStableDoublePointer(): " << key << Nebulite::Utility::Capture::endl;
        Nebulite::Utility::Capture::cerr() << "For integrity, we will create a cache entry with the malformed key" << Nebulite::Utility::Capture::endl;
        auto new_entry = std::make_unique<CacheEntry>();
        new_entry->value = 0.0;
        *new_entry->stable_double_ptr = 0.0;
        new_entry->last_double_value = 0.0;
        new_entry->state = CacheEntry::EntryState::MALFORMED;
        cache[key] = std::move(new_entry);
        return cache[key]->stable_double_ptr;
    }

    // Check cache first
    auto it = cache.find(key);
    if (it != cache.end()) {
        // If the entry is deleted, we need to update its value from the document
        if (it->second->state == CacheEntry::EntryState::DELETED) {
            *it->second->stable_double_ptr = get<double>(key, 0.0);
            it->second->last_double_value = *it->second->stable_double_ptr;
            it->second->state = CacheEntry::EntryState::DERIVED;
        }
        return it->second->stable_double_ptr;
    }

    // Try loading from document into cache
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key.c_str(), doc); val != nullptr) {
        jsonValueToCache<double>(key, val, 0.0);
    }

    // Check cache again
    it = cache.find(key);
    if (it != cache.end()) {
        return it->second->stable_double_ptr;
    }

    // If loading from document failed, create a new derived entry
    auto new_entry = std::make_unique<CacheEntry>();
    new_entry->value = 0.0;
    *new_entry->stable_double_ptr = 0.0;
    new_entry->last_double_value = 0.0;
    new_entry->state = CacheEntry::EntryState::DERIVED;
    cache[key] = std::move(new_entry);
    return cache[key]->stable_double_ptr;
}

//------------------------------------------
// Set methods

void JSON::setVariant(std::string const& key, RjDirectAccess::simpleValue const& val) {
    std::scoped_lock const lockGuard(mtx);

    // Check if key is valid
    if (!RjDirectAccess::isValidKey(key)) {
        Nebulite::Utility::Capture::cerr() << "Invalid key: " << key << Nebulite::Utility::Capture::endl;
        return;
    }

    // Check if key contains transformations
    if (key.find('|') != std::string::npos) {
        Nebulite::Utility::Capture::cerr() << "Transformations are not supported in set(): " << key << Nebulite::Utility::Capture::endl;
        return;
    }

    // Set value in cache
    if (auto const it = cache.find(key); it != cache.end()) {
        // Existing cache value, structure validity guaranteed

        // Update the entry, mark as dirty
        it->second->value = val;
        it->second->state = CacheEntry::EntryState::DIRTY;

        // Update double pointer value
        *it->second->stable_double_ptr = convertVariant<double>(val);
        it->second->last_double_value = *it->second->stable_double_ptr;
    } else {
        // New cache value, structural validity is not guaranteed
        // so we flush contents into the rapidjson document after inserting

        // Remove any child keys to synchronize the structure
        invalidate_child_keys(key);

        // Create new entry directly in DIRTY state
        auto new_entry = std::make_unique<CacheEntry>();

        // Set entry values
        new_entry->value = val;
        // Pointer was created in constructor, no need to redo make_shared
        *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value, 0.0);
        new_entry->last_double_value = *new_entry->stable_double_ptr;
        new_entry->state = CacheEntry::EntryState::DIRTY;

        // Insert into cache
        cache[key] = std::move(new_entry);

        // Flush to RapidJSON document for structural integrity
        flush();
    }
}

void JSON::setSubDoc(char const* key, JSON& child) {
    std::scoped_lock const lockGuard(mtx);

    // Flush own contents
    flush();

    // Ensure key path exists
    // Insert child document
    if (rapidjson::Value* keyVal = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator()); keyVal != nullptr) {
        child.flush();
        RjDirectAccess::ConvertToJSONValue<rapidjson::Document>(child.doc, *keyVal, doc.GetAllocator());

        // Since we inserted an entire document, we need to invalidate its child keys:
        invalidate_child_keys(key);
    } else {
        Nebulite::cerr() << "Failed to create or access path: " << key << Nebulite::endl;
    }
}

void JSON::setEmptyArray(char const* key) {
    std::scoped_lock const lockGuard(mtx);
    flush();
    rapidjson::Value* val = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator());
    val->SetArray();
}

//------------------------------------------
// Serialize/Deserialize

std::string JSON::serialize(std::string const& key) {
    std::scoped_lock const lockGuard(mtx);
    flush(); // Ensure all changes are reflected in the document
    if (key.empty()) {
        // Serialize entire doc
        return RjDirectAccess::serialize(doc);
    }
    JSON sub = getSubDoc(key);
    return sub.serialize();
}

void JSON::deserialize(std::string const& serialOrLink) {
    std::scoped_lock const lockGuard(mtx);

    // Reset document and cache
    flush();
    doc.SetObject();
    for (auto const& entry : std::views::values(cache)) {
        entry->state = CacheEntry::EntryState::DELETED; // Mark as deleted
        entry->value = 0.0; // Reset value to default
        *entry->stable_double_ptr = 0.0;
        entry->last_double_value = 0.0;
    }

    //------------------------------------------
    // Split the input into tokens
    std::vector<std::string> tokens;
    if (isJsonOrJsonc(serialOrLink)) {
        // Direct JSON string, no splitting
        tokens.push_back(serialOrLink);
    } else {
        // Split based on transformations, indicated by '|'
        tokens = Utility::StringHandler::split(serialOrLink, '|');
    }

    //------------------------------------------
    // Validity check
    if (tokens.empty()) {
        // Error: No file path given
        return; // or handle error properly
    }

    //------------------------------------------
    // Load the JSON file
    RjDirectAccess::deserialize(doc, tokens[0]);

    //------------------------------------------
    // Delete all cache entries
    for (auto& [key, entry] : cache) {
        // Mark all as deleted
        entry->state = CacheEntry::EntryState::DELETED;

        // Set its double pointer value
        *entry->stable_double_ptr = RjDirectAccess::get<double>(key.c_str(), 0.0, doc);
        entry->last_double_value = *entry->stable_double_ptr;
    }

    //------------------------------------------
    // Now apply modifications
    tokens.erase(tokens.begin()); // Remove the first token (path or serialized JSON)
    for (auto const& token : tokens) {
        if (token.empty())
            continue; // Skip empty tokens

        // Legacy: Handle key=value pairs
        if (auto const pos = token.find('='); pos != std::string::npos) {
            // Handle transformation (key=value)
            std::string keyAndValue = token;
            if (pos != std::string::npos)
                keyAndValue[pos] = ' ';

            // New implementation through functioncall
            if (std::string const callStr = std::string(__FUNCTION__) + " set " + keyAndValue; parseStr(callStr) != Constants::ErrorTable::NONE()) {
                Nebulite::cerr() << "Failed to apply deserialize transformation: " << callStr << Nebulite::endl;
            }
        } else {
            // Forward to FunctionTree for resolution
            if (std::string const callStr = std::string(__FUNCTION__) + " " + token; parseStr(callStr) != Constants::ErrorTable::NONE()) {
                Nebulite::cerr() << "Failed to apply deserialize transformation: " << callStr << Nebulite::endl;
            }
        }
    }
    reinitModules();
}

//------------------------------------------
// Key Types, Sizes

JSON::KeyType JSON::memberType(std::string const& key) {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains('|')) {
        // Apply transformations to a temp document
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.memberType("");
        }
        return KeyType::null;
    }

    // Check cache first
    if (cache.find(key) != cache.end()) {
        // Key is cached, return its type
        return KeyType::value;
    }

    // Not directly in cache, flush before accessing the document
    flush();

    // If not cached, check rapidjson doc
    auto const val = RjDirectAccess::traversePath(key.c_str(), doc);
    if (val == nullptr) {
        return KeyType::null;
    }
    if (val->IsArray()) {
        return KeyType::array;
    }
    if (val->IsObject()) {
        return KeyType::object;
    }
    if (val->IsNumber() || val->IsString() || val->IsBool()) {
        return KeyType::value;
    }
    return KeyType::null;
}

size_t JSON::memberSize(std::string const& key) {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains('|')) {
        // Apply transformations to a temp document
        if (JSON tempDoc; getSubDocWithTransformations(key, tempDoc)) {
            return tempDoc.memberSize("");
        }
        return 0;
    }

    auto const kt = memberType(key);
    if (kt == KeyType::null) {
        return 0;
    }
    if (kt == KeyType::value || kt == KeyType::object) {
        return 1;
    }
    // Is array, get size
    flush(); // Ensure cache is flushed before accessing doc
    auto const val = RjDirectAccess::traversePath(key.c_str(), doc);
    return val->Size();
}

void JSON::removeKey(char const* key) {
    std::scoped_lock const lockGuard(mtx);

    // Ensure cache is flushed before removing key
    flush();

    // Remove member from cache
    cache.erase(key);
    invalidate_child_keys(key);

    // Find in RapidJSON document
    RjDirectAccess::removeMember(key, doc);
}

//------------------------------------------
// Threadsafe sets

// TODO: optimize by avoiding double cache lookups
// special get-function that returns the cache pointer instead of value

void JSON::set_add(std::string_view const& key, double const& val) {
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key, 0.0);
    double const newValue = current + val;

    // Update double pointer value
    if (auto it = cache.find(key); it != cache.end()) {
        *it->second->stable_double_ptr = newValue;
    } else {
        set<double>(key, newValue);
        it = cache.find(key);
        if (it != cache.end()) {
            *it->second->stable_double_ptr = newValue;
            it->second->last_double_value = newValue;
        }
    }
}

void JSON::set_multiply(std::string_view const& key, double const& val) {
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key, 0.0);
    double const newValue = current * val;

    // Update double pointer value
    if (auto it = cache.find(key); it != cache.end()) {
        *it->second->stable_double_ptr = newValue;
    } else {
        set<double>(key, newValue);
        it = cache.find(key);
        if (it != cache.end()) {
            *it->second->stable_double_ptr = newValue;
            it->second->last_double_value = newValue;
        }
    }
}

void JSON::set_concat(std::string_view const& key, std::string const& valStr) {
    std::scoped_lock const lockGuard(mtx);

    auto const current = get<std::string>(key, "");
    set<std::string>(key, current + valStr);

    // Update double pointer value to default 0.0
    if (auto const it = cache.find(key); it != cache.end()) {
        // Strings default to 0.0
        *it->second->stable_double_ptr = 0.0;
        it->second->last_double_value = 0.0;
    }
}

} // namespace Nebulite::Utility
