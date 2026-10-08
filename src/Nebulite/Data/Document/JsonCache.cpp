//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <memory>
#include <optional>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/JsonCache.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Math/Equality.hpp"

//------------------------------------------
namespace Nebulite::Data {

CacheEntry::CacheEntry([[clang::lifetimebound]] CacheAllocator<standardNumericValue>& allocator) : stableDoublePointer(allocator.allocate<2>()) {
    lastDoubleValue = stableDoublePointer + 1;
}

CacheEntry::~CacheEntry() = default;

void CacheEntry::updateNumericValue(){
    if (!Math::isEqualAllowNan(*stableDoublePointer, *lastDoubleValue)) {
        // Value changed since last check
        *lastDoubleValue = *stableDoublePointer;
        value = *lastDoubleValue;
        state = State::dirty;
    }
}

void CacheEntry::markAsDeleted() {
    state = State::deleted;
    value = standardNumericValue;
    *stableDoublePointer = standardNumericValue;
    *lastDoubleValue = standardNumericValue;
}

void CacheEntry::setValueClean(RjDirectAccess::SimpleValue const& newValue) {
    state = State::clean;
    value = newValue;
    *stableDoublePointer = convertTo<double>().value_or(standardNumericValue);
    *lastDoubleValue = *stableDoublePointer;
}

void CacheEntry::setValueDirty(RjDirectAccess::SimpleValue const& newValue) {
    state = State::dirty;
    value = newValue;
    *stableDoublePointer = convertTo<double>().value_or(standardNumericValue);
    *lastDoubleValue = *stableDoublePointer;
}

CacheEntry& JsonCache::createNewCacheEntry(std::string_view const key) {
    auto newEntry = std::make_shared<CacheEntry>(allocator);
    cache[key] = newEntry;
    cacheVector.emplace_back(std::string(key), newEntry);
    return *newEntry.get();
}

JsonCache::JsonCache() = default;

JsonCache::JsonCache(JsonCache&& other) noexcept = default;

JsonCache& JsonCache::operator=(JsonCache&& other) noexcept = default;

JsonCache::~JsonCache() {
    cache.clear();
    cacheVector.clear();
}

void JsonCache::clear() {
    cache.clear();
    cacheVector.clear();
}

void JsonCache::deleteEntry(std::string_view const key) {
    if (auto const it = cache.find(key); it != cache.end()) {
        it->second->markAsDeleted();
    }
}

[[nodiscard]] auto JsonCache::begin() const [[clang::lifetimebound]] -> decltype(cacheVector.begin()) {
    return cacheVector.begin();
}

[[nodiscard]] auto JsonCache::end() const [[clang::lifetimebound]] -> decltype(cacheVector.end()) {
    return cacheVector.end();
}

[[nodiscard]] std::optional<CacheEntry&> JsonCache::find(std::string_view const key) const [[clang::lifetimebound]] {
    if (auto const it = cache.find(key); it != cache.end()) {
        return *it->second;
    }
    return std::nullopt;
}

} // namespace Nebulite::Data
