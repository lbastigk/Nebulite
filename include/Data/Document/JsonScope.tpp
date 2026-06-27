#ifndef DATA_DOCUMENT_JSONSCOPE_TPP
#define DATA_DOCUMENT_JSONSCOPE_TPP

//------------------------------------------
// Includes

// Standard library
#include <expected>

// Nebulite
#include "Data/Document/JSON.hpp"
#include "Data/Document/ScopedKey.hpp"
#include "Data/Document/SimpleValueError.hpp"

//------------------------------------------
// Conditional includes

#ifndef DATA_DOCUMENT_JSONSCOPE_HPP
    #include "Data/Document/JsonScope.hpp"
#endif // DATA_DOCUMENT_JSONSCOPE_HPP

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
#endif // DATA_DOCUMENT_JSONSCOPE_TPP
