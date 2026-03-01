#ifndef NEBULITE_DATA_DOCUMENT_JSON_TPP
#define NEBULITE_DATA_DOCUMENT_JSON_TPP

#include <expected>
#include <type_traits>

#include "Data/Document/JSON.hpp"
#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Data {

template<typename>
struct is_std_optional : std::false_type {};

template<typename U>
struct is_std_optional<std::optional<U>> : std::true_type {};

template<typename T>
inline constexpr bool is_std_optional_v =
    is_std_optional<std::remove_cvref_t<T>>::value;

template<typename>
struct is_std_expected : std::false_type {};

template<typename T, typename E>
struct is_std_expected<std::expected<T, E>> : std::true_type {};

template<typename T>
inline constexpr bool is_std_expected_v =
    is_std_expected<std::remove_cvref_t<T>>::value;

template<typename T>
void JSON::set(std::string const& key, T const& val){
    // Check if T is an optional type, and if so, throw an assertion error
    static_assert(!is_std_optional_v<T>,
        "Setting optional types directly is not allowed. "
        "Please use the value inside the optional instead."
    );
    static_assert(!is_std_expected_v<T>,
        "Setting expected types directly is not allowed. "
        "Please use the value inside the expected instead."
    );

    // Basically the same as setVariant, but for template types
    std::scoped_lock const lockGuard(mtx);
    setVariant(key, RjDirectAccess::simpleValue(val));
}

template<typename T>
std::expected<T, SimpleValueRetrievalError> JSON::get(std::string const& key) const {
    std::scoped_lock const lockGuard(mtx);

    // Check if a transformation is present
    if (key.contains('|')){
        auto const optionalVal = getWithTransformations<T>(key);
        if ( optionalVal.has_value()){
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
        return std::unexpected(CONVERSION_FAILURE);
    }
    return std::unexpected{var.error()};
}

template<typename T>
std::expected<T, SimpleValueRetrievalError> JSON::getWithTransformations(std::string const& key) const {
    auto args = splitKeyWithTransformations(key);

    std::string const baseKey = args[0];
    args.erase(args.begin());

    // In order to minimize the re-initialization overhead of an entire JSON document,
    // we use a thread-local temporary JSON document for applying transformations.
    // Then, on each call, we clear the entire document and re-initialize it with the base key's sub-document,
    // which we use as the starting point for transformations.
    // This approach ensures a temporary document with the same value as this JSON object,
    // but without the overhead of creating and destroying a new JSON object on each call.
    thread_local JSON tempDoc;

    // Simply overwriting with setSubDoc isn't enough, as this may leave behind stale entries for stable double pointers, which we don't need here.
    // So we manually clear the entire cache.
    tempDoc.cache.clear();
    tempDoc.doc.SetObject();
    tempDoc.setSubDoc("", *this, baseKey.c_str()); // Make a copy of the required member to transform

    // Apply each transformation in sequence
    if (!getTransformer()->parse(args, &tempDoc)) {
        return std::unexpected(TRANSFORMATION_FAILURE); // if any transformation fails, return default value
    }
    return tempDoc.get<T>(TransformationModule::rootKeyStr);
}

template<typename T>
std::optional<T> JSON::jsonValueToCache(std::string const& key, rapidjson::Value const* val) const {
    // Create a new cache entry
    auto new_entry = std::make_unique<CacheEntry>(CACHELINE, cacheline_index);

    // Get supported types
    auto const& v = RjDirectAccess::getSimpleValue(val);
    if(!v.has_value()) {
        return std::nullopt; // Unsupported type, do not cache
    }
    new_entry->value = v.value();

    // Mark as clean
    new_entry->state = CacheEntry::EntryState::CLEAN;

    // Set stable double pointer
    *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value).value_or(0.0); // Default to 0.0 if conversion fails
    new_entry->last_double_value = *new_entry->stable_double_ptr;

    // Insert into cache
    cache[key] = std::move(new_entry);

    // Return converted value
    return convertVariant<T>(cache[key]->value);
}

// Having this flag active used to cause issues on macOS.
// Since then the arguments were renamed, which likely caused compilation errors.
#define ALLOW_STRING_TO_INTEGRAL_CONVERSIONS 1

// Converter helper functions for convertVariant
namespace ConverterHelper {
    inline std::optional<bool> stringToBool(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return true;
        if (stored == "false") return false;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        // Handle numeric strings and "true"
        if(Utility::StringHandler::isNumber(stored)){
            try {
                return std::stoi(stored) != 0;
            } catch (...){
                return std::nullopt;
            }
        }
        return stored == "true";
    }

    inline std::optional<int> stringToInt(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return std::stoi(stored);
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<uint8_t> stringToUInt8(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<uint8_t>(std::stoul(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<int8_t> stringToInt8(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<int8_t>(std::stol(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<uint16_t> stringToUInt16(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<uint16_t>(std::stoul(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<int16_t> stringToInt16(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<int16_t>(std::stol(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<uint32_t> stringToUInt32(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<uint32_t>(std::stoul(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<int32_t> stringToInt32(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<int32_t>(std::stol(stored));
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<uint64_t> stringToUInt64(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return std::stoull(stored);
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<int64_t> stringToInt64(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return std::stoll(stored);
        } catch (...){
            return std::nullopt;
        }
    }

    inline std::optional<double> stringToDouble(std::string const& stored){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1.0;
        if (stored == "false") return 0.0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return std::stod(stored);
        } catch (...){
            return std::nullopt;
        }
    }
} // namespace ConverterHelper

// Using NOLINTNEXTLINE to silence "Arguments passed in possible wrong order" warnings
template<typename newType>
std::optional<newType> JSON::convertVariant(RjDirectAccess::simpleValue const& var){
    return std::visit([&]<typename T>(T const& value)
    {
        // Removing all qualifiers (const, volatile, references, etc.)
        using ValueT = std::decay_t<decltype(value)>;

        // [DOUBLE] -> [BOOL]
        // First, as the static_cast from a direct conversion doesn't work well here
        if constexpr (std::is_same_v<ValueT, double> && std::is_same_v<newType, bool>){
            return std::optional<newType>{std::fabs(value) > std::numeric_limits<double>::epsilon()};
        }

        // Basic direct conversions
        else if constexpr (std::is_convertible_v<ValueT, newType>){
            return std::optional<newType>{static_cast<newType>(value)};
        }

        // [STRING] -> [VALUE]
        else if constexpr (std::is_same_v<ValueT, std::string>) {
            if constexpr (std::is_same_v<newType, bool>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToBool(value);
            }
            else if constexpr (std::is_same_v<newType, int>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToInt(value);
            }
            else if constexpr (std::is_same_v<newType, uint8_t>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToUInt8(value);
            }
            else if constexpr (std::is_same_v<newType, int8_t>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToInt8(value);
            }
            else if constexpr (std::is_same_v<newType, uint16_t>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToUInt16(value);
            }
            else if constexpr (std::is_same_v<newType, int16_t>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToInt16(value);
            }
            else if constexpr (std::is_same_v<newType, uint32_t>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToUInt32(value);
            }
            else if constexpr (std::is_same_v<newType, int32_t>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToInt32(value);
            }
            else if constexpr (std::is_same_v<newType, uint64_t>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToUInt64(value);
            }
            else if constexpr (std::is_same_v<newType, int64_t>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToInt64(value);
            }
            else if constexpr (std::is_same_v<newType, double>){
                // NOLINTNEXTLINE
                return ConverterHelper::stringToDouble(value);
            }
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
}   // namespace Nebulite::Utility
#endif // NEBULITE_DATA_DOCUMENT_JSON_TPP
