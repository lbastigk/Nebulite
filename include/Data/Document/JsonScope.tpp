#ifndef NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_TPP
#define NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_TPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/JSON.hpp"
#include "Data/Document/ScopedKey.hpp"

//------------------------------------------
namespace Nebulite::Data {

template<typename T>
std::expected<T, SimpleValueRetrievalError> JsonScope::get(ScopedKeyView const& key) const {
    return baseDocument->get<T>(key.full(*this));
}

template<typename T>
void JsonScope::set(ScopedKeyView const& key, T const& value) {
    baseDocument->set<T>(key.full(*this), value);
}

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_TPP
