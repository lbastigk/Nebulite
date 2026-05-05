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

template<Reflectable Obj>
Obj JsonScope::getObject() {
    Obj obj{};
    (void)obj;
    (void)this;
    std::unreachable();

    // TODO: not yet implemented
    // for each member M m of obj, call:
    // obj.m = get<M>(getRootScope().addMember(identifier_of(m))).value_or(obj.m);
    // if M is not a simple value, call:
    // auto& innerScope = shareScope(identifier_of(m));
    // obj.m = innerScope.getObject<M>();
}

template<Reflectable Obj>
void JsonScope::setObject(Obj const& obj) {
    (void)obj;
    (void)this;
    std::unreachable();

    // TODO: not yet implemented
    // for each member M m of obj, call:
    // set<M>(getRootScope().addMember(identifier_of(m)), m);
    // if M is not a simple value, call:
    // auto& innerScope = shareScope(identifier_of(m));
    // inner.setObject<M>(m);
}

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_TPP
