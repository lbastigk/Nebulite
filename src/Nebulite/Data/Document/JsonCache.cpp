//------------------------------------------
// Includes

// Standard library
#include <cstddef>
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

CacheEntry::CacheEntry([[clang::lifetimebound]] CacheLine& cl, std::size_t& index) {
    if (index >= cachelineSize) [[unlikely]] {
        stableDoublePointer = new double(standardNumericValue);
        managedInternalDouble = true;
    }
    else [[likely]] {
        // Assign stable double pointer from cacheline
        stableDoublePointer = &cl[index];
        index++;
        *stableDoublePointer = standardNumericValue;
        managedInternalDouble = false;
    }
}

CacheEntry::~CacheEntry() {
    if (managedInternalDouble) {
        delete stableDoublePointer;
    }
}

void CacheEntry::updateNumericValue(){
    if (!Math::isEqualAllowNan(*stableDoublePointer, lastDoubleValue)) {
        // Value changed since last check
        lastDoubleValue = *stableDoublePointer;
        value = lastDoubleValue;
        state = State::dirty;
    }
}

void CacheEntry::markAsDeleted() {
    state = State::deleted;
    value = standardNumericValue;
    *stableDoublePointer = standardNumericValue;
    lastDoubleValue = standardNumericValue;
}

void CacheEntry::setValueClean(RjDirectAccess::SimpleValue const& newValue) {
    state = State::clean;
    value = newValue;
    *stableDoublePointer = convertTo<double>().value_or(standardNumericValue);
    lastDoubleValue = *stableDoublePointer;
}

void CacheEntry::setValueDirty(RjDirectAccess::SimpleValue const& newValue) {
    state = State::dirty;
    value = newValue;
    *stableDoublePointer = convertTo<double>().value_or(standardNumericValue);
    lastDoubleValue = *stableDoublePointer;
}

CacheEntry& JsonCache::createNewCacheEntry(std::string_view const key) {
    auto newEntry = std::make_shared<CacheEntry>(*cacheLine, cacheLineIndex);
    cache[key] = newEntry;
    cacheVector.emplace_back(std::string(key), newEntry);
    return *newEntry.get();
}

JsonCache::JsonCache() : cacheLine(std::make_unique<CacheLine>()) {}

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
