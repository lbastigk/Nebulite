#include <cfloat>

#include "Nebulite.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/JsonScope.hpp"

namespace Nebulite::Data {

JSON::JSON() = default;

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
// Lazy-initialized transformer

JsonRvalueTransformer* JSON::getTransformer() {
    thread_local JsonRvalueTransformer transformer;
    return &transformer;
}

//------------------------------------------
// Scope sharing

JsonScope& JSON::shareManagedScopeBase(std::string const& prefix) {
    std::scoped_lock const lockGuard(mtx);

    if (auto const it = managedScopeBases.find(prefix); it != managedScopeBases.end()) {
        return *it->second;
    }
    managedScopeBases[prefix] = std::make_unique<JsonScope>(*this, prefix);
    return *managedScopeBases[prefix];
}

//------------------------------------------
// Dummy sharing

JsonScope& JSON::getDummyScopeBase() {
    std::scoped_lock const lockGuard(mtx);
    if (!dummyScopeBaseInstance) {
        dummyScopeBaseInstance = std::make_unique<JsonScope>(*this, std::nullopt);
    }
    return *dummyScopeBaseInstance;
}

//------------------------------------------
// Custom copy method

/**
 * @brief Copies the entire content from another JSON document into this one.
 * @param other The other JSON document to copy from.
 */
void JSON::copyFrom(JSON const& other) {
    setSubDoc("", other);
}

//------------------------------------------
// Validity check

/**
 * @brief Checks if a string is in JSON or JSONC format.
 * @param str The string to check.
 * @return true if the string is JSON or JSONC, false otherwise.
 */
bool JSON::isJsonOrJsonc(std::string const& str) {
    return RjDirectAccess::isJsonOrJsonc(str);
}

//------------------------------------------
// Argument splitting for transformations

std::vector<std::string> JSON::splitKeyWithTransformations(std::string const& key) {
    // Split based on transformation pipe character, but respecting inner braces
    auto const braceArgs = Utility::StringHandler::splitOnSameDepth(key, '{');
    std::vector<std::string> args;

    if (!key.empty() && key.starts_with('|')) {
        // No key provided, assume root and push back an empty string
        args.push_back("");
    }

    for (auto const& arg : braceArgs) {
        if (arg.starts_with('{')) {
            // Add to last argument
            if (!args.empty()) {
                args.back() += arg;
            }
            else {
                args.push_back(arg); // Should not happen, but just in case
            }
        } else {
            // Split further on '|'
            auto splitArgs = Utility::StringHandler::split(arg, '|');
            args.insert(args.end(), splitArgs.begin(), splitArgs.end());
        }
    }
    return args;
}

//------------------------------------------
// Private methods

JsonScope& JSON::fullScopeBase() {
    std::scoped_lock const lockGuard(mtx);
    if (!fullScopeBaseInstance) {
        fullScopeBaseInstance = std::make_unique<JsonScope>(*this, "");
    }
    return *fullScopeBaseInstance;
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

std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> JSON::getVariant(std::string const& key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.contains('|')) {
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.getVariant(TransformationModule::rootKeyStr);
        }
        return std::unexpected(TRANSFORMATION_FAILURE);
    }

    // Check cache first
    auto it = cache.find(key);

    // Checking for malformed shouldn't be necessary, but just in case
    if (it != cache.end() && it->second->state == CacheEntry::EntryState::MALFORMED) {
        Error::println("Warning: Attempted to access malformed key in getVariant(): ", key);
        Error::println("This is a serious logic issue, the malformed key check should have happened already. Please report to the developers!");
        return std::unexpected(MALFORMED_KEY);
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
        if (it == cache.end() && RjDirectAccess::getSimpleValue(val).has_value()) {
            // Insert only if the value is of a supported type, otherwise complex types might be interpreted as simple values.
            // Create new cache entry and insert into cache
            auto new_entry = std::make_unique<CacheEntry>(CACHELINE, cacheline_index);
            cache[key] = std::move(new_entry);
            it = cache.find(key);
        }

        if (it != cache.end()) {
            // Modify existing entry
            if (auto const& v = RjDirectAccess::getSimpleValue(val); v.has_value()) {
                it->second->value = v.value();

                // Mark as clean
                it->second->state = CacheEntry::EntryState::CLEAN;

                // Set stable double pointer
                *it->second->stable_double_ptr = convertVariant<double>(it->second->value).value_or(0.0); // Default to 0.0 if conversion fails
                it->second->last_double_value = *it->second->stable_double_ptr;

                return v.value();
            }
        }
        if (val->IsNull()) {
            return std::unexpected(IS_NULL);
        }
        if (val->IsArray()) {
            return std::unexpected(IS_ARRAY);
        }
        if (val->IsObject()) {
            return std::unexpected(IS_OBJECT);
        }
        return std::unexpected(CONVERSION_FAILURE);
    }

    // Value could not be found, return empty optional
    return std::unexpected(IS_NULL);
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
    auto args = splitKeyWithTransformations(key);
    std::string const baseKey = args[0];
    args.erase(args.begin());

    // Using getSubDoc to properly populate the tempDoc with the rapidjson::Value
    // Slower than a manual copy that handles types, but more secure and less error-prone
    outDoc = getSubDoc(baseKey);

    // Apply each transformation in sequence
    if (!getTransformer()->parse(args, &outDoc)) {
        return false; // if any transformation fails, return default value
    }
    return true;
}

double* JSON::getStableDoublePointer(std::string const& key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.find('|') != std::string::npos) {
        throw std::runtime_error("Transformations are not supported in getStableDoublePointer()");
    }

    // Check cache first
    if (auto const it = cache.find(key); it != cache.end()) {
        // If the entry is deleted, we need to update its value from the document
        if (it->second->state == CacheEntry::EntryState::DELETED) {
            *it->second->stable_double_ptr = get<double>(key).value_or(0.0); // Default to 0.0 if retrieval fails
            it->second->last_double_value = *it->second->stable_double_ptr;
            it->second->state = CacheEntry::EntryState::DERIVED;
        }
        return it->second->stable_double_ptr;
    }

    // Try loading from document into cache
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key.c_str(), doc); val != nullptr) {
        if (jsonValueToCache<double>(key, val).has_value()) {
            // Successfully loaded into cache, return pointer
            return cache[key]->stable_double_ptr;
        }
    }

    // If loading from document failed, create a new derived entry
    auto new_entry = std::make_unique<CacheEntry>(CACHELINE, cacheline_index);
    new_entry->value = 0.0;
    *new_entry->stable_double_ptr = 0.0;
    new_entry->last_double_value = 0.0;
    new_entry->state = CacheEntry::EntryState::DERIVED;
    cache[key] = std::move(new_entry);
    return cache[key]->stable_double_ptr;
}

std::unique_lock<std::recursive_mutex> JSON::lock() const {
    return std::unique_lock(mtx);
}


//------------------------------------------
// Set methods

void JSON::setVariant(std::string const& key, RjDirectAccess::simpleValue const& val) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Check if key is valid
    if (!RjDirectAccess::isValidKey(key)) {
        Global::capture().error.println("Invalid key: ", key);
        return;
    }

    // Check if key contains transformations
    if (key.find('|') != std::string::npos) {
        Global::capture().error.println("Transformations are not supported in set(): ", key);
        return;
    }

    // Set value in cache
    if (auto const it = cache.find(key); it != cache.end()) {
        // Existing cache value, structure validity guaranteed

        // Update the entry, mark as dirty
        it->second->value = val;
        it->second->state = CacheEntry::EntryState::DIRTY;

        // Update double pointer value
        *it->second->stable_double_ptr = convertVariant<double>(val).value_or(0.0); // Default to 0.0 if conversion fails
        it->second->last_double_value = *it->second->stable_double_ptr;
    } else {
        // New cache value, structural validity is not guaranteed
        // so we flush contents into the rapidjson document after inserting

        // Remove any child keys to synchronize the structure
        invalidate_child_keys(key);

        // Create new entry directly in DIRTY state
        auto new_entry = std::make_unique<CacheEntry>(CACHELINE, cacheline_index);

        // Set entry values
        new_entry->value = val;
        // Pointer was created in constructor, no need to redo make_shared
        *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value).value_or(0.0); // Default to 0.0 if conversion fails
        new_entry->last_double_value = *new_entry->stable_double_ptr;
        new_entry->state = CacheEntry::EntryState::DIRTY;

        // Insert into cache
        cache[key] = std::move(new_entry);

        // Flush to RapidJSON document for structural integrity
        flush();
    }
}

void JSON::setSubDoc(char const* key, JSON const& child, char const* childKey) {
    std::scoped_lock const lockGuard(mtx);

    // Flush own contents
    flush();
    helperNonConstVar++; // Signal non-const operation

    child.flush();
    if (auto const childVal = RjDirectAccess::traversePath(childKey, child.doc); childVal == nullptr) {
        RjDirectAccess::removeMember(key, doc);
    }
    else {
        // If the child and this object are the same, we need to be careful with copying to avoid self-assignment issues
        if (&child == this) {
            // Copy childVal into keyVal
            rapidjson::Value childCopy;
            childCopy.CopyFrom(*childVal, doc.GetAllocator());
            rapidjson::Value* keyVal = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator());
            if (keyVal == nullptr) {
                throw std::runtime_error("Failed to create or access path: " + std::string(key));
            }
            keyVal->CopyFrom(childCopy, doc.GetAllocator());

            // Delete the child copy to free memory, since it's no longer needed
            childCopy.SetNull();
        }
        else {
            // Normal case, just copy the value from child to this document
            rapidjson::Value* keyVal = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator());
            if (keyVal == nullptr) {
                    throw std::runtime_error("Failed to create or access path: " + std::string(key));
            }
            keyVal->CopyFrom(*childVal, doc.GetAllocator());
        }
    }

    // Since we inserted an entire document, we need to invalidate its child keys:
    invalidate_child_keys(key);
}

void JSON::setEmptyArray(char const* key) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation
    flush();
    rapidjson::Value* val = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator());
    val->SetArray();
}

//------------------------------------------
// Serialize/Deserialize

std::string JSON::serialize(std::string const& key) const {
    std::scoped_lock const lockGuard(mtx);
    flush(); // Ensure all changes are reflected in the document
    if (key.empty()) {
        // Serialize entire doc
        return RjDirectAccess::serialize(doc);
    }
    JSON const sub = getSubDoc(key);
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
    if (auto const it = cache.find(key); it != cache.end()) {
        // Key is cached, return its type
        // Only consider CLEAN entries as valid
        // For all other entries, we must flush and check the document
        if (it->second->state == CacheEntry::EntryState::CLEAN) {
            return KeyType::value;
        }
    }

    // Not directly in cache, flush before accessing the document
    flush();

    // If not cached, check rapidjson doc
    auto const val = RjDirectAccess::traversePath(key.c_str(), doc);
    if (val == nullptr || val->IsNull()) {
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
    std::unreachable(); // If it's any other type, something went wrong...
}

std::string JSON::memberTypeString(std::string const& key) const {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains('|')) {
        // Apply transformations to a temp document
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.memberTypeString("");
        }
        return "null";
    }

    // Flush before accessing the document to ensure integrity
    flush();

    // If not cached, check rapidjson doc
    auto const val = RjDirectAccess::traversePath(key.c_str(), doc);
    if (val == nullptr || val->IsNull()) {
        return "null";
    }
    if (val->IsArray()) {
        return "array:" + std::to_string(val->Size());
    }
    if (val->IsObject()) {
        return "object:" + std::to_string(val->MemberCount());
    }
    if (val->IsNumber()) {
        if (val->IsInt64()) {
            return "value:int:64";
        }
        if (val->IsInt()) {
            return "value:int:32";
        }
        if (val->IsDouble()) {
            return "value:float:64";
        }
        if (val->IsFloat()) {
            return "value:float:32";
        }
        if (val->IsUint64()) {
            return "value:uint:64";
        }
        if (val->IsUint()) {
            return "value:uint:32";
        }
    }
    if (val->IsString()) {
        std::string const str = val->GetString();
        return "value:string:" + std::to_string(str.size());
    }
    if (val->IsBool()) {
        return "value:bool";
    }

    // Throw error for unsupported type
    Error::println("Unsupported type for key: '", key, "'");
    Error::println("Please add support for this type in memberTypeStr() and report to the developers if this is unexpected!");
    Error::println("Document is:");
    Error::println(RjDirectAccess::serialize(doc));
    std::abort();
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

void JSON::removeMember(char const* key) {
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

void JSON::moveMember(char const* fromKey, char const* toKey) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Ensure cache is flushed before moving key
    flush();

    // Edge case: toKey starts with fromKey, we need a temporary key to avoid deleting after moving
    if (std::string(toKey).starts_with(std::string(fromKey))) {
        // Edge case 2: if fromKey is empty
        if (std::string(fromKey).empty()) {
            setSubDoc(toKey, *this);

            // Normalize toKey by removing trailing dot if present
            auto toKeyStr = std::string(toKey);
            if (toKeyStr.ends_with('.')) {
                toKeyStr.pop_back();
            }

            // Remove all other members except toKey
            for (auto const keys = listAvailableKeys(); auto const& key : keys) {
                if (key != toKeyStr) removeMember(key);
            }

            return;
        }

        std::string const tempKey = std::string("__temp_move_") + fromKey; // Unlikely to collide with existing keys
        setSubDoc(tempKey.c_str(), *this, fromKey);
        setSubDoc(toKey, *this, tempKey.c_str());
        removeMember(tempKey.c_str());
    }
    else {
        // Direct move without temporary key
        setSubDoc(toKey, *this, fromKey);
        removeMember(fromKey);
    }
}

void JSON::copyMember(char const* fromKey, char const* toKey) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Ensure cache is flushed before copying key
    flush();

    setSubDoc(toKey, *this, fromKey);
}

std::vector<std::string> JSON::listAvailableKeys(std::string const& key) const {
    std::scoped_lock const lockGuard(mtx);

    // Flush cache before accessing document
    flush();

    // Traverse to the specified key
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key.c_str(), doc); val != nullptr) {
        return RjDirectAccess::listAvailableKeys(*val);
    }
    return {};
}

//------------------------------------------
// Threadsafe sets

// TODO: optimize by avoiding double cache lookups
// special get-function that returns the cache pointer instead of value

void JSON::set_add(std::string_view const& key, double const& val) {
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key).value_or(0.0); // Default to 0.0 if retrieval fails
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
    auto const current = get<double>(key).value_or(0.0); // Default to 0.0 if retrieval fails
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

    auto const current = get<std::string>(key).value_or(""); // Default to empty string if retrieval fails
    set<std::string>(key, current + valStr);

    // Update double pointer value to default 0.0
    if (auto const it = cache.find(key); it != cache.end()) {
        // Strings default to 0.0
        *it->second->stable_double_ptr = 0.0;
        it->second->last_double_value = 0.0;
    }
}

} // namespace Utility
