#ifndef NEBULITE_DATA_DOCUMENT_JSONCACHE_TPP
#define NEBULITE_DATA_DOCUMENT_JSONCACHE_TPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <optional>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/RjDirectAccess.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_DOCUMENT_JSONCACHE_HPP
    #include "Nebulite/Data/Document/JsonCache.hpp"
#endif // NEBULITE_DATA_DOCUMENT_JSONCACHE_HPP

//------------------------------------------
namespace Nebulite::Data {

template <typename NewType>
std::optional<NewType> CacheEntry::convertTo(){
    return RjDirectAccess::convertSimpleValue<NewType>(value);
}

template<typename F>
void JsonCache::insert(std::string_view const key, F&& modifier) {
    static_assert(std::is_invocable_r_v<void, F, CacheEntry&>, "Modifier function must be invocable with CacheEntry& and return void.");
    assert(!find(key).has_value() && "Key already exists in cache.");

    auto& newEntry = createNewCacheEntry(key);
    std::invoke(std::forward<F>(modifier), newEntry);
}

template<typename R, typename ModifierFunc, typename ReturnFunc>
R JsonCache::insertAndGet(std::string_view const key, ModifierFunc&& modifierFunc, ReturnFunc&& returnFunc) {
    static_assert(std::is_invocable_r_v<void, ModifierFunc, CacheEntry&>, "Modifier function must be invocable with CacheEntry& and return void.");
    static_assert(std::is_invocable_r_v<R, ReturnFunc, CacheEntry&>, "Result function must be invocable with CacheEntry& and return R.");
    assert(!find(key).has_value() && "Key already exists in cache.");

    auto& newEntry = createNewCacheEntry(key);
    std::invoke(std::forward<ModifierFunc>(modifierFunc), newEntry);
    auto r = std::invoke(std::forward<ReturnFunc>(returnFunc), newEntry);
    return r;
}

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSONCACHE_TPP
