#ifndef NEBULITE_DATA_DOCUMENT_JSON_TPP
#define NEBULITE_DATA_DOCUMENT_JSON_TPP

#include "Data/Document/JSON.hpp"

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
    return tempDoc.get<T>(JsonRvalueTransformer::valueKeyStr, T());
}

template<typename T>
T JSON::jsonValueToCache(std::string const& key, rapidjson::Value const* val, T const& defaultValue) const {
    // Create a new cache entry
    auto new_entry = std::make_unique<CacheEntry>();

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

// Converter helper functions for convertVariant
namespace ConverterHelper {
    inline bool stringToBool(std::string const& stored, bool const& defaultValue){
        // Handle numeric strings and "true"
        if(Utility::StringHandler::isNumber(stored)){
            try {
                return std::stoi(stored) != 0;
            } catch (...){
                return defaultValue;
            }
        }
        return stored == "true";
    }

    inline int stringToInt(std::string const& stored, int const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return std::stoi(stored);
        } catch (...){
            return defaultValue;
        }
    }

    inline uint8_t stringToUInt8(std::string const& stored, uint8_t const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<uint8_t>(std::stoul(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline int8_t stringToInt8(std::string const& stored, int8_t const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<int8_t>(std::stol(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline uint16_t stringToUInt16(std::string const& stored, uint16_t const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<uint16_t>(std::stoul(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline int16_t stringToInt16(std::string const& stored, int16_t const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<int16_t>(std::stol(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline uint32_t stringToUInt32(std::string const& stored, uint32_t const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<uint32_t>(std::stoul(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline int32_t stringToInt32(std::string const& stored, int32_t const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<int32_t>(std::stol(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline uint64_t stringToUInt64(std::string const& stored, uint64_t const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<uint64_t>(std::stoull(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline int64_t stringToInt64(std::string const& stored, int64_t const& defaultValue){
        //if (stored == "true") return 1;
        //if (stored == "false") return 0;
        try {
            return static_cast<int64_t>(std::stoll(stored));
        } catch (...){
            return defaultValue;
        }
    }

    inline double stringToDouble(std::string const& stored, double const& defaultValue){
        //if (stored == "true") return 1.0;
        //if (stored == "false") return 0.0;
        try {
            return std::stod(stored);
        } catch (...){
            return defaultValue;
        }
    }

    inline void convertVariantErrorMessage(std::string const& oldType, std::string const& newType){
        std::string const message = "[ERROR] JSON::convert_variant - Unsupported conversion from "
                    + oldType
                    + " to " + newType + ".\n"
                    + "Please add the required conversion.\n"
                    + "Fallback conversion from String to any Integral type was disabled due to potential lossy data conversion.\n"
                    + "Rather, it is recommended to add one explicit conversion path per datatype.\n"
                    + "Returning default value.";
        Nebulite::Utility::Capture::cerr() << message << Nebulite::Utility::Capture::endl;
        // Exiting the program would be nice, but since this is likely run in a threaded environment, we just display the error.
    }
} // namespace ConverterHelper

namespace breakBuild {
template <typename From, typename To>
struct unsupported_conversion;
} // anonymous namespace

template<typename newType>
newType JSON::convertVariant(RjDirectAccess::simpleValue const& var, newType const& defaultValue){
    return std::visit([&]<typename T>(T const& stored)
    {
        // Removing all qualifiers (const, volatile, references, etc.)
        using StoredT = std::decay_t<decltype(stored)>;

        // [DOUBLE] -> [BOOL]
        // First, as static_cast doesn't work well for this conversion
        if constexpr (std::is_same_v<StoredT, double> && std::is_same_v<newType, bool>){
            return std::fabs(stored) > std::numeric_limits<double>::epsilon();
        }

        // Basic direct conversions
        else if constexpr (std::is_convertible_v<StoredT, newType>){
            return static_cast<newType>(stored);
        }

        // [STRING] -> [BOOL]
        // Handle string to bool
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, bool>){
            return ConverterHelper::stringToBool(stored, defaultValue);
        }

        // [STRING] -> [INT]
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, int>){
            return ConverterHelper::stringToInt(stored, defaultValue);
        }
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, uint8_t>){
            return ConverterHelper::stringToUInt8(stored, defaultValue);
        }
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, int8_t>){
            return ConverterHelper::stringToInt8(stored, defaultValue);
        }
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, uint16_t>){
            return ConverterHelper::stringToUInt16(stored, defaultValue);
        }
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, int16_t>){
            return ConverterHelper::stringToInt16(stored, defaultValue);
        }
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, uint32_t>){
            return ConverterHelper::stringToUInt32(stored, defaultValue);
        }
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, int32_t>){
            return ConverterHelper::stringToInt32(stored, defaultValue);
        }
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, uint64_t>){
            return ConverterHelper::stringToUInt64(stored, defaultValue);
        }
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, int64_t>){
            return ConverterHelper::stringToInt64(stored, defaultValue);
        }

        // [STRING] -> [DOUBLE]
        else if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, double>){
            return ConverterHelper::stringToDouble(stored, defaultValue);
        }

        // [ARITHMETIC] -> [STRING]
        else if constexpr (std::is_arithmetic_v<StoredT> && std::is_same_v<newType, std::string>){
            return std::to_string(stored);
        }

        //------------------------------------------
        // [ERROR] Unsupported conversion
        else {
            breakBuild::unsupported_conversion<StoredT, newType> error;
            (void)error; // to avoid unused variable warning
            return defaultValue; // unreachable, but keeps the compiler happy

            // Old runtime error message
            //std::string constexpr oldTypeName = abi::__cxa_demangle(typeid(stored).name(), nullptr, nullptr, nullptr);
            //std::string constexpr newTypeName = abi::__cxa_demangle(typeid(newType).name(), nullptr, nullptr, nullptr);
            //ConverterHelper::convertVariantErrorMessage(oldTypeName, newTypeName);
            //return defaultValue;
        }
    },
    var);
}
}   // namespace Nebulite::Utility
#endif // NEBULITE_DATA_DOCUMENT_JSON_TPP
