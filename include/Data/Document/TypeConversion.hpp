#ifndef DATA_DOCUMENT_TYPECONVERSION_HPP
#define DATA_DOCUMENT_TYPECONVERSION_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <optional>
#include <string>

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
                // NOLINTNEXTLINE
                return toBool(value);
            }
            else if constexpr (std::is_same_v<newType, int>){
                // NOLINTNEXTLINE
                return toInt(value);
            }
            else if constexpr (std::is_same_v<newType, uint8_t>){
                // NOLINTNEXTLINE
                return toUInt8(value);
            }
            else if constexpr (std::is_same_v<newType, int8_t>){
                // NOLINTNEXTLINE
                return toInt8(value);
            }
            else if constexpr (std::is_same_v<newType, uint16_t>){
                // NOLINTNEXTLINE
                return toUInt16(value);
            }
            else if constexpr (std::is_same_v<newType, int16_t>){
                // NOLINTNEXTLINE
                return toInt16(value);
            }
            else if constexpr (std::is_same_v<newType, uint32_t>){
                // NOLINTNEXTLINE
                return toUInt32(value);
            }
            else if constexpr (std::is_same_v<newType, int32_t>){
                // NOLINTNEXTLINE
                return toInt32(value);
            }
            else if constexpr (std::is_same_v<newType, uint64_t>){
                // NOLINTNEXTLINE
                return toUInt64(value);
            }
            else if constexpr (std::is_same_v<newType, int64_t>){
                // NOLINTNEXTLINE
                return toInt64(value);
            }
            else if constexpr (std::is_same_v<newType, double>){
                // NOLINTNEXTLINE
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

        static std::optional<uint8_t> toUInt8(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<uint8_t>(std::stoul(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<int8_t> toInt8(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<int8_t>(std::stol(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<uint16_t> toUInt16(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<uint16_t>(std::stoul(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<int16_t> toInt16(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<int16_t>(std::stol(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<uint32_t> toUInt32(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<uint32_t>(std::stoul(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<int32_t> toInt32(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return static_cast<int32_t>(std::stol(stored));
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<uint64_t> toUInt64(std::string const& stored){
            if (stored == "true") return 1;
            if (stored == "false") return 0;
            try {
                return std::stoull(stored);
            } catch (...){
                return std::nullopt;
            }
        }

        static std::optional<int64_t> toInt64(std::string const& stored){
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
