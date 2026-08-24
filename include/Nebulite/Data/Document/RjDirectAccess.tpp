#ifndef NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_TPP
#define NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_TPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint> // NOLINT
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

// External
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>

// Nebulite
#include "Nebulite/Utility/Convert/Cast.hpp"

//------------------------------------------
// Conditional includes

#ifndef NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_HPP
#include "Nebulite/Data/Document/RjDirectAccess.hpp"
#endif // NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_HPP

//------------------------------------------
namespace Nebulite::Data::RjDirectAccess {

template <typename NewType>
std::optional<NewType> convertSimpleValue(SimpleValue const& simpleValue) {
    return std::visit([&]<typename T>(T const& visitValue){
        // Removing all qualifiers (const, volatile, references, etc.)
        using ValueT = std::decay_t<decltype(visitValue)>;

        //------------------------------------------
        // To float is seen as special case, as we do not store floats.
        // If NewType is float, get double first and convert to float
        if constexpr(std::is_same_v<NewType, float>) {
            if (auto const val = convertSimpleValue<double>(visitValue); val.has_value()) {
                return std::optional<NewType>(static_cast<float>(val.value()));
            }
            return std::optional<NewType>(std::nullopt);
        }

        //------------------------------------------
        // Try some special conversions first

        // [BOOL] -> [STRING]
        else if constexpr(std::is_same_v<ValueT, bool> && std::is_same_v<NewType, std::string>) {
            return Utility::Convert::Cast::Bool::to<std::string>(visitValue);
        }

        // [DOUBLE] -> [BOOL]
        // First, as the static_cast from a direct conversion doesn't work well here
        else if constexpr (std::is_same_v<ValueT, double> && std::is_same_v<NewType, bool>){
            return Utility::Convert::Cast::Double::to<bool>(visitValue);
        }

        // [STRING] -> [ANY]
        else if constexpr (std::is_same_v<ValueT, std::string>) {
            return Utility::Convert::Cast::String::to<NewType>(visitValue);
        }

        //------------------------------------------
        // Try basic direct conversions

        // [ANY] -> [ANY] via static_cast
        else if constexpr (std::is_convertible_v<ValueT, NewType>){
            return std::optional<NewType>{static_cast<NewType>(visitValue)};
        }

        // [ARITHMETIC] -> [STRING]
        else if constexpr (std::is_arithmetic_v<ValueT> && std::is_same_v<NewType, std::string>){
            return std::optional<NewType>{std::to_string(visitValue)};
        }

        //------------------------------------------
        // [ERROR] Unsupported conversion
        else {
            std::unreachable();
        }
    },
    simpleValue);
}

template<typename RjValType>
std::optional<SimpleValue> getSimpleValue(std::string_view key, RjValType& doc) {
  // The given RjValType should be a Document.
  // If we pass a rapidjson value, we risk not starting at the top of the document, where we should apply the key traversal.
  // This fixes implicit conversion worries.
  static_assert(
      std::is_same_v<RjValType, rapidjson::Document>,
      "The given Rapidjson Value type should be a document to ensure the key traversal happens at the top! "
      "Passing, for example, a rapidjson::Value would risk starting the traversal at the wrong point in the document, which could lead to incorrect retrieval or failure to find the value."
  );

  if (auto const rjVal = traversePath(key, doc); rjVal != nullptr) {
    return getSimpleValue(rjVal);
  }
  return std::nullopt;
}

//------------------------------------------
// Direct access get/set

template <typename T>
T get(char const* key, T const& defaultValue, rapidjson::Value& val) {
    rapidjson::Value const* keyVal = traversePath(key, val);
    if (keyVal == nullptr) {
        // Value doesn't exist in doc, return default
        return defaultValue;
    }
    // Base case: convert currentVal to T using JSONHandler
    T tmp;
    convertFromJsonValue<T>(*keyVal, tmp, defaultValue);
    return tmp;
}

template <typename T>
bool set(char const* key, T const& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
    // Ensure key path exists
    if (rapidjson::Value* keyVal = ensurePath(key, val, allocator); keyVal != nullptr) {
        convertToJsonValue<T>(value, *keyVal, allocator);
        return true;
    }
    return false;
}

//------------------------------------------
// All conversion variants from/to rapidjson values

//------------------------------------------
// 1.) to JSON value
//------------------------------------------

template <> inline void convertToJsonValue<bool>(bool const data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& /*allocator*/) {
    jsonValue.SetBool(data);
}

template <> inline void convertToJsonValue<int>(int const data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& /*allocator*/) {
    jsonValue.SetInt(data);
}

template <> inline void convertToJsonValue<std::uint32_t>(std::uint32_t const data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& /*allocator*/) {
    jsonValue.SetUint(data);
}

template <> inline void convertToJsonValue<std::uint64_t>(std::uint64_t const data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& /*allocator*/) {
    jsonValue.SetUint64(data);
}

template <> inline void convertToJsonValue<double>(double const data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& /*allocator*/) {
    jsonValue.SetDouble(data);
}

template <> inline void convertToJsonValue<std::int64_t>(std::int64_t const data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& /*allocator*/) {
    jsonValue.SetInt64(data);
}

template <> inline void convertToJsonValue<std::string>(std::string const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.SetString(
        data.c_str(),
        static_cast<rapidjson::SizeType>(data.length()),
        allocator
    );
}

template <> inline void convertToJsonValue<char const*>(char const* data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data != nullptr) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void convertToJsonValue<char*>(char* data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data != nullptr) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void convertToJsonValue<rapidjson::Value*>(rapidjson::Value* data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.CopyFrom(*data, allocator);
}

template <> inline void convertToJsonValue<rapidjson::Document*>(rapidjson::Document* data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.CopyFrom(*data, allocator);
}

template <> inline void convertToJsonValue<rapidjson::Document>(rapidjson::Document const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.CopyFrom(data, allocator);
}

// Template specialization for std::variant
// So we don't have to manually call std::visit every time
template <> inline void convertToJsonValue(SimpleValue const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    std::visit([&]<typename T>(T const& value) {
        using Decayed = std::decay_t<T>;
        convertToJsonValue<Decayed>(value, jsonValue, allocator);
    }, data);
}

//------------------------------------------
// 2.) from JSON Value
//------------------------------------------

template <> inline void convertFromJsonValue(rapidjson::Value const& jsonValue, bool& result, bool const defaultValue) {
    if (jsonValue.IsBool()) {
        result = jsonValue.GetBool();
    } else {
        result = defaultValue;
    }
}

template <> inline void convertFromJsonValue(rapidjson::Value const& jsonValue, int& result, int const defaultValue) {
    if (jsonValue.IsInt()) {
        result = jsonValue.GetInt();
    } else if (jsonValue.IsBool()) {
        result = static_cast<int>(jsonValue.GetBool());
    } else {
        result = defaultValue;
    }
}

template <> inline void convertFromJsonValue(rapidjson::Value const& jsonValue, std::uint32_t& result, std::uint32_t const defaultValue) {
    if (jsonValue.IsUint()) {
        result = jsonValue.GetUint();
    } else if (jsonValue.IsNumber()) {
        if (int const tmp = jsonValue.GetInt(); tmp >= 0) {
            result = static_cast<std::uint32_t>(tmp);
        }
    } else if (jsonValue.IsString()) {
        std::istringstream iss(jsonValue.GetString());
        iss >> result;
    } else {
        result = defaultValue;
    }
}

template <> inline void convertFromJsonValue(rapidjson::Value const& jsonValue, std::uint64_t& result, std::uint64_t const defaultValue) {
    if (jsonValue.IsString()) {
        std::istringstream iss(jsonValue.GetString());
        iss >> result;
    } else if (jsonValue.IsUint64()) {
        result = jsonValue.GetUint64();
    } else if (jsonValue.IsUint()) {
        result = static_cast<std::uint64_t>(jsonValue.GetUint());
    } else if (jsonValue.IsNumber()) {
        if (std::int64_t const tmp = jsonValue.GetInt64(); tmp >= 0) {
            result = static_cast<std::uint64_t>(tmp);
        } else {
            result = defaultValue;
        }
    } else {
        result = defaultValue;
    }
}

template <> inline void convertFromJsonValue(rapidjson::Value const& jsonValue, double& result, double const defaultValue) {
    if (jsonValue.IsNumber()) {
        result = jsonValue.GetDouble();
    } else if (jsonValue.IsString()) {
        if (auto const converted = Utility::Convert::Cast::String::to<double>(jsonValue.GetString()); converted.has_value()) {
            result = converted.value();
        } else {
            result = defaultValue;
        }
    } else {
        result = defaultValue;
    }
}

template <> inline void convertFromJsonValue(rapidjson::Value const& jsonValue, std::string& result, std::string const& defaultValue) {
    if (jsonValue.IsBool()) {
        result = jsonValue.GetBool() ? "true" : "false";
    } else if (jsonValue.IsString()) {
        result = std::string(jsonValue.GetString());
    } else if (jsonValue.IsInt()) {
        result = std::to_string(jsonValue.GetInt());
    } else if (jsonValue.IsUint()) {
        result = std::to_string(jsonValue.GetUint());
    } else if (jsonValue.IsInt64()) {
        result = std::to_string(jsonValue.GetInt64());
    } else if (jsonValue.IsUint64()) {
        result = std::to_string(jsonValue.GetUint64());
    } else if (jsonValue.IsDouble()) {
        result = std::to_string(jsonValue.GetDouble());
    } else if (jsonValue.IsNull()) {
        result = "null";
    } else if (jsonValue.IsArray()) {
        result = "{Array}";
    } else if (jsonValue.IsObject()) {
        result = "{Object}"; // Just a placeholder since objects can't easily be converted to a single string
    } else {
        result = defaultValue;
    }
}

} // namespace Nebulite::Data::RjDirectAccess
#endif // NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_TPP
