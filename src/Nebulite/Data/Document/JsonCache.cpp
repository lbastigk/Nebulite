//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <memory>
#include <optional>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/JsonCache.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Math/Equality.hpp"

//------------------------------------------
namespace Nebulite::Data {

void CacheEntry::updateNumericValue(){
    if (!Math::isEqualAllowNan(*stableDoublePointer, lastDoubleValue)) {
        // Value changed since last check
        lastDoubleValue = *stableDoublePointer;
        value = lastDoubleValue;
        state = EntryState::dirty;
    }
}

void CacheEntry::markAsDeleted() {
    state = EntryState::deleted;
    value = standardNumericValue;
    *stableDoublePointer = standardNumericValue;
    lastDoubleValue = standardNumericValue;
}

void CacheEntry::setValueClean(RjDirectAccess::SimpleValue const& newValue) {
    state = EntryState::clean;
    value = newValue;
    *stableDoublePointer = convertTo<double>().value_or(standardNumericValue);
    lastDoubleValue = *stableDoublePointer;
}

void CacheEntry::setValueDirty(RjDirectAccess::SimpleValue const& newValue) {
    state = EntryState::dirty;
    value = newValue;
    *stableDoublePointer = convertTo<double>().value_or(standardNumericValue);
    lastDoubleValue = *stableDoublePointer;
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

void JsonCache::deleteEntry(std::string_view key) {
    if (auto it = cache.find(key); it != cache.end()) {
        it->second->markAsDeleted();
    }
}

[[nodiscard]] auto JsonCache::begin() const [[clang::lifetimebound]] -> decltype(cacheVector.begin()) {
    return cacheVector.begin();
}

[[nodiscard]] auto JsonCache::end() const [[clang::lifetimebound]] -> decltype(cacheVector.end()) {
    return cacheVector.end();
}

[[nodiscard]] std::optional<CacheEntry&> JsonCache::find(std::string_view key) const [[clang::lifetimebound]] {
    auto it = cache.find(key);
    if (it != cache.end()) {
        return *it->second;
    }
    return std::nullopt;
}

} // namespace Nebulite::Data
