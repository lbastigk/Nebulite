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

namespace {

template<typename T>
concept ArrayLike = std::is_array_v<T> || requires(T t) {t.size(); t[0];};

template<typename T>
struct array_traits;

template<typename T, std::size_t N>
struct array_traits<std::array<T, N>> {
    using value_type = T;
    static constexpr std::size_t size = N;
};

template<typename T, std::size_t N>
struct array_traits<T[N]> {
    using value_type = T;
    static constexpr std::size_t size = N;
};

} // namespace



template<Reflectable Obj>
Obj JsonScope::getObject() const {
    Obj obj{};
    template for (auto constexpr member : define_static_array(nonstatic_data_members_of(^^Obj, std::meta::access_context::current()))) {
        using T = typename[: type_of(member) :];
        auto key = getRootScope().addMember(identifier_of(member));
        if constexpr (ArrayLike<T>) {
            using Elem = array_traits<T>::value_type;
            constexpr std::size_t N = array_traits<T>::size;
            auto& scope = shareScope(key);
            for (std::size_t i = 0; i < N; ++i) {
                auto idxKey = scope.getRootScope().addIndex(i);

                if constexpr (isSimpleValue<Elem>) {
                    obj.[:member:][i] =
                        get<Elem>(idxKey.view())
                            .value_or(obj.[:member:][i]);
                } else {
                    auto child = scope.shareScope(idxKey);
                    obj.[:member:][i] =
                        child.getObject<Elem>();
                }
            }
        }
        else if constexpr (isSimpleValue<T>) {
            obj.[:member:] = get<T>(key.view()).value_or(obj.[:member:]);
        }
        else {
            auto& scope = shareScope(key);
            obj.[:member:] = scope.getObject<T>();
        }
    }
    return obj;
}

template<Reflectable Obj>
void JsonScope::setObject(Obj const& obj) {
    template for (auto constexpr member : define_static_array(nonstatic_data_members_of(^^Obj, std::meta::access_context::current()))) {
        using T = typename[: type_of(member) :];
        auto key = getRootScope().addMember(identifier_of(member));
        if constexpr (ArrayLike<T>) {
            using Elem = array_traits<T>::value_type;
            constexpr std::size_t N = array_traits<T>::size;
            auto& scope = shareScope(key);
            for (std::size_t i = 0; i < N; ++i) {
                auto idxKey = scope.getRootScope().addIndex(i);
                if constexpr (isSimpleValue<Elem>) {
                    set<Elem>(idxKey.view(), obj.[:member:][i]);
                } else {
                    auto child = scope.shareScope(idxKey);
                    child.setObject<Elem>(obj.[:member:][i]);
                }
            }
        }
        else if constexpr (isSimpleValue<T>) {
            set<T>(key.view(), obj.[:member:]);
        }
        else {
            auto& scope = shareScope(key);
            scope.setObject<T>(obj.[:member:]);
        }
    }
}

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_SCOPE_BASE_TPP
