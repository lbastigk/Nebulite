#ifndef NEBULITE_UTILITY_TYPECONVERSION_HPP
#define NEBULITE_UTILITY_TYPECONVERSION_HPP

//------------------------------------------
// Includes

// Standard library
#include <charconv>
#include <cmath>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

//------------------------------------------
namespace Nebulite::Utility {
// Converter helper functions for convertVariant
class TypeConversion {
public:
    class String {
    public:
        template<typename NewType>
        static constexpr std::optional<NewType> to(std::string_view const value) {
            if constexpr (std::is_same_v<NewType, bool>){
                if (value == "true") return true;
                if (value == "false") return false;
                float result{};
                if (auto const [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result); ec == std::errc{}) {
                    return std::fabs(result) > std::numeric_limits<float>::epsilon();
                }
                return std::nullopt;
            }
            else if constexpr (std::is_same_v<NewType, std::string>) {
                return std::string(value);
            }
            else if constexpr (std::is_arithmetic_v<NewType>) {
                if (value == "true") return static_cast<NewType>(1);
                if (value == "false") return static_cast<NewType>(0);
                NewType result{};
                if (auto const [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result); ec == std::errc{}) {
                    return result;
                }
                return std::nullopt;
            }
            else {
                std::unreachable();
            }
        }
    };

    class Bool {
    public:
        static std::optional<std::string> toString(bool value);

        template<typename NewType>
        static std::optional<NewType> to(bool const value) {
            if constexpr (std::is_same_v<NewType, std::string>) {
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

        template<typename NewType>
        static std::optional<NewType> to(double const value) {
            if constexpr (std::is_same_v<NewType, bool>) {
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
