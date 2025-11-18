#include "Utility/JSON.hpp"

namespace Nebulite::Utility {

template<typename T>
void JSON::set(std::string const& key, T const& val){
    std::scoped_lock const lockGuard(mtx);

    // Check if key is valid
    if (!RjDirectAccess::isValidKey(key)){
        Nebulite::Utility::Capture::cerr() << "Invalid key: " << key << Nebulite::Utility::Capture::endl;
        return;
    }

    if (auto const it = cache.find(key); it != cache.end()){
        // Existing cache value, structure validity guaranteed

        // Update the entry, mark as dirty
        it->second->value = val;
        it->second->state = EntryState::DIRTY;

        // Update double pointer value
        *it->second->stable_double_ptr = convertVariant<double>(val);
        it->second->last_double_value = *it->second->stable_double_ptr;
    } else {
        // New cache value, structural validity is not guaranteed

        // Remove any child keys to synchronize the structure
        invalidate_child_keys(key);

        // Create new entry directly in DIRTY state
        auto new_entry = std::make_unique<CacheEntry>();

        // Set entry values
        new_entry->value = val;
        // Pointer was created in constructor, no need to redo make_shared
        *new_entry->stable_double_ptr = convertVariant<double>(new_entry->value, 0.0);
        new_entry->last_double_value = *new_entry->stable_double_ptr;
        new_entry->state = EntryState::DIRTY;

        // Insert into cache
        cache[key] = std::move(new_entry);

        // Flush to RapidJSON document for structural integrity
        flush();
    }
}

template<typename T>
T JSON::get(std::string const& key, T const& defaultValue){
    std::scoped_lock const lockGuard(mtx);
    // Check cache first
    auto const it = cache.find(key);
    if (it != cache.end() && it->second->state != EntryState::DELETED){
        // Entry exists and is not deleted

        // Check its double value for change detection using an epsilon to avoid unsafe direct comparison
        {
            if(std::fabs(*it->second->stable_double_ptr - it->second->last_double_value) > DBL_EPSILON){
                // Value changed since last check
                // We update the actual value with the new double value
                // Then we convert the double to the requested type
                it->second->last_double_value = *it->second->stable_double_ptr;
                it->second->value = it->second->last_double_value;
                it->second->state = EntryState::DIRTY; // Mark as dirty to sync back
                return convertVariant<T>(it->second->value, defaultValue);
            }
        }
        return convertVariant<T>(it->second->value, defaultValue);
    }

    // Check document, if not in cache
    if(rapidjson::Value const* val = RjDirectAccess::traverse_path(key.c_str(), doc); val != nullptr){
        if(it != cache.end()){
            // Modify existing entry
            if(!RjDirectAccess::getSimpleValue(&it->second->value, val)){
                return defaultValue;
            }

            // Mark as clean
            it->second->state = EntryState::CLEAN;

            // Set stable double pointer
            *it->second->stable_double_ptr = convertVariant<double>(it->second->value, 0.0);
            it->second->last_double_value = *it->second->stable_double_ptr;

            // Return converted value
            return convertVariant<T>(it->second->value, defaultValue);
        }
        // Create new cache entry
        return jsonValueToCache<T>(key, val, defaultValue);
    }

    // Value could not be created, return default
    return defaultValue;
}

template<typename T>
T JSON::jsonValueToCache(std::string const& key, rapidjson::Value const* val, T const& defaultValue){
    // Create a new cache entry
    auto new_entry = std::make_unique<CacheEntry>();

    // Get supported types
    if(!RjDirectAccess::getSimpleValue(&new_entry->value, val)){
        return defaultValue;
    }

    // Mark as clean
    new_entry->state = EntryState::CLEAN;

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
        if(StringHandler::isNumber(stored)){
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

template<typename newType>
newType JSON::convertVariant(RjDirectAccess::simpleValue const& var, newType const& defaultValue){
    return std::visit([&]<typename T>(T const& stored)
    {
        // Removing all qualifiers (const, volatile, references, etc.)
        using StoredT = std::decay_t<decltype(stored)>;

        // [DIRECT]
        // Directly cast if types are convertible
        if constexpr (std::is_convertible_v<StoredT, newType>){
            return static_cast<newType>(stored);
        }

        // [STRING] -> [BOOL]
        // Handle string to bool
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, bool>){
            return ConverterHelper::stringToBool(stored, defaultValue);
        }

        // [STRING] -> [DOUBLE]
        if constexpr (std::is_same_v<StoredT, std::string> && std::is_same_v<newType, double>){
            return ConverterHelper::stringToDouble(stored, defaultValue);
        }


        // [ARITHMETIC] -> [STRING]
        if constexpr (std::is_arithmetic_v<StoredT> && std::is_same_v<newType, std::string>){
            return std::to_string(stored);
        }

        //------------------------------------------
        // [ERROR] Unsupported conversion
        std::string const oldTypeName = abi::__cxa_demangle(typeid(stored).name(), nullptr, nullptr, nullptr);
        std::string const newTypeName = abi::__cxa_demangle(typeid(newType).name(), nullptr, nullptr, nullptr);
        ConverterHelper::convertVariantErrorMessage(oldTypeName, newTypeName);
        return defaultValue;
    },
    var);
}
}   // namespace Nebulite::Utility