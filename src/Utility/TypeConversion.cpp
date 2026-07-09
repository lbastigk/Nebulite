//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>

// Nebulite
#include "Nebulite/Utility/StringHandler.hpp"
#include "Nebulite/Utility/TypeConversion.hpp"

//------------------------------------------
namespace Nebulite::Utility {

// String

std::optional<bool> TypeConversion::String::toBool(std::string const& stored){
    if (stored == "true") return true;
    if (stored == "false") return false;
    if(StringHandler::isNumber(stored)){
        try {
            return std::stoi(stored) != 0;
        } catch (...){
            return std::nullopt;
        }
    }
    return stored == "true";
}

std::optional<int> TypeConversion::String::toInt(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return std::stoi(stored);
    } catch (...){
        return std::nullopt;
    }
}

std::optional<std::uint8_t> TypeConversion::String::toUInt8(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return static_cast<std::uint8_t>(std::stoul(stored));
    } catch (...){
        return std::nullopt;
    }
}

std::optional<std::int8_t> TypeConversion::String::toInt8(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return static_cast<std::int8_t>(std::stol(stored));
    } catch (...){
        return std::nullopt;
    }
}

std::optional<std::uint16_t> TypeConversion::String::toUInt16(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return static_cast<std::uint16_t>(std::stoul(stored));
    } catch (...){
        return std::nullopt;
    }
}

std::optional<std::int16_t> TypeConversion::String::toInt16(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return static_cast<std::int16_t>(std::stol(stored));
    } catch (...){
        return std::nullopt;
    }
}

std::optional<std::uint32_t> TypeConversion::String::toUInt32(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return static_cast<std::uint32_t>(std::stoul(stored));
    } catch (...){
        return std::nullopt;
    }
}

std::optional<std::int32_t> TypeConversion::String::toInt32(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return static_cast<std::int32_t>(std::stol(stored));
    } catch (...){
        return std::nullopt;
    }
}

std::optional<std::uint64_t> TypeConversion::String::toUInt64(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return std::stoull(stored);
    } catch (...){
        return std::nullopt;
    }
}

std::optional<std::int64_t> TypeConversion::String::toInt64(std::string const& stored){
    if (stored == "true") return 1;
    if (stored == "false") return 0;
    try {
        return std::stoll(stored);
    } catch (...){
        return std::nullopt;
    }
}

std::optional<double> TypeConversion::String::toDouble(std::string const& stored){
    if (stored == "true") return 1.0;
    if (stored == "false") return 0.0;
    try {
        return std::stod(stored);
    } catch (...){
        return std::nullopt;
    }
}

// Bool

std::optional<std::string> TypeConversion::Bool::toString(bool const value){
    return value ? std::optional<std::string>{"true"} : std::optional<std::string>{"false"};
}

// Double

std::optional<bool> TypeConversion::Double::toBool(double const value) {
    return std::optional{std::fabs(value) > std::numeric_limits<double>::epsilon()};
}

} // namespace Nebulite::Utility