#ifndef NEBULITE_DATA_DOCUMENT_JSON_TPP
#define NEBULITE_DATA_DOCUMENT_JSON_TPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <expected>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// External
#include <rapidjson/document.h>

// Nebulite
#include "Nebulite/Data/Document/JsonCache.hpp"
#include "Nebulite/Data/Document/JsonTransformer.hpp"
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#include "Nebulite/Data/Document/SimpleValueError.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/TypeCheck.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_DOCUMENT_JSON_HPP
    #include "Nebulite/Data/Document/Json.hpp"
#endif // NEBULITE_DATA_DOCUMENT_JSON_HPP

//------------------------------------------
namespace Nebulite::Data {

template<typename T>
void Json::set(std::string_view const key, T const& val){
    // Check if T is an optional/expected type, and if so, throw an assertion error
    static_assert(!Utility::TypeCheck::isOptional<T>,
        "Setting optional types directly is not allowed. "
        "Please use the value inside the optional instead."
    );
    static_assert(!Utility::TypeCheck::isExpected<T>,
        "Setting expected types directly is not allowed. "
        "Please use the value inside the expected instead."
    );

    // Basically the same as setVariant, but for template types
    std::scoped_lock const lockGuard(mtx);

    if constexpr (std::is_same_v<T, std::string_view>) {
        setVariant(key, RjDirectAccess::SimpleValue(std::string(val)));
    }
    else {
        setVariant(key, RjDirectAccess::SimpleValue(val));
    }
}

template<typename T>
std::expected<T, SimpleValueRetrievalError> Json::get(std::string_view const key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check if a transformation is present
    if (key.contains(SpecialCharacter::transformationPipe)) {
        auto const optionalVal = getWithTransformations<T>(key);
        if (optionalVal.has_value()){
            return optionalVal.value();
        }
        return std::unexpected(optionalVal.error());
    }

    // Get variant and convert to requested type
    auto const var = getVariant(key);
    if(var.has_value()){
        if (auto const converted = RjDirectAccess::convertSimpleValue<T>(var.value()); converted.has_value()) {
            return converted.value();
        }
        return std::unexpected(SimpleValueRetrievalError::conversionFailure);
    }
    return std::unexpected{var.error()};
}

template<typename T>
std::expected<T, SimpleValueRetrievalError> Json::getWithTransformations(std::string_view const key) const {
    auto args = splitKeyWithTransformations(key);
    assert(!args.empty());

    // In order to minimize the re-initialization overhead of an entire JSON document,
    // we use a thread-local temporary JSON document for applying transformations.
    // Then, on each call, we clear the entire document and re-initialize it with the base keys sub-document,
    // which we use as the starting point for transformations.
    // This approach ensures a temporary document with the same value as this JSON object,
    // but without the overhead of creating and destroying a new JSON object on each call.
    thread_local Json tempDoc;
    {
        // Simply overwriting with setSubDoc isn't enough, as this may leave behind stale entries for stable double pointers, which we don't need here.
        // So we manually clear the entire cache.
        auto const& baseKey = args[0];
        tempDoc.cache.clear();
        tempDoc.doc.SetObject();
        tempDoc.setSubDoc("", *this, baseKey); // Make a copy of the required member to transform
    }

    // Apply each transformation in sequence
    if (auto const argsSpan = std::span<std::string_view const>(args.begin()+1, args.end()); !JsonTransformer::instance().parse(argsSpan, tempDoc)) {
        return std::unexpected(SimpleValueRetrievalError::transformationFailure); // if any transformation fails, return default value
    }
    return tempDoc.get<T>(Module::Base::TransformationModule::rootKeyStr);
}

template<typename T>
std::optional<T> Json::jsonValueToCache(std::string_view const key, rapidjson::Value const* val) const {
    // Get supported types
    auto const& v = RjDirectAccess::getSimpleValue(val);
    if(!v.has_value()) {
        return std::nullopt; // Unsupported type, do not cache
    }

    // Insert into cache and return value
    auto newEntry = std::make_unique<CacheEntry>(*cacheLine, cachelineIndex);
    newEntry->setValueClean(v.value());
    auto const value = newEntry->convertTo<T>();
    cache[key] = std::move(newEntry);
    return value;
}

} // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_JSON_TPP
