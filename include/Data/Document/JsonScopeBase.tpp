#ifndef NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_TPP
#define NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_TPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/JsonScopeBase.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/ScopedKey.hpp"

namespace Nebulite::Data {

//------------------------------------------
// Getter

template<typename T>
T JsonScopeBase::get(ScopedKeyView const& key, T const& defaultValue) const {
    return baseDocument->get<T>(key.full(*this), defaultValue);
}

//------------------------------------------
// Setter

template<typename T>
void JsonScopeBase::set(ScopedKeyView const& key, T const& value) {
    baseDocument->set<T>(key.full(*this), value);
}

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_TPP
