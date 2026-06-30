#ifndef DATA_DOCUMENT_JSON_TPP
#define DATA_DOCUMENT_JSON_TPP


//------------------------------------------
// Includes

// Standard library
#include <expected>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// External
#include <rapidjson/document.h>

// Nebulite
#include "Data/Document/JsonRvalueTransformer.hpp"
#include "Data/Document/RjDirectAccess.hpp"
#include "Data/Document/SimpleValueError.hpp"
#include "Data/Document/TypeConversion.hpp"
#include "Module/Base/TransformationModule.hpp"
#include "Utility/TypeCheck.hpp"

//------------------------------------------
// Conditional includes

#ifndef DATA_DOCUMENT_JSON_HPP
    #include "Data/Document/JSON.hpp"
#endif // DATA_DOCUMENT_JSON_HPP

//------------------------------------------
namespace Nebulite::Data {

template<typename T>
void JSON::set(std::string_view const key, T const& val){
    // Check if T is an optional/expected type, and if so, throw an assertion error
    static_assert(!Utility::TypeCheck::is_std_optional_v<T>,
        "Setting optional types directly is not allowed. "
        "Please use the value inside the optional instead."
    );
    static_assert(!Utility::TypeCheck::is_std_expected_v<T>,
        "Setting expected types directly is not allowed. "
        "Please use the value inside the expected instead."
    );

    // Basically the same as setVariant, but for template types
    std::scoped_lock const lockGuard(mtx);

    if constexpr (std::is_same_v<T, std::string_view>) {
        setVariant(key, RjDirectAccess::simpleValue(std::string(val)));
    }
    else {
        setVariant(key, RjDirectAccess::simpleValue(val));
    }
}

template<typename T>
std::expected<T, SimpleValueRetrievalError> JSON::get(std::string_view const key) const {
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
        if (auto const converted = convertVariant<T>(var.value()); converted.has_value()) {
            return converted.value();
        }
        return std::unexpected(SimpleValueRetrievalError::CONVERSION_FAILURE);
    }
    return std::unexpected{var.error()};
}

template<typename T>
std::expected<T, SimpleValueRetrievalError> JSON::getWithTransformations(std::string_view const key) const {
    auto args = splitKeyWithTransformations(key);

    // In order to minimize the re-initialization overhead of an entire JSON document,
    // we use a thread-local temporary JSON document for applying transformations.
    // Then, on each call, we clear the entire document and re-initialize it with the base keys sub-document,
    // which we use as the starting point for transformations.
    // This approach ensures a temporary document with the same value as this JSON object,
    // but without the overhead of creating and destroying a new JSON object on each call.
    thread_local JSON tempDoc;
    {
        // Simply overwriting with setSubDoc isn't enough, as this may leave behind stale entries for stable double pointers, which we don't need here.
        // So we manually clear the entire cache.
        auto const& baseKey = args[0];
        tempDoc.cache.clear();
        tempDoc.doc.SetObject();
        tempDoc.setSubDoc("", *this, baseKey); // Make a copy of the required member to transform
    }

    // Apply each transformation in sequence
    args.erase(args.begin());
    if (!JsonRvalueTransformer::instance().parse(args, &tempDoc)) {
        return std::unexpected(SimpleValueRetrievalError::TRANSFORMATION_FAILURE); // if any transformation fails, return default value
    }
    return tempDoc.get<T>(Module::Base::TransformationModule::rootKeyStr);
}

template<typename T>
std::optional<T> JSON::jsonValueToCache(std::string_view const key, rapidjson::Value const* val) const {
    // Create a new cache entry
    auto new_entry = std::make_unique<CacheEntry>(*cacheLine, cacheline_index);

    // Get supported types
    auto const& v = RjDirectAccess::getSimpleValue(val);
    if(!v.has_value()) {
        return std::nullopt; // Unsupported type, do not cache
    }
    new_entry->value = v.value();

    // Mark as clean
    new_entry->state = CacheEntry::EntryState::CLEAN;

    // Set stable double pointer
    *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value).value_or(standardNumericValue); // Default to NAN if conversion fails
    new_entry->last_double_value = *new_entry->stable_double_ptr;

    // Insert into cache
    auto const value = convertVariant<T>(new_entry->value);
    cache[key] = std::move(new_entry);

    // Return converted value
    return value;
}

// Using NOLINTNEXTLINE to silence "Arguments passed in possible wrong order" warnings
template<typename newType>
std::optional<newType> JSON::convertVariant(RjDirectAccess::simpleValue const& var){
    return std::visit([&]<typename T>(T const& value){
        // Removing all qualifiers (const, volatile, references, etc.)
        using ValueT = std::decay_t<decltype(value)>;

        //------------------------------------------
        // To float is seen as special case, as we do not store floats.
        // If newType is float, get double first and convert to float
        if constexpr(std::is_same_v<newType, float>) {
            if (auto const val = convertVariant<double>(var); val.has_value()) {
                return std::optional<newType>(static_cast<float>(val.value()));
            }
            return std::optional<newType>(std::nullopt);
        }

        //------------------------------------------
        // Try some special conversions first

        // [BOOL] -> [STRING]
        else if constexpr(std::is_same_v<ValueT, bool> && std::is_same_v<newType, std::string>) {
            return TypeConversion::Bool::toString(value);
        }

        // [DOUBLE] -> [BOOL]
        // First, as the static_cast from a direct conversion doesn't work well here
        else if constexpr (std::is_same_v<ValueT, double> && std::is_same_v<newType, bool>){
            return TypeConversion::Double::toBool(value);
        }

        // [STRING] -> [ANY]
        else if constexpr (std::is_same_v<ValueT, std::string>) {
            return TypeConversion::String::toAny<newType>(value);
        }

        //------------------------------------------
        // Try basic direct conversions

        // [ANY] -> [ANY] via static_cast
        else if constexpr (std::is_convertible_v<ValueT, newType>){
            return std::optional<newType>{static_cast<newType>(value)};
        }
        // [ARITHMETIC] -> [STRING]
        else if constexpr (std::is_arithmetic_v<ValueT> && std::is_same_v<newType, std::string>){
            return std::optional<newType>{std::to_string(value)};
        }

        //------------------------------------------
        // [ERROR] Unsupported conversion
        else {
            std::unreachable();
        }
    },
    var);
}
} // namespace Nebulite::Data
#endif // DATA_DOCUMENT_JSON_TPP
