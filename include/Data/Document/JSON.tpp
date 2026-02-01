#ifndef NEBULITE_DATA_DOCUMENT_JSON_TPP
#define NEBULITE_DATA_DOCUMENT_JSON_TPP

#include "Data/Document/JSON.hpp"
#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Data {

template<typename T>
void JSON::set(std::string const& key, T const& val){
    // Basically the same as setVariant, but for template types
    std::scoped_lock const lockGuard(mtx);
    setVariant(key, RjDirectAccess::simpleValue(val));
}

template<typename T>
T JSON::get(std::string const& key, T const& defaultValue) const {
    std::scoped_lock const lockGuard(mtx);

    // Check if a transformation is present
    if (key.contains('|')){
        if (auto optionalVal = getWithTransformations<T>(key); optionalVal.has_value()){
            return optionalVal.value();
        }
        return defaultValue;
    }

    // Get variant and convert to requested type
    if(auto const var = getVariant(key); var.has_value()){
        return convertVariant<T>(var.value(), defaultValue);
    }
    return defaultValue;
}

// TODO: same for getSubDocWithTransformations!
template<typename T>
std::optional<T> JSON::getWithTransformations(std::string const& key) const {
    auto args = Utility::StringHandler::split(key, '|');
    std::string const baseKey = args[0];
    args.erase(args.begin());

    // Using getSubDoc to properly populate the tempDoc with the rapidjson::Value
    // Slower than a manual copy that handles types, but more secure and less error-prone
    auto tempDoc = getSubDoc(baseKey);

    // Apply each transformation in sequence
    if (!transformer.parse(args, &tempDoc)) {
        return {};    // if any transformation fails, return default value
    }
    // This should not fail, so we use T() as default value
    return tempDoc.get<T>(TransformationModule::rootKeyStr, T());
}

template<typename T>
T JSON::jsonValueToCache(std::string const& key, rapidjson::Value const* val, T const& defaultValue) const {
    // Create a new cache entry
    auto new_entry = std::make_unique<CacheEntry>(CACHELINE, cacheline_index);

    // Get supported types
    auto const& v = RjDirectAccess::getSimpleValue(val);
    if(!v.has_value()) {
        return defaultValue;
    }
    new_entry->value = v.value();

    // Mark as clean
    new_entry->state = CacheEntry::EntryState::CLEAN;

    // Set stable double pointer
    *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value, 0.0);
    new_entry->last_double_value = *new_entry->stable_double_ptr;

    // Insert into cache
    cache[key] = std::move(new_entry);

    // Return converted value
    return convertVariant<T>(cache[key]->value, defaultValue);
}

// Having this flag active used to cause issues on macOS.
// Since then the arguments were renamed, which likely caused compilation errors.
#define ALLOW_STRING_TO_INTEGRAL_CONVERSIONS 1

// Converter helper functions for convertVariant
namespace ConverterHelper {
    inline bool stringToBool(std::string const& stored, bool const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return true;
        if (stored == "false") return false;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        // Handle numeric strings and "true"
        if(Utility::StringHandler::isNumber(stored)){
            try {
                return std::stoi(stored) != 0;
            } catch (...){
                return fallback;
            }
        }
        return stored == "true";
    }

    inline int stringToInt(std::string const& stored, int const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return std::stoi(stored);
        } catch (...){
            return fallback;
        }
    }

    inline uint8_t stringToUInt8(std::string const& stored, uint8_t const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<uint8_t>(std::stoul(stored));
        } catch (...){
            return fallback;
        }
    }

    inline int8_t stringToInt8(std::string const& stored, int8_t const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<int8_t>(std::stol(stored));
        } catch (...){
            return fallback;
        }
    }

    inline uint16_t stringToUInt16(std::string const& stored, uint16_t const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<uint16_t>(std::stoul(stored));
        } catch (...){
            return fallback;
        }
    }

    inline int16_t stringToInt16(std::string const& stored, int16_t const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<int16_t>(std::stol(stored));
        } catch (...){
            return fallback;
        }
    }

    inline uint32_t stringToUInt32(std::string const& stored, uint32_t const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<uint32_t>(std::stoul(stored));
        } catch (...){
            return fallback;
        }
    }

    inline int32_t stringToInt32(std::string const& stored, int32_t const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return static_cast<int32_t>(std::stol(stored));
        } catch (...){
            return fallback;
        }
    }

    inline uint64_t stringToUInt64(std::string const& stored, uint64_t const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return std::stoull(stored);
        } catch (...){
            return fallback;
        }
    }

    inline int64_t stringToInt64(std::string const& stored, int64_t const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1;
        if (stored == "false") return 0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return std::stoll(stored);
        } catch (...){
            return fallback;
        }
    }

    inline double stringToDouble(std::string const& stored, double const& fallback){
#if ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        if (stored == "true") return 1.0;
        if (stored == "false") return 0.0;
#endif // ALLOW_STRING_TO_INTEGRAL_CONVERSIONS
        try {
            return std::stod(stored);
        } catch (...){
            return fallback;
        }
    }
} // namespace ConverterHelper

namespace breakBuild {
template <typename From, typename To>
struct unsupported_conversion;
} // anonymous namespace

// Using NOLINTNEXTLINE to silence "Arguments passed in possible wrong order" warnings
template<typename newType>
newType JSON::convertVariant(RjDirectAccess::simpleValue const& var, newType const& defaultValue){
    return std::visit([&]<typename T>(T const& value)
    {
        // Removing all qualifiers (const, volatile, references, etc.)
        using ValueT = std::decay_t<decltype(value)>;

        // [DOUBLE] -> [BOOL]
        // First, as static_cast doesn't work well for this conversion
        if constexpr (std::is_same_v<ValueT, double> && std::is_same_v<newType, bool>){
            return std::fabs(value) > std::numeric_limits<double>::epsilon();
        }

        // Basic direct conversions
        else if constexpr (std::is_convertible_v<ValueT, newType>){
            return static_cast<newType>(value);
        }

        // [STRING] -> [BOOL]
        // Handle string to bool
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, bool>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToBool(value, defaultValue);
        }

        // [STRING] -> [INT]
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, int>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToInt(value, defaultValue);
        }
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, uint8_t>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToUInt8(value, defaultValue);
        }
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, int8_t>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToInt8(value, defaultValue);
        }
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, uint16_t>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToUInt16(value, defaultValue);
        }
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, int16_t>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToInt16(value, defaultValue);
        }
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, uint32_t>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToUInt32(value, defaultValue);
        }
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, int32_t>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToInt32(value, defaultValue);
        }
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, uint64_t>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToUInt64(value, defaultValue);
        }
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, int64_t>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToInt64(value, defaultValue);
        }

        // [STRING] -> [DOUBLE]
        else if constexpr (std::is_same_v<ValueT, std::string> && std::is_same_v<newType, double>){
            // NOLINTNEXTLINE
            return ConverterHelper::stringToDouble(value, defaultValue);
        }

        // [ARITHMETIC] -> [STRING]
        else if constexpr (std::is_arithmetic_v<ValueT> && std::is_same_v<newType, std::string>){
            return std::to_string(value);
        }

        //------------------------------------------
        // [ERROR] Unsupported conversion
        else {
            breakBuild::unsupported_conversion<ValueT, newType> error;
            (void)error; // to avoid unused variable warning
            return defaultValue; // unreachable, but keeps the compiler happy
        }
    },
    var);
}
}   // namespace Nebulite::Utility
#endif // NEBULITE_DATA_DOCUMENT_JSON_TPP
