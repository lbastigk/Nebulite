#ifndef NEBULITE_DATA_DOCUMENT_JSONSCOPE_TPP
#define NEBULITE_DATA_DOCUMENT_JSONSCOPE_TPP

//------------------------------------------
// Includes

// Standard library
#include <expected>

// Nebulite
#include "Nebulite/Data/Document/JSON.hpp"
#include "Nebulite/Data/Document/ScopedKey.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_DOCUMENT_JSONSCOPE_HPP
    #include "Nebulite/Data/Document/JsonScope.hpp"
#endif // NEBULITE_DATA_DOCUMENT_JSONSCOPE_HPP

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
#endif // NEBULITE_DATA_DOCUMENT_JSONSCOPE_TPP
