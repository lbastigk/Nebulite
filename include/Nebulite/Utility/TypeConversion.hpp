#ifndef NEBULITE_UTILITY_TYPECONVERSION_HPP
#define NEBULITE_UTILITY_TYPECONVERSION_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

//------------------------------------------
namespace Nebulite::Utility {
// Converter helper functions for convertVariant
class TypeConversion {
public:
    class String {
    public:
        static std::optional<bool> toBool(std::string const& stored);

        static std::optional<int> toInt(std::string const& stored);

        static std::optional<std::uint8_t> toUInt8(std::string const& stored);

        static std::optional<std::int8_t> toInt8(std::string const& stored);

        static std::optional<std::uint16_t> toUInt16(std::string const& stored);

        static std::optional<std::int16_t> toInt16(std::string const& stored);

        static std::optional<std::uint32_t> toUInt32(std::string const& stored);

        static std::optional<std::int32_t> toInt32(std::string const& stored);

        static std::optional<std::uint64_t> toUInt64(std::string const& stored);

        static std::optional<std::int64_t> toInt64(std::string const& stored);

        static std::optional<double> toDouble(std::string const& stored);

        template<typename newType>
        static constexpr std::optional<newType> to(std::string const& value) {
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
    };

    class Bool {
    public:
        static std::optional<std::string> toString(bool value);

        template<typename newType>
        static std::optional<newType> to(bool const value) {
            if constexpr (std::is_same_v<newType, std::string>) {
                return toString(value);
            }
            else {
                std::unreachable();
            }
        }
    };

    class Double {
    public:
        static std::optional<bool> toBool(double value);

        template<typename newType>
        static std::optional<newType> to(double const value) {
            if constexpr (std::is_same_v<newType, bool>) {
                return toBool(value);
            }
            else {
                std::unreachable();
            }
        }
    };
};
} // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_TYPECONVERSION_HPP
