#ifndef NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_TPP
#define NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_TPP

//------------------------------------------
// Includes

// Standard library
#include <sstream>

// Nebulite
#include "Data/Document/RjDirectAccess.hpp"
#include "Utility/Capture.hpp"

//------------------------------------------
namespace Nebulite::Data {

//------------------------------------------
// Direct access get/set

template <typename T>
T RjDirectAccess::get(char const* key, T const& defaultValue, rapidjson::Value& val) {
    rapidjson::Value const* keyVal = traversePath(key, val);
    if (keyVal == nullptr) {
        // Value doesn't exist in doc, return default
        return defaultValue;
    }
    // Base case: convert currentVal to T using JSONHandler
    T tmp;
    RjDirectAccess::ConvertFromJSONValue<T>(*keyVal, tmp, defaultValue);
    return tmp;
}

template <typename T>
bool RjDirectAccess::set(char const* key, T const& value, rapidjson::Value& val, rapidjson::Document::AllocatorType& allocator) {
    // Ensure key path exists
    if (rapidjson::Value* keyVal = ensurePath(key, val, allocator); keyVal != nullptr) {
        RjDirectAccess::ConvertToJSONValue<T>(value, *keyVal, allocator);
        return true;
    }
    return false;
}

//------------------------------------------
// All conversion variants from/to rapidjson values

//------------------------------------------
// 1.) to JSON value
//------------------------------------------

// cppcheck-suppress constParameterReference
template <> inline void RjDirectAccess::ConvertToJSONValue<bool>(bool const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.SetBool(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void RjDirectAccess::ConvertToJSONValue<int>(int const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.SetInt(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void RjDirectAccess::ConvertToJSONValue<uint32_t>(uint32_t const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.SetUint(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void RjDirectAccess::ConvertToJSONValue<uint64_t>(uint64_t const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.SetUint64(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void RjDirectAccess::ConvertToJSONValue<double>(double const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.SetDouble(data);
    (void)allocator; // Suppress unused parameter warning
}

// cppcheck-suppress constParameterReference
template <> inline void RjDirectAccess::ConvertToJSONValue<int64_t>(int64_t const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.SetInt64(data);
    (void)allocator; // Suppress unused parameter warning
}

template <> inline void RjDirectAccess::ConvertToJSONValue<std::string>(std::string const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    jsonValue.SetString(
        data.c_str(),
        static_cast<rapidjson::SizeType>(data.length()), allocator
        );
}

template <> inline void RjDirectAccess::ConvertToJSONValue<char const*>(char const* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data != nullptr) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void RjDirectAccess::ConvertToJSONValue<char*>(char* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    if (data != nullptr) {
        jsonValue.SetString(data, allocator);
    } else {
        jsonValue.SetNull();
    }
}

template <> inline void RjDirectAccess::ConvertToJSONValue<rapidjson::Value*>(rapidjson::Value* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) { jsonValue.CopyFrom(*data, allocator); }

template <> inline void RjDirectAccess::ConvertToJSONValue<rapidjson::Document*>(rapidjson::Document* const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) { jsonValue.CopyFrom(*data, allocator); }

template <> inline void RjDirectAccess::ConvertToJSONValue<rapidjson::Document>(rapidjson::Document const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) { jsonValue.CopyFrom(data, allocator); }

// Template specialization for std::variant
// So we don't have to manually call std::visit every time
template <> inline void RjDirectAccess::ConvertToJSONValue(simpleValue const& data, rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) {
    std::visit([&]<typename T>(T const& value) {
        using Decayed = std::decay_t<T>;
        ConvertToJSONValue<Decayed>(value, jsonValue, allocator);
    }, data);
}

//------------------------------------------
// 2.) from JSON Value
//------------------------------------------

template <> inline void RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, bool& result, bool const& defaultValue) {
    if (jsonValue.IsBool()) {
        result = jsonValue.GetBool();
    } else {
        result = defaultValue;
    }
}

template <> inline void RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, int& result, int const& defaultValue) {
    if (jsonValue.IsInt()) {
        result = jsonValue.GetInt();
    } else if (jsonValue.IsBool()) {
        result = static_cast<int>(jsonValue.GetBool());
    } else {
        result = defaultValue;
    }
}

template <> inline void RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, uint32_t& result, uint32_t const& defaultValue) {
    if (jsonValue.IsUint()) {
        result = jsonValue.GetUint();
    } else if (jsonValue.IsNumber()) {
        if (int const tmp = jsonValue.GetInt(); tmp >= 0) {
            result = static_cast<uint32_t>(tmp);
        }
    } else if (jsonValue.IsString()) {
        std::istringstream iss(jsonValue.GetString());
        iss >> result;
    } else {
        result = defaultValue;
    }
}

template <> inline void RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, uint64_t& result, uint64_t const& defaultValue) {
    if (jsonValue.IsString()) {
        std::istringstream iss(jsonValue.GetString());
        iss >> result;
    } else if (jsonValue.IsUint64()) {
        result = jsonValue.GetUint64();
    } else if (jsonValue.IsUint()) {
        result = static_cast<uint64_t>(jsonValue.GetUint());
    } else if (jsonValue.IsNumber()) {
        if (int64_t const tmp = jsonValue.GetInt64(); tmp >= 0) {
            result = static_cast<uint64_t>(tmp);
        } else {
            result = defaultValue;
        }
    } else {
        result = defaultValue;
    }
}

template <> inline void RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, double& result, double const& defaultValue) {
    if (jsonValue.IsNumber()) {
        result = jsonValue.GetDouble();
    } else if (jsonValue.IsString()) {
        if (Utility::StringHandler::isNumber(jsonValue.GetString())) {
            result = std::stod(jsonValue.GetString());
        } else {
            result = defaultValue;
        }
    } else {
        result = defaultValue;
    }
}

template <> inline void RjDirectAccess::ConvertFromJSONValue(rapidjson::Value const& jsonValue, std::string& result, std::string const& defaultValue) {
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

}   // namespace Nebulite::Data
#endif // NEBULITE_DATA_DOCUMENT_RJDIRECTACCESS_TPP
