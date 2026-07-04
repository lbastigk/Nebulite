#ifndef DATA_DOCUMENT_TYPECONVERSION_HPP
#define DATA_DOCUMENT_TYPECONVERSION_HPP

//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>

// Nebulite
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Data {
// Converter helper functions for convertVariant
class TypeConversion {
public:
    class String {
    public:
        template<typename newType>
        static constexpr std::optional<newType> toAny(std::string const& value) {
            if constexpr (std::is_same_v<newType, bool>){
                return toBool(value);
            }
            else if constexpr (std::is_same_v<newType, int>){
                return toInt(value);
            }
            else if constexpr (std::is_same_v<newType, std::uint8_t>){
                return toUInt8(value);
            }
            else if constexpr (std::is_same_v<newType, std::int8_t>){
                return toInt8(value);
            }
            else if constexpr (std::is_same_v<newType, std::uint16_t>){
                return toUInt16(value);
            }
            else if constexpr (std::is_same_v<newType, std::int16_t>){
                return toInt16(value);
            }
            else if constexpr (std::is_same_v<newType, std::uint32_t>){
                return toUInt32(value);
            }
            else if constexpr (std::is_same_v<newType, std::int32_t>){
                return toInt32(value);
            }
            else if constexpr (std::is_same_v<newType, std::uint64_t>){
                return toUInt64(value);
            }
            else if constexpr (std::is_same_v<newType, std::int64_t>){
                return toInt64(value);
            }
            else if constexpr (std::is_same_v<newType, double>){
                return toDouble(value);
            }
            else if constexpr (std::is_same_v<newType, std::string>) {
                return value;
            }
            else {
                std::unreachable();
            }
        }

        static std::optional<bool> toBool(std::string const& stored){
            if (stored == "true") return true;
            if (stored == "false") return false;
            if(Utility::StringHandler::isNumber(stored)){
                try {
                    return std::stoi(stored) != 0;
                } catch (...){
                    return std::nullopt;
                }
            }
            return stored == "true";
        }

        static std::optional<int> toInt(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return std::stoi(stored);
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<std::uint8_t> toUInt8(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<std::uint8_t>(std::stoul(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<std::int8_t> toInt8(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<std::int8_t>(std::stol(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<std::uint16_t> toUInt16(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<std::uint16_t>(std::stoul(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<std::int16_t> toInt16(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<std::int16_t>(std::stol(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<std::uint32_t> toUInt32(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<std::uint32_t>(std::stoul(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<std::int32_t> toInt32(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<std::int32_t>(std::stol(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<std::uint64_t> toUInt64(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return std::stoull(stored);
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<std::int64_t> toInt64(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return std::stoll(stored);
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<double> toDouble(std::string const& stored){
            if (stored == "true") return 1.0;
            if (stored == "false") return 0.0;
            try {
                return std::stod(stored);
            } catch (...){
                return std::nullopt;
            }
        }
    };

    class Bool {
    public:
        static std::optional<std::string> toString(bool const value){
            return value ? std::optional<std::string>{"true"} : std::optional<std::string>{"false"};
        }
    };

    class Double {
    public:
        static std::optional<bool> toBool(double const value) {
            return std::optional{std::fabs(value) > std::numeric_limits<double>::epsilon()};
        }
    };
};
} // namespace Nebulite::Data
#endif // DATA_DOCUMENT_TYPECONVERSION_HPP
