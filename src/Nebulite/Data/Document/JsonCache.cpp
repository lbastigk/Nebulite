//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <optional>

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

} // namespace Nebulite::Data
