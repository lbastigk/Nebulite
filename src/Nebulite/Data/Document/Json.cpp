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
#include "Nebulite/Data/Document/Json.hpp"
#include "Nebulite/Data/Document/JsonCache.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/JsonTransformer.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Data {

//------------------------------------------
// Prefix: parent finder

std::string_view Json::findParentKey(std::string_view const key) {
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

Json::Json() = default;

Json::~Json() {
    std::scoped_lock const lockGuard(mtx);
    doc.SetObject();
}

//------------------------------------------
// Allow move

Json& Json::operator=(Json&& other) noexcept {
    if (this != &other) {
        std::scoped_lock const lockGuard(mtx, other.mtx);
        doc = std::move(other.doc);
        cache = std::move(other.cache);
    }
    return *this;
}

Json::Json(Json&& other) noexcept : cache(std::move(other.cache)), doc(std::move(other.doc)) {}

//------------------------------------------
// Scope sharing

JsonScope& Json::shareManagedScope(std::string_view const prefix) {
    std::scoped_lock const lockGuard(mtx);

    if (auto const it = managedScopes.find(prefix); it != managedScopes.end()) {
        return *it->second;
    }
    managedScopes[prefix] = std::make_unique<JsonScope>(*this, std::string(prefix));
    return *managedScopes[prefix];
}

//------------------------------------------
// Dummy sharing

JsonScope& Json::getDummyScope() {
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
void Json::copyFrom(Json const& other) {
    setSubDoc("", other);
}

//------------------------------------------
// Validity check

/**
 * @brief Checks if a string is in JSON or JSONC format.
 * @param str The string to check.
 * @return true if the string is JSON or JSONC, false otherwise.
 */
bool Json::isJsonOrJsonc(std::string_view const str) {
    return RjDirectAccess::isJsonOrJsonc(str);
}

//------------------------------------------
// Argument splitting for transformations

std::vector<std::string_view> Json::splitKeyWithTransformations(std::string_view const key) {
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

JsonScope& Json::fullScope() {
    std::scoped_lock const lockGuard(mtx);
    if (!fullScopeInstance) {
        fullScopeInstance = std::make_unique<JsonScope>(*this, "");
    }
    return *fullScopeInstance;
}

void Json::synchronizeChildren(std::string_view const parentKey) const {
    std::scoped_lock const lockGuard(mtx);

    // Find all child keys and invalidate them
    for (auto& [key, entry] : cache.cache) {
        bool const base = key.starts_with(parentKey) && key.length() > parentKey.length();
        bool const startsWithParentKeyPlusDot = base && key[parentKey.length()] == SpecialCharacter::dot;
        bool const startsWithParentKeyPlusArr = base && key[parentKey.length()] == SpecialCharacter::arrayOpen;
        if (bool const parentKeyIsRoot = parentKey.empty(); parentKeyIsRoot || startsWithParentKeyPlusDot || startsWithParentKeyPlusArr) {
            if (auto const variant = RjDirectAccess::getSimpleValue(key, doc); variant.has_value()) {
                entry->setValueClean(variant.value());
            }
            else {
                entry->markAsDeleted();
            }
        }
    }
}

void Json::flush(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    auto const parent = findParentKey(key);

    for (auto& [entryKey, entry] : cache.cache) {
        if (!entryKey.starts_with(parent)) continue;

        // Skip malformed entries
        if (entry->state == CacheEntry::EntryState::malformed) {
            continue;
        }

        // Every dirty entry is flushed back to the document and marked clean
        entry->updateNumericValue();
        if (entry->state == CacheEntry::EntryState::dirty) {
            (void)RjDirectAccess::set(entryKey.c_str(), entry->value, doc, doc.GetAllocator());
            entry->state = CacheEntry::EntryState::clean;
        }
    }
}

//------------------------------------------
// Get methods

std::expected<RjDirectAccess::SimpleValue, SimpleValueRetrievalError> Json::getVariant(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.contains(SpecialCharacter::transformationPipe)) {
        if (Json tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp.getVariant(Module::Base::TransformationModule::rootKeyStr);
        }
        return std::unexpected(SimpleValueRetrievalError::transformationFailure);
    }

    // Check cache first
    if (!std::ranges::any_of(cache.cache, [&key](auto const& pair) {
        auto& [cachedKey, entry] = pair;
        return cachedKey.starts_with(key)
            && cachedKey != key
            && entry->state != CacheEntry::EntryState::deleted
            && Math::isEqualAllowNan(*entry->stableDoublePointer, entry->lastDoubleValue);
    })) {
        // Checking for malformed shouldn't be necessary, but just in case
        auto const it = cache.cache.find(key);
        if (it != cache.cache.end() && it->second->state == CacheEntry::EntryState::malformed) {
            Global::capture().error.println("Warning: Attempted to access malformed key in getVariant(): ", key);
            Global::capture().error.println("This is a serious logic issue, the malformed key check should have happened already. Please report to the developers!");
            return std::unexpected(SimpleValueRetrievalError::malformedKey);
        }

        if (it != cache.cache.end() && it->second->state != CacheEntry::EntryState::deleted) {
            // Entry exists and is not deleted
            it->second->updateNumericValue();
            return it->second->value;
        }
    }

    // Check document, if not in cache
    flush(key);
    return getSimpleValueFromDocument(key);
}

Json Json::getSubDoc(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Handle integrity via flushing
    // Makes sure we don't have to worry about cache and double pointers here
    // Full access to the rapidjson document after this point
    flush(key);

    // Check if a transformation is present
    if (key.contains(SpecialCharacter::transformationPipe)) {
        if (Json tmp; getSubDocWithTransformations(key, tmp)) {
            return tmp;
        }
        return Json{};
    }

    if (rapidjson::Value const* keyVal = RjDirectAccess::traversePath(key, doc); keyVal != nullptr) {
        // turn keyVal to doc
        Json json;
        json.doc.CopyFrom(*keyVal, json.doc.GetAllocator());
        return json;
    }
    return Json{};
}

bool Json::getSubDocWithTransformations(std::string_view const key, Json& outDoc) const {
    auto args = splitKeyWithTransformations(key);
    {
        auto const& baseKey = args[0];

        // Using getSubDoc to properly populate the tempDoc with the rapidjson::Value
        // Slower than a manual copy that handles types, but more secure and less error-prone
        outDoc = getSubDoc(baseKey);
    }

    // Apply each transformation in sequence
    args.erase(args.begin());
    return JsonTransformer::instance().parse(args, outDoc);
}

double* Json::getStableDoublePointer(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check for transformations
    if (key.contains(SpecialCharacter::transformationPipe)) {
        throw std::runtime_error("Transformations are not supported in getStableDoublePointer()");
    }

    // Check cache first
    if (auto const it = cache.cache.find(key); it != cache.cache.end()) {
        // If the entry is deleted, we need to update its value from the document
        if (it->second->state == CacheEntry::EntryState::deleted) {
            *it->second->stableDoublePointer = get<double>(key).value_or(0.0); // Default to 0.0 if retrieval fails
            it->second->lastDoubleValue = *it->second->stableDoublePointer;
            it->second->state = CacheEntry::EntryState::derived;
        }
        return it->second->stableDoublePointer;
    }

    // Try loading from document into cache
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key, doc); val != nullptr) {
        if (jsonValueToCache<double>(key, val).has_value()) {
            // Successfully loaded into cache, return pointer
            return cache.cache[key]->stableDoublePointer;
        }
    }

    // If loading from document failed, create a new derived entry
    auto newEntry = std::make_unique<CacheEntry>(*cache.cacheLine, cache.cacheLineIndex);
    newEntry->value = CacheEntry::standardNumericValue;
    *newEntry->stableDoublePointer = CacheEntry::standardNumericValue;
    newEntry->lastDoubleValue = CacheEntry::standardNumericValue;
    newEntry->state = CacheEntry::EntryState::derived;
    auto* const ptr = newEntry->stableDoublePointer;
    cache.cache[key] = std::move(newEntry);
    return ptr;
}

std::unique_lock<std::recursive_mutex> Json::lock() const {
    return std::unique_lock(mtx);
}


//------------------------------------------
// Set methods

void Json::setVariant(std::string_view const key, RjDirectAccess::SimpleValue const& val) {
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
    if (auto const it = cache.cache.find(key); it != cache.cache.end()) {
        // Existing cache value, structure validity guaranteed
        it->second->setValueDirty(val);
    } else {
        // New cache value, structural validity is not guaranteed
        // so we flush contents into the rapidjson document after inserting

        // Synchronize structure
        synchronizeChildren(key);

        // Create new entry directly in DIRTY state
        auto newEntry = std::make_unique<CacheEntry>(*cache.cacheLine, cache.cacheLineIndex);
        newEntry->setValueDirty(val);

        // Insert into cache
        cache.cache[key] = std::move(newEntry);

        // Flush to RapidJSON document for structural integrity
        flush(key);
    }
}

void Json::setSubDoc(std::string_view const key, Json const& child, std::string_view const childKey) {
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

void Json::setEmptyArray(std::string_view const key) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation
    flush(key);
    rapidjson::Value* val = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator());
    val->SetArray();
}

void Json::setEmptyObject(std::string_view const key) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation
    flush(key);
    rapidjson::Value* val = RjDirectAccess::ensurePath(key, doc, doc.GetAllocator());
    val->SetObject();
}

//------------------------------------------
// Serialize/Deserialize

std::string Json::serialize(std::string_view const key, RjDirectAccess::SerializationType const type) const {
    std::scoped_lock const lockGuard(mtx);
    flush(key); // Ensure all changes are reflected in the document
    if (key.empty()) {
        // Serialize entire doc
        return RjDirectAccess::serialize(doc, type);
    }
    Json const sub = getSubDoc(key);
    return sub.serialize();
}

void Json::deserialize(std::string_view const serialOrLink) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Reset document and cache
    flush("");
    doc.SetObject();
    for (auto const& entry : std::views::values(cache.cache)) {
        entry->markAsDeleted();
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

KeyType Json::memberType(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains(SpecialCharacter::transformationPipe)) {
        // Apply transformations to a temp document
        if (Json tmp; getSubDocWithTransformations(key, tmp)) {
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

std::array<std::pair<Fn, char const*>, 6> numericTypeList = {{
    {&rapidjson::Value::IsInt64, "value:int:64"},
    {&rapidjson::Value::IsInt, "value:int:32"},
    {&rapidjson::Value::IsDouble, "value:float:64"},
    {&rapidjson::Value::IsFloat, "value:float:32"},
    {&rapidjson::Value::IsUint64, "value:uint:64"},
    {&rapidjson::Value::IsUint, "value:uint:32"},
},};

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
    {&rapidjson::Value::IsBool, [](rapidjson::Value const*) -> std::string { return "value:bool"; }},
},};

} // namespace

std::string Json::memberTypeString(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains(SpecialCharacter::transformationPipe)) {
        // Apply transformations to a temp document
        if (Json tmp; getSubDocWithTransformations(key, tmp)) {
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

size_t Json::memberSize(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // See if transformations are present
    if (key.contains(SpecialCharacter::transformationPipe)) {
        // Apply transformations to a temp document
        if (Json tempDoc; getSubDocWithTransformations(key, tempDoc)) {
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

void Json::removeMember(std::string_view const key) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Ensure cache is flushed before removing key
    flush(key);

    // Remove member from cache, synchronize children
    cache.cache.erase(key);
    RjDirectAccess::removeMember(key, doc);
    synchronizeChildren(key);
}

void Json::moveMember(std::string_view const fromKey, std::string_view const toKey) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Ensure cache is flushed before moving key
    flush("");

    // Edge case: toKey starts with fromKey, we need a temporary key to avoid deleting after moving
    if (std::string(toKey).starts_with(fromKey)) {
        // Edge case 2: if fromKey is empty
        if (std::string(fromKey).empty()) {
            setSubDoc(toKey, *this);

            // Normalize toKey by removing trailing dot if present
            auto toKeyStr = std::string(toKey);
            if (toKeyStr.ends_with(SpecialCharacter::dot)) {
                toKeyStr.pop_back();
            }

            // Remove all other members except toKey
            for (auto const keys = listAvailableMembers(); auto const& key : keys) {
                if (key != toKeyStr) removeMember(key);
            }
        }
        // Edge case 3: if fromKey starts with an array, the temporary key must be inside the array to avoid collisions
        else if (fromKey.starts_with(SpecialCharacter::arrayOpen)) {
            auto const arraySize = memberSize("");
            std::string tempKey = std::string("[") + std::to_string(arraySize) + "]"; // Unlikely to collide with existing keys

            // Edge case 4: If the new temp key collides with toKey, we need to find a new temp key
            if (toKey.starts_with(tempKey)) {
                tempKey = std::string("[") + std::to_string(arraySize + 1) + "]"; // Unlikely to collide with existing keys
            }

            setSubDoc(tempKey, *this, fromKey);
            setSubDoc(toKey, *this, tempKey);
            removeMember(tempKey);
        }
        else {
            std::string const tempKey = std::string("__temp_move_") + fromKey; // Unlikely to collide with existing keys
            setSubDoc(tempKey, *this, fromKey);
            setSubDoc(toKey, *this, tempKey);
            removeMember(tempKey);
        }
    }
    else {
        // Direct move without temporary key
        setSubDoc(toKey, *this, fromKey);
        removeMember(fromKey);
    }
}

void Json::copyMember(std::string_view const fromKey, std::string_view const toKey) {
    std::scoped_lock const lockGuard(mtx);
    helperNonConstVar++; // Signal non-const operation

    // Ensure cache is flushed before copying key
    flush("");

    setSubDoc(toKey, *this, fromKey);
}

std::vector<std::string> Json::listAvailableMembers(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Flush cache before accessing document
    flush(key);

    // Traverse to the specified key
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key, doc); val != nullptr) {
        return RjDirectAccess::listAvailableMembers(*val);
    }
    return {};
}

//------------------------------------------
// Threadsafe sets

// TODO: optimize by avoiding double cache lookups
// special get-function that returns the cache pointer instead of value

void Json::setAdditive(std::string_view const key, double const val) {
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key).value_or(CacheEntry::standardNumericValue); // Default to 0 if retrieval fails
    double const newValue = current + val;

    // Update double pointer value
    if (auto it = cache.cache.find(key); it != cache.cache.end()) {
        *it->second->stableDoublePointer = newValue;
    } else {
        set<double>(key, newValue);
        it = cache.cache.find(key);
        if (it != cache.cache.end()) {
            *it->second->stableDoublePointer = newValue;
            it->second->lastDoubleValue = newValue;
        }
    }
}

void Json::setAdditive(std::string_view const key, std::int64_t const val) {
    std::scoped_lock const lockGuard(mtx);
    static_assert(Math::isZero(CacheEntry::standardNumericValue),
        "This function relies on the standard numeric value being 0 for correct defaulting."
        " If this assertion fails, please review the implementation of setAdditive for int"
        " and ensure it properly defaults to 0 when retrieval fails."
    );
    auto const current = getVariant(key).value_or(static_cast<int>(CacheEntry::standardNumericValue));
    std::visit([&]<typename T>(T const& currentVal) {
        // Check if it's an integer
        if constexpr(std::is_integral_v<T>) {
            set<int64_t>(key, static_cast<int64_t>(currentVal) + val);
        }
        else if constexpr(std::is_floating_point_v<T>) {
            set<double>(key, currentVal + static_cast<double>(val));
        }
        else {
            auto const currentDbl = get<double>(key).value_or(static_cast<int>(CacheEntry::standardNumericValue));
            set<double>(key, currentDbl + static_cast<double>(val));
        }
    }, current);
}

void Json::setMultiplicative(std::string_view const key, double const val) {
    std::scoped_lock const lockGuard(mtx);

    // Get current value
    auto const current = get<double>(key).value_or(CacheEntry::standardNumericValue); // Default to 0 if retrieval fails
    double const newValue = current * val;

    // Update double pointer value
    if (auto it = cache.cache.find(key); it != cache.cache.end()) {
        *it->second->stableDoublePointer = newValue;
    } else {
        set<double>(key, newValue);
        it = cache.cache.find(key);
        if (it != cache.cache.end()) {
            *it->second->stableDoublePointer = newValue;
            it->second->lastDoubleValue = newValue;
        }
    }
}

void Json::setMultiplicative(std::string_view const key, std::int64_t const val) {
    std::scoped_lock const lockGuard(mtx);
    static_assert(Math::isZero(CacheEntry::standardNumericValue),
        "This function relies on the standard numeric value being 0 for correct defaulting."
        " If this assertion fails, please review the implementation of setAdditive for int"
        " and ensure it properly defaults to 0 when retrieval fails."
    );
    auto const current = getVariant(key).value_or(static_cast<int>(CacheEntry::standardNumericValue));
    std::visit([&]<typename T>(T const& currentVal) {
        // Check if it's an integer
        if constexpr(std::is_integral_v<T>) {
            set<int64_t>(key, static_cast<int64_t>(currentVal) * val);
        }
        else if constexpr(std::is_floating_point_v<T>) {
            set<double>(key, currentVal * static_cast<double>(val));
        }
        else {
            auto const currentDbl = get<double>(key).value_or(static_cast<int>(CacheEntry::standardNumericValue));
            set<double>(key, currentDbl + static_cast<double>(val));
        }
    }, current);
}

void Json::setConcatenative(std::string_view const key, std::string_view const valStr) {
    std::scoped_lock const lockGuard(mtx);

    auto const current = get<std::string>(key).value_or(""); // Default to empty string if retrieval fails
    set<std::string>(key, current + valStr);

    // Update double pointer value to default NAN
    if (auto const it = cache.cache.find(key); it != cache.cache.end()) {
        // Strings Default to 0
        *it->second->stableDoublePointer = CacheEntry::standardNumericValue;
        it->second->lastDoubleValue = CacheEntry::standardNumericValue;
    }
}

//------------------------------------------
// Cache management

void Json::deleteCacheEntry(std::string_view const key) const {
    if (auto const it = cache.cache.find(key); it != cache.cache.end()) {
        auto const& entry = it->second;
        entry->markAsDeleted();
    }
}

//------------------------------------------
// JSON - Rapidjson

std::expected<RjDirectAccess::SimpleValue, SimpleValueRetrievalError> Json::getSimpleValueFromDocument(std::string_view const key) const {
    if (rapidjson::Value const* val = RjDirectAccess::traversePath(key, doc); val != nullptr) {
        auto it = cache.cache.find(key);
        if (it == cache.cache.end() && RjDirectAccess::getSimpleValue(val).has_value()) {
            // Insert only if the value is of a supported type, otherwise complex types might be interpreted as simple values.
            // Create new cache entry and insert into cache
            auto newEntry = std::make_unique<CacheEntry>(*cache.cacheLine, cache.cacheLineIndex);
            cache.cache[key] = std::move(newEntry);
            it = cache.cache.find(key);
        }

        if (it != cache.cache.end()) {
            // Modify existing entry
            if (auto const& v = RjDirectAccess::getSimpleValue(val); v.has_value()) {
                it->second->setValueClean(v.value());
                return v.value();
            }
        }
        if (val->IsNull()) {
            return std::unexpected(SimpleValueRetrievalError::isNull);
        }
        if (val->IsArray()) {
            return std::unexpected(SimpleValueRetrievalError::isArray);
        }
        if (val->IsObject()) {
            return std::unexpected(SimpleValueRetrievalError::isObject);
        }
        return std::unexpected(SimpleValueRetrievalError::conversionFailure);
    }

    // Value could not be found, return empty optional
    return std::unexpected(SimpleValueRetrievalError::isNull);
}

} // namespace Nebulite::Data
