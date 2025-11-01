/**
 * @file Time.hpp
 * @brief Provides utilities for time-related functions.
 */

#ifndef NEBULITE_UTILITY_TIME_HPP
#define NEBULITE_UTILITY_TIME_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstdint>
#include <string>

// Nebulite
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Time
 * @brief Provides utilities for time-related functions.
 * 
 * This class provides methods to get the current time, format it as a string,
 * and perform various time-related calculations and actions.
 */
class Time {
public:
    struct Conversion{
        static constexpr uint64_t MS_IN_SECOND = 1000U;
        static constexpr uint64_t MS_IN_MINUTE = 60U * MS_IN_SECOND;
        static constexpr uint64_t MS_IN_HOUR   = 60U * MS_IN_MINUTE;
        static constexpr uint64_t MS_IN_DAY    = 24U * MS_IN_HOUR;
    };

    /**
     * @brief Enum for ISO 8601 date formats.
     * 
     * This enum defines the different formats available for ISO 8601 date strings
     * and is to be used with the TimeIso8601 function.
     */
    enum class ISO8601Format : uint8_t {
        YYYY,
        YYYY_MM,
        YYYY_MM_DD,
        YYYY_MM_DD_HH_MM_SS,
        YYYY_MM_DD_HH_MM_SS_TZ,
    };

    struct alignas(SIMD_ALIGNMENT) IsoFmtInfo {
        std::string_view fmt;   // strftime format or a token you use
        std::size_t maxLen;
    };

    /**
     * @brief Returns the current time in ISO 8601 format as std::string.
     * Using strftime with the format %FT%TZ
     * Total length is up to 20 Characters: 2021-03-01T10:44:10Z
     * 
     * @param format The format of the string to return. 
     * 
     * @param local If true, return the local time; otherwise, return UTC time.
     * 
     * @return The current time in ISO 8601 format.
     */
    static std::string TimeIso8601(Time::ISO8601Format format, bool local) noexcept ;

    /**
     * @brief Returns the current time since epoch in milliseconds.
     * @return The current time since epoch in milliseconds.
     */
    static uint64_t gettime() noexcept ;

    /**
     * @brief Waits for the specified amount of time in milliseconds.
     */
    static void wait(uint64_t milliseconds);

    /**
     * @brief Waits for the specified amount of time in microseconds.
     */
    static void waitmicroseconds(uint64_t microseconds);

    /**
     * @brief Waits for the specified amount of time in nanoseconds.
     */
    static void waitnanoseconds(uint64_t nanoseconds);
};
}   // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_TIME_HPP