//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint> // NOLINT
#include <expected>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

// External
#include <rapidjson/document.h>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/JsonRvalueTransformer.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
// Try to catch potential caching issues early

static_assert(
    sizeof(Nebulite::Data::JSON) - sizeof(std::recursive_mutex) == 200,
    "JSON size has changed, please review the move assignment operator for potential cache invalidation issues."
);



//------------------------------------------
namespace Nebulite::Data {

//------------------------------------------
// Prefix: parent finder

std::string_view JSON::findParentKey(std::string_view const key) {
    if (key.empty()) {
        return key.substr(0, 0); // Return empty string view
    }
    std::size_t const lastPos = key.find_last_of(".]");
    if (lastPos == std::string_view::npos || lastPos == 0) {
        return key.substr(0, 0); // Return empty string view
    }
    if (lastPos == key.length() - 1) {
        return findParentKey(key.substr(0, key.length() > 1 ? key.length() - 1 : 0));
    }
    if (key[lastPos] == ']') {
        return key.substr(0, lastPos+1);
    }
    return key.substr(0, lastPos);
}

//------------------------------------------
// Construct / Destruct

JSON::JSON() {
    cacheLine = std::make_unique<CacheLine>();
}

JSON::~JSON() {
    std::scoped_lock const lockGuard(mtx);
    doc.SetObject();
    cache.clear();
}

//------------------------------------------
// Allow move

JSON& JSON::operator=(JSON&& other) noexcept {
    if (this != &other) {
        std::scoped_lock const lockGuard(mtx, other.mtx);
        doc = std::move(other.doc);
        cache = std::move(other.cache);
        cacheLine = std::move(other.cacheLine);
    }
    return *this;
}

JSON::JSON(JSON&& other) noexcept : cacheLine(std::move(other.cacheLine)), cache(std::move(other.cache)), doc(std::move(other.doc)) {
    std::scoped_lock const lockGuard(mtx, other.mtx); // Locks both, deadlock-free
}

//------------------------------------------
// Scope sharing

JsonScope& JSON::shareManagedScope(std::string_view const prefix) {
    std::scoped_lock const lockGuard(mtx);

    if (auto const it = managedScopes.find(prefix); it != managedScopes.end()) {
        return *it->second;
    }
    managedScopes[prefix] = std::make_unique<JsonScope>(*this, std::string(prefix));
    return *managedScopes[prefix];
}

//------------------------------------------
// Dummy sharing

JsonScope& JSON::getDummyScope() {
    std::scoped_lock const lockGuard(mtx);
    if (!dummyScopeInstance) {
        dummyScopeInstance = std::make_unique<JsonScope>(*this, std::nullopt);
    }
    return *dummyScopeInstance;
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
bool JSON::isJsonOrJsonc(std::string_view const str) {
    return RjDirectAccess::isJsonOrJsonc(str);
}

//------------------------------------------
// Argument splitting for transformations

std::vector<std::string_view> JSON::splitKeyWithTransformations(std::string_view const key) {
    auto result = Utility::StringHandler::splitOnSameDepth(key, SpecialCharacter::transformationPipe);
    for (auto& arg : result) {
        if (arg.starts_with(SpecialCharacter::transformationPipe)) {
            arg = arg.substr(1);
        }
    }
    std::erase_if(result, [](std::string_view const arg) { return arg.empty(); });
    if (!key.empty() && key.starts_with(SpecialCharacter::transformationPipe)) {
        // No key provided, assume root and push back an empty string
        result.insert(result.begin(), "");
    }
    return result;
}

//------------------------------------------
// Private methods

JsonScope& JSON::fullScope() {
    std::scoped_lock const lockGuard(mtx);
    if (!fullScopeInstance) {
        fullScopeInstance = std::make_unique<JsonScope>(*this, "");
    }
    return *fullScopeInstance;
}

void JSON::synchronizeChildren(std::string_view const parentKey) const {
    std::scoped_lock const lockGuard(mtx);

    // Find all child keys and invalidate them
    for (auto& [key, entry] : cache) {
        bool const base = key.starts_with(parentKey) && key.length() > parentKey.length();
        bool const startsWithParentKeyPlusDot = base && key[parentKey.length()] == SpecialCharacter::dot;
        bool const startsWithParentKeyPlusArr = base && key[parentKey.length()] == SpecialCharacter::arrayOpen;
        if (bool const parentKeyIsRoot = parentKey.empty(); parentKeyIsRoot || startsWithParentKeyPlusDot || startsWithParentKeyPlusArr) {
            if (auto const variant = RjDirectAccess::getSimpleValue(key, doc); variant.has_value()) {
                entry->state = CacheEntry::EntryState::CLEAN;
                entry->value = variant.value();
                *entry->stable_double_ptr = convertVariant<double>(entry->value).value_or(standardNumericValue); // Default to 0 if conversion fails
                entry->last_double_value = *entry->stable_double_ptr;
            }
            else {
                deleteCacheEntry(entry);
            }
        }
    }
}

void JSON::flush(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    auto const parent = findParentKey(key);

    for (auto& [entryKey, entry] : cache) {
        if (!entryKey.starts_with(parent)) continue;

        // Skip malformed entries
        if (entry->state == CacheEntry::EntryState::MALFORMED) {
            continue;
        }

        // If double values changed, mark dirty
        if (!Math::isEqualAllowNan(entry->last_double_value, *entry->stable_double_ptr)) {
            entry->state = CacheEntry::EntryState::DIRTY;
            entry->last_double_value = *entry->stable_double_ptr;
            entry->value = *entry->stable_double_ptr;
        }

        // Every dirty entry is flushed back to the document and marked clean
        if (entry->state == CacheEntry::EntryState::DIRTY) {
            (void)RjDirectAccess::set(entryKey.c_str(), entry->value, doc, doc.GetAllocator());
            entry->state = CacheEntry::EntryState::CLEAN;
        }
    }
}

//------------------------------------------
// Get methods

std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> JSON::getVariant(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.contains(SpecialCharacter::transformationPipe)) {
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.getVariant(Module::Base::TransformationModule::rootKeyStr);
        }
        return std::unexpected(SimpleValueRetrievalError::TRANSFORMATION_FAILURE);
    }

    // Check cache first
    if (!std::ranges::any_of(cache, [&key](auto const& pair) {
        auto& [cachedKey, entry] = pair;
        return cachedKey.starts_with(key)
            && cachedKey != key
            && entry->state != CacheEntry::EntryState::DELETED
            && Math::isEqualAllowNan(*entry->stable_double_ptr, entry->last_double_value);
    })) {
        // Checking for malformed shouldn't be necessary, but just in case
        auto const it = cache.find(key);
        if (it != cache.end() && it->second->state == CacheEntry::EntryState::MALFORMED) {
            Global::capture().error.println("Warning: Attempted to access malformed key in getVariant(): ", key);
            Global::capture().error.println("This is a serious logic issue, the malformed key check should have happened already. Please report to the developers!");
            return std::unexpected(SimpleValueRetrievalError::MALFORMED_KEY);
        }

        if (it != cache.end() && it->second->state != CacheEntry::EntryState::DELETED) {
            // Entry exists and is not deleted

            // Check its double value for change detection using an epsilon to avoid unsafe direct comparison
            if (!Math::isEqualAllowNan(*it->second->stable_double_ptr, it->second->last_double_value)) {
                // Value changed since last check
                // We update the actual value with the new double value
                // Then we convert the double to the requested type
                it->second->last_double_value = *it->second->stable_double_ptr;
                it->second->value = it->second->last_double_value;
                it->second->state = CacheEntry::EntryState::DIRTY; // Mark as dirty to sync back
            }
            return it->second->value;
        }
    }

    // Check document, if not in cache
    flush(key);
    return getSimpleValueFromDocument(key);
}

JSON JSON::getSubDoc(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Handle integrity via flushing
    // Makes sure we don't have to worry about cache and double pointers here
    // Full access to the rapidjson document after this point
    flush(key);

    // Check if a transformation is present
    if (key.contains(SpecialCharacter::transformationPipe)) {
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp;
        }
        return JSON{};
    }

    if (rapidjson::Value const* keyVal = RjDirectAccess::traversePath(key, doc); keyVal != nullptr) {
        // turn keyVal to doc
        JSON json;
        json.doc.CopyFrom(*keyVal, json.doc.GetAllocator());
        return json;
    }
    return JSON{};
}

bool JSON::getSubDocWithTransformations(std::string_view const key, JSON& outDoc) const {
    auto args = splitKeyWithTransformations(key);
    {
        auto const& baseKey = args[0];

        // Using getSubDoc to properly populate the tempDoc with the rapidjson::Value
        // Slower than a manual copy that handles types, but more secure and less error-prone
        outDoc = getSubDoc(baseKey);
    }

    // Apply each transformation in sequence
    args.erase(args.begin());
    return JsonRvalueTransformer::instance().parse(args, outDoc);
}

double* JSON::getStableDoublePointer(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.contains(SpecialCharacter::transformationPipe)) {
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
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key, doc); val != nullptr) {
        if (jsonValueToCache<double>(key, val).has_value()) {
            // Successfully loaded into cache, return pointer
            return cache[key]->stable_double_ptr;
        }
    }

    // If loading from document failed, create a new derived entry
    auto new_entry = std::make_unique<CacheEntry>(*cacheLine, cacheline_index);
    new_entry->value = standardNumericValue;
    *new_entry->stable_double_ptr = standardNumericValue;
    new_entry->last_double_value = standardNumericValue;
    new_entry->state = CacheEntry::EntryState::DERIVED;
    auto* const ptr = new_entry->stable_double_ptr;
    cache[key] = std::move(new_entry);
    return ptr;
}

std::unique_lock<std::recursive_mutex> JSON::lock() const {
    return std::unique_lock(mtx);
}


//------------------------------------------
// Set methods

void JSON::setVariant(std::string_view const key, RjDirectAccess::simpleValue const& val) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Check if key is valid
    if (!RjDirectAccess::isValidKey(key)) {
        Global::capture().error.println("Invalid key: ", key);
        return;
    }

    // Check if key contains transformations
    if (key.contains(SpecialCharacter::transformationPipe)) {
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
        *it->second->stable_double_ptr = convertVariant<double>(val).value_or(standardNumericValue); // Default to 0 if conversion fails
        it->second->last_double_value = *it->second->stable_double_ptr;
    } else {
        // New cache value, structural validity is not guaranteed
        // so we flush contents into the rapidjson document after inserting

        // Synchronize structure
        synchronizeChildren(key);

        // Create new entry directly in DIRTY state
        auto new_entry = std::make_unique<CacheEntry>(*cacheLine, cacheline_index);

        // Set entry values
        new_entry->value = val;
        // Pointer was created in constructor, no need to redo make_shared
        *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value).value_or(standardNumericValue); // Default to 0 if conversion fails
        new_entry->last_double_value = *new_entry->stable_double_ptr;
        new_entry->state = CacheEntry::EntryState::DIRTY;

        // Insert into cache
        cache[key] = std::move(new_entry);

        // Flush to RapidJSON document for structural integrity
        flush(key);
    }
}

void JSON::setSubDoc(std::string_view const key, JSON const& child, std::string_view const childKey) {
    std::scoped_lock const lockGuard(mtx);

    // Delete cache entry
    deleteCacheEntry(key);

    // Flush own contents
    flush(key);
    helperNonConstVar++; // Signal non-const operation
    child.flush(childKey);

    if (auto const* childVal = RjDirectAccess::traversePath(childKey, child.doc); childVal == nullptr) {
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

            // Delete cache entry
            deleteCacheEntry(childKey);

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

    // Check if cache holds the key mark as deleted
    deleteCacheEntry(key);

    // Since we inserted an entire document, we need sync its children
    synchronizeChildren(key);
}

void JSON::setEmptyArray(std::string_view const key) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation
    flush(key);
    rapidjson::Value* val = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator());
    val->SetArray();
}

//------------------------------------------
// Serialize/Deserialize

std::string JSON::serialize(std::string_view const key, RjDirectAccess::SerializationType const type) const {
    std::scoped_lock const lockGuard(mtx);
    flush(key); // Ensure all changes are reflected in the document
    if (key.empty()) {
        // Serialize entire doc
        return RjDirectAccess::serialize(doc, type);
    }
    JSON const sub = getSubDoc(key);
    return sub.serialize();
}

void JSON::deserialize(std::string_view const serialOrLink) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Reset document and cache
    flush("");
    doc.SetObject();
    for (auto const& entry : std::views::values(cache)) {
        deleteCacheEntry(entry);
    }

    //------------------------------------------
    // Load the JSON file
    RjDirectAccess::deserialize(doc, serialOrLink);

    //------------------------------------------
    // Sync all cache entries
    synchronizeChildren("");
}

//------------------------------------------
// Key Types, Sizes

KeyType JSON::memberType(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains(SpecialCharacter::transformationPipe)) {
        // Apply transformations to a temp document
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.memberType("");
        }
        return KeyType::null;
    }

    // Checking cache is risky, as inner values may have changed ...
    // Once partial flushing is available, we should use that to minimize the performance impact!
    // Flush before accessing the document to ensure integrity
    flush(key);

    // If not cached, check rapidjson doc
    auto const* val = RjDirectAccess::traversePath(key, doc);
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

namespace {

using Fn = bool (rapidjson::Value::*)() const;
using Formatter = std::string (*)(rapidjson::Value const*);

std::array<std::pair<Fn, const char*>, 6> numericTypeList = {{
    {&rapidjson::Value::IsInt64, "value:int:64"},
    {&rapidjson::Value::IsInt, "value:int:32"},
    {&rapidjson::Value::IsDouble, "value:float:64"},
    {&rapidjson::Value::IsFloat, "value:float:32"},
    {&rapidjson::Value::IsUint64, "value:uint:64"},
    {&rapidjson::Value::IsUint, "value:uint:32"}
}};

std::string numberType(rapidjson::Value const* val) {
    for (auto const& [checkFunc, typeStr] : numericTypeList) {
        if ((val->*checkFunc)()) {
            return typeStr;
        }
    }
    std::unreachable();
}

std::array<std::pair<Fn, Formatter>, 6> constexpr generalTypeList = {{
    {&rapidjson::Value::IsNull, [](rapidjson::Value const*) -> std::string { return "null"; }},
    {&rapidjson::Value::IsArray, [](rapidjson::Value const* val) -> std::string { return "array:" + std::to_string(val->Size()); }},
    {&rapidjson::Value::IsObject, [](rapidjson::Value const* val) -> std::string { return "object:" + std::to_string(val->MemberCount()); }},
    {&rapidjson::Value::IsNumber, [](rapidjson::Value const* val) -> std::string { return numberType(val); }},
    {&rapidjson::Value::IsString, [](rapidjson::Value const* val) -> std::string { return "value:string:" + std::to_string(val->GetStringLength()); }},
    {&rapidjson::Value::IsBool, [](rapidjson::Value const*) -> std::string { return "value:bool"; }}
}};

} // namespace

std::string JSON::memberTypeString(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains(SpecialCharacter::transformationPipe)) {
        // Apply transformations to a temp document
        if (JSON tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.memberTypeString("");
        }
        return "null";
    }

    // Checking cache is risky, as inner values may have changed ...
    // Once partial flushing is available, we should use that to minimize the performance impact!
    // Flush before accessing the document to ensure integrity
    flush(key);
    auto const* val = RjDirectAccess::traversePath(key, doc);
    if (val == nullptr) {
        return "null";
    }
    for (auto const& [checkFunc, typeFunc] : generalTypeList) {
        if ((val->*checkFunc)()) {
            return typeFunc(val);
        }
    }
    std::unreachable(); // If it's any other type, something went wrong...
}

size_t JSON::memberSize(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains(SpecialCharacter::transformationPipe)) {
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
    flush(key); // Ensure cache is flushed before accessing doc
    auto const* val = RjDirectAccess::traversePath(key, doc);
    return val->Size();
}

void JSON::removeMember(std::string_view const key) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Ensure cache is flushed before removing key
    flush(key);

    // Remove member from cache, synchronize children
    cache.erase(key);
    RjDirectAccess::removeMember(key, doc);
    synchronizeChildren(key);
}

void JSON::moveMember(std::string_view const fromKey, std::string_view const toKey) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Ensure cache is flushed before moving key
    flush("");

    // Edge case: toKey starts with fromKey, we need a temporary key to avoid deleting after moving
    if (std::string(toKey).starts_with(std::string(fromKey))) {
        // Edge case 2: if fromKey is empty
        if (std::string(fromKey).empty()) {
            setSubDoc(toKey, *this);

            // Normalize toKey by removing trailing dot if present
            auto toKeyStr = std::string(toKey);
            if (toKeyStr.ends_with(SpecialCharacter::dot)) {
                toKeyStr.pop_back();
            }

            // Remove all other members except toKey
            for (auto const keys = listAvailableKeys(); auto const& key : keys) {
                if (key != toKeyStr) removeMember(key);
            }

            return;
        }

        std::string const tempKey = std::string("__temp_move_") + fromKey; // Unlikely to collide with existing keys
        setSubDoc(tempKey, *this, fromKey);
        setSubDoc(toKey, *this, tempKey);
        removeMember(tempKey);
    }
    else {
        // Direct move without temporary key
        setSubDoc(toKey, *this, fromKey);
        removeMember(fromKey);
    }
}

void JSON::copyMember(std::string_view const fromKey, std::string_view const toKey) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Ensure cache is flushed before copying key
    flush("");

    setSubDoc(toKey, *this, fromKey);
}

std::vector<std::string> JSON::listAvailableKeys(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Flush cache before accessing document
    flush(key);

    // Traverse to the specified key
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key, doc); val != nullptr) {
        return RjDirectAccess::listAvailableKeys(*val);
    }
    return {};
}

//------------------------------------------
// Threadsafe sets

// TODO: optimize by avoiding double cache lookups
// special get-function that returns the cache pointer instead of value

void JSON::set_add(std::string_view const key, double const val) {
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key).value_or(standardNumericValue); // Default to 0 if retrieval fails
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

void JSON::set_add(std::string_view const key, std::int64_t const val) {
    std::scoped_lock const lockGuard(mtx);
    static_assert(Math::isZero(standardNumericValue),
        "This function relies on the standard numeric value being 0 for correct defaulting."
        " If this assertion fails, please review the implementation of set_add for int"
        " and ensure it properly defaults to 0 when retrieval fails."
    );
    auto const current = getVariant(key).value_or(static_cast<int>(standardNumericValue));
    std::visit([&]<typename T>(T const& currentVal) {
        // Check if it's an integer
        if constexpr(std::is_integral_v<T>) {
            set<int64_t>(key, static_cast<int64_t>(currentVal) + val);
        }
        else if constexpr(std::is_floating_point_v<T>) {
            set<double>(key, currentVal + static_cast<double>(val));
        }
        else {
            auto const currentDbl = get<double>(key).value_or(static_cast<int>(standardNumericValue));
            set<double>(key, currentDbl + static_cast<double>(val));
        }
    }, current);
}

void JSON::set_multiply(std::string_view const key, double const val) {
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key).value_or(standardNumericValue); // Default to 0 if retrieval fails
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

void JSON::set_multiply(std::string_view const key, std::int64_t const val) {
    std::scoped_lock const lockGuard(mtx);
    static_assert(Math::isZero(standardNumericValue),
        "This function relies on the standard numeric value being 0 for correct defaulting."
        " If this assertion fails, please review the implementation of set_add for int"
        " and ensure it properly defaults to 0 when retrieval fails."
    );
    auto const current = getVariant(key).value_or(static_cast<int>(standardNumericValue));
    std::visit([&]<typename T>(T const& currentVal) {
        // Check if it's an integer
        if constexpr(std::is_integral_v<T>) {
            set<int64_t>(key, static_cast<int64_t>(currentVal) * val);
        }
        else if constexpr(std::is_floating_point_v<T>) {
            set<double>(key, currentVal * static_cast<double>(val));
        }
        else {
            auto const currentDbl = get<double>(key).value_or(static_cast<int>(standardNumericValue));
            set<double>(key, currentDbl + static_cast<double>(val));
        }
    }, current);
}

void JSON::set_concat(std::string_view const key, std::string_view const valStr) {
    std::scoped_lock const lockGuard(mtx);

    auto const current = get<std::string>(key).value_or(""); // Default to empty string if retrieval fails
    set<std::string>(key, current + valStr);

    // Update double pointer value to default NAN
    if (auto const it = cache.find(key); it != cache.end()) {
        // Strings Default to 0
        *it->second->stable_double_ptr = standardNumericValue;
        it->second->last_double_value = standardNumericValue;
    }
}

//------------------------------------------
// JSON - Rapidjson

std::expected<RjDirectAccess::simpleValue, SimpleValueRetrievalError> JSON::getSimpleValueFromDocument(std::string_view const key) const {
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key, doc); val != nullptr) {
        auto it = cache.find(key);
        if (it == cache.end() && RjDirectAccess::getSimpleValue(val).has_value()) {
            // Insert only if the value is of a supported type, otherwise complex types might be interpreted as simple values.
            // Create new cache entry and insert into cache
            auto new_entry = std::make_unique<CacheEntry>(*cacheLine, cacheline_index);
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
                *it->second->stable_double_ptr = convertVariant<double>(it->second->value).value_or(standardNumericValue); // Default to 0 if conversion fails
                it->second->last_double_value = *it->second->stable_double_ptr;

                return v.value();
            }
        }
        if (val->IsNull()) {
            return std::unexpected(SimpleValueRetrievalError::IS_NULL);
        }
        if (val->IsArray()) {
            return std::unexpected(SimpleValueRetrievalError::IS_ARRAY);
        }
        if (val->IsObject()) {
            return std::unexpected(SimpleValueRetrievalError::IS_OBJECT);
        }
        return std::unexpected(SimpleValueRetrievalError::CONVERSION_FAILURE);
    }

    // Value could not be found, return empty optional
    return std::unexpected(SimpleValueRetrievalError::IS_NULL);
}

} // namespace Nebulite::Data
