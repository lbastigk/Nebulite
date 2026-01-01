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

//------------------------------------------
namespace Nebulite::Utility {
/**
 * @class Time
 * @brief Provides utilities for time-related functions.
 * @details Provides static methods to get the current time, format it as a string,
 *          and perform various time-related calculations and actions.
 */
class Time {
public:
    /**
     * @struct Conversion
     * @brief Contains constants for time unit conversions.
     */
    struct Conversion{
        static constexpr uint64_t millisecondsPerSecond = 1000U;
        static constexpr uint64_t millisecondsPerMinute = 60U * millisecondsPerSecond;
        static constexpr uint64_t millisecondsPerHour   = 60U * millisecondsPerMinute;
        static constexpr uint64_t millisecondsPerDay    = 24U * millisecondsPerHour;
    };

    /**
     * @brief ISO 8601 date formats.
     * @details Defines the different formats available for ISO 8601 date strings
     *          and is to be used with the TimeIso8601 function.
     */
    enum class ISO8601Format : uint8_t {
        YYYY,
        YYYY_MM,
        YYYY_MM_DD,
        YYYY_MM_DD_HH_MM_SS,
        YYYY_MM_DD_HH_MM_SS_TZ,
    };

    /**
     * @struct IsoFmtInfo
     * @brief Contains information about ISO 8601 format strings.
     */
    struct IsoFmtInfo {
        std::string_view fmt;   // strftime format or a token you use
        std::size_t maxLen;
    };

    /**
     * @brief Returns the current time in ISO 8601 format as std::string.
     * @details Using strftime with the format %FT%TZ
     *          Total length is up to 20 Characters: 2021-03-01T10:44:10Z
     * @param format The format of the string to return.
     * @param local If true, return the local time; otherwise, return UTC time.
     * @return The current time in ISO 8601 format.
     */
    static std::string TimeIso8601(ISO8601Format const& format, bool const& local) noexcept ;

    /**
     * @brief Returns the current time since epoch in milliseconds.
     * @return The current time since epoch in milliseconds.
     */
    static uint64_t getTime() noexcept ;

    /**
     * @brief Waits for the specified amount of time in milliseconds.
     * @param milliseconds The amount of time to wait in milliseconds.
     */
    static void wait(uint64_t const& milliseconds);

    /**
     * @brief Waits for the specified amount of time in microseconds.
     * @param microseconds The amount of time to wait in microseconds.
     */
    static void waitMicroseconds(uint64_t const& microseconds);

    /**
     * @brief Waits for the specified amount of time in nanoseconds.
     * @param nanoseconds The amount of time to wait in nanoseconds.
     */
    static void waitNanoseconds(uint64_t const& nanoseconds);
};
}   // namespace Nebulite::Utility
#endif // NEBULITE_UTILITY_TIME_HPP
