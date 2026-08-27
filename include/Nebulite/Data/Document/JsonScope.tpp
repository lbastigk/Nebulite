#ifndef NEBULITE_DATA_DOCUMENT_JSONSCOPE_TPP
#define NEBULITE_DATA_DOCUMENT_JSONSCOPE_TPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <complex>
#include <concepts>
#include <cstddef>
#include <cstdint> // NOLINT
#include <expected>
#include <optional>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>

// Nebulite
#include "Nebulite/Data/Document/Json.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"
#include "Nebulite/Utility/Ranges.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_DOCUMENT_JSONSCOPE_HPP
    #include "Nebulite/Data/Document/JsonScope.hpp"
#endif // NEBULITE_DATA_DOCUMENT_JSONSCOPE_HPP

//------------------------------------------
namespace Nebulite::Data {

template<typename DocType>
std::optional<std::string> JsonScope::generateScopePrefix(DocType const& doc, std::optional<std::string> const& prefix) {
    if constexpr (std::is_same_v<DocType, Json>) {
        if (prefix.has_value()) {
            return {generatePrefix(prefix.value())};
        }
    } else if constexpr (std::is_same_v<DocType, JsonScope>) {
        if (prefix.has_value()) { // More complicated: We need to generate the full prefix based on the other JsonScope and the new prefix
            return {ScopedKeyView(generatePrefix(prefix.value())).full(doc)};
        }
    } else {
        static_assert(std::is_same_v<DocType, Json> || std::is_same_v<DocType, JsonScope>, "Unsupported document type for generateScopePrefix");
    }
    // No prefix provided
    return std::nullopt;
}

template<typename T, typename... Keys>
auto JsonScope::getMultiple(Keys const&... keys) const {
    return std::tuple{
        get<T>(keys)...
    };
}

template<typename T>
std::expected<T, SimpleValueRetrievalError> JsonScope::get(ScopedKeyView const& key) const {
    return baseDocument->get<T>(key.full(*this));
}

template<typename T>
void JsonScope::set(ScopedKeyView const& key, T const& value) {
    baseDocument->set<T>(key.full(*this), value);
}

template <std::ranges::input_range R>
void JsonScope::setArray(ScopedKeyView const& key, R const& range) {
    setEmptyArray(key);
    for (auto const [index, indexKey] : getArrayKeys(key, range.size()) | Utility::Ranges::enumerate) {
        if constexpr (std::is_same_v<typename R::value_type, std::complex<double>>) {
            setComplex(indexKey, range[index]);
        }
        else if constexpr (std::is_same_v<typename R::value_type, Json> || std::is_same_v<typename R::value_type, JsonScope>) {
            setSubDoc(indexKey, range[index]);
        }
        else {
            set<typename R::value_type>(indexKey, range[index]);
        }
    }
}

template <std::ranges::input_range R> requires std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<R>>,ScopedKeyView>
double** JsonScope::ensureOrderedCacheList(std::uint64_t uniqueId, R const& keys) {
    thread_local std::size_t const threadIndex = assignCacheLookupIndex();
    assert(threadIndex < cacheLookupThreadCount && "Thread index exceeds non-locking array size! Too many threads accessing ordered cache lists, increase cacheLookupThreadCount or reduce thread count.");
    return odpCache[threadIndex].get(*this).ensureOrderedCacheListNoLock(uniqueId, keys);
}

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSONSCOPE_TPP
