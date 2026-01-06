#include <cfloat>

#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::Data {

JSON::JSON(){}

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

JSON::JSON(JSON&& other) noexcept {
    std::scoped_lock lockGuard(mtx, other.mtx); // Locks both, deadlock-free
    doc = std::move(other.doc);
    cache = std::move(other.cache);
}

//------------------------------------------
// Scope sharing

Core::JsonScope JSON::shareScope(std::string const& prefix) {
    std::scoped_lock const lockGuard(mtx);
    return Core::JsonScope(*this, prefix, "Externally Managed JSON Scope");
}

Core::JsonScope& JSON::shareManagedScope(std::string const& prefix) {
    std::scoped_lock const lockGuard(mtx);
    auto newScope = std::make_unique<Core::JsonScope>(*this, prefix, "Managed Shared JSON Scope");
    managedScopes.push_back(std::move(newScope));
    return *managedScopes.back();
}

JsonScopeBase& JSON::shareManagedScopeBase(std::string const& prefix) {
    std::scoped_lock const lockGuard(mtx);
    auto newScope = std::make_unique<JsonScopeBase>(*this, prefix);
    managedScopeBases.push_back(std::move(newScope));
    return *managedScopeBases.back();
}

//------------------------------------------
// Private methods

Core::JsonScope& JSON::fullScope() {
    static Core::JsonScope fullScopeInstance(*this, "");
    return fullScopeInstance;
}

// Mark all child keys as virtual
// e.g.: "parent.child1", "parent.child2.subchild", "parent[0]", etc.
void JSON::invalidate_child_keys(std::string const& parent_key) const {
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

void JSON::flush() const {
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
// Get methods

std::optional<RjDirectAccess::simpleValue> JSON::getVariant(std::string const& key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.contains('|')) {
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.getVariant(JsonRvalueTransformer::valueKeyStr);
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
            auto const& v = RjDirectAccess::getSimpleValue(val);
            if (v.has_value()) {
                it->second->value = v.value();

                // Mark as clean
                it->second->state = CacheEntry::EntryState::CLEAN;

                // Set stable double pointer
                *it->second->stable_double_ptr = convertVariant<double>(it->second->value, 0.0);
                it->second->last_double_value = *it->second->stable_double_ptr;
            }
            return v;
        }
    }

    // Value could not be found, return empty optional
    return {};
}

JSON JSON::getSubDoc(std::string const& key) const {
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

bool JSON::getSubDocWithTransformations(std::string const& key, JSON& outDoc) const {
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

double* JSON::getStableDoublePointer(std::string const& key) const {
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
        (void)jsonValueToCache<double>(key, val, 0.0);
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

// NOLINTBEGIN(readability-convert-member-functions-to-static)
void JSON::setVariant(std::string const& key, RjDirectAccess::simpleValue const& val) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

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

void JSON::setSubDoc(char const* key, JSON const& child) {
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
    helperNonConstVar++; // Signal non-const operation
    flush();
    rapidjson::Value* val = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator());
    val->SetArray();
}

// NOLINTEND(readability-convert-member-functions-to-static)

//------------------------------------------
// Serialize/Deserialize

std::string JSON::serialize(std::string const& key) const {
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
    helperNonConstVar++; // Signal non-const operation

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
    // Load the JSON file
    RjDirectAccess::deserialize(doc, serialOrLink);

    //------------------------------------------
    // Delete all cache entries
    for (auto& [key, entry] : cache) {
        // Mark all as deleted
        entry->state = CacheEntry::EntryState::DELETED;

        // Set its double pointer value
        *entry->stable_double_ptr = RjDirectAccess::get<double>(key.c_str(), 0.0, doc);
        entry->last_double_value = *entry->stable_double_ptr;
    }
}

//------------------------------------------
// Key Types, Sizes

KeyType JSON::memberType(std::string const& key) const {
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

size_t JSON::memberSize(std::string const& key) const {
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
    helperNonConstVar++; // Signal non-const operation

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
