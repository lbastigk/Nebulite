/**
 * @file Time.h
 * @brief Provides utilities for time-related functions.
 */

#pragma once

//-----------------------------------------------------------
// Includes

// General
#include <thread>
#include <iostream>
#include <sstream>

//-----------------------------------------------------------
namespace Nebulite {
namespace Utility {
/**
 * @class Time
 * @brief Provides utilities for time-related functions.
 * 
 * This class provides methods to get the current time, format it as a string,
 * and perform various time-related calculations and actions.
 */
class Time {
public:
    /**
     * @brief Enum for ISO 8601 date formats.
     * 
     * This enum defines the different formats available for ISO 8601 date strings
     * and is to be used with the time_iso8601 function.
     */
    enum ISO8601_FORMAT {
        YYYY = 4,
        YYYY_MM = 7,
        YYYY_MM_DD = 10,
        YYYY_MM_DD_HH_MM_SS = 18,
        YYYY_MM_DD_HH_MM_SS_TZ = 19, // Full ISO 8601 format with Timezone
    };

    /**
     * @brief Returns the current time in ISO 8601 format as std::string.
     * Using strftime with the format %FT%TZ
     * Total length is up to 19 Characters: 2021-03-01T10:44:10Z
     * 
     * @param format The format of the string to return. 
     * 
     * @param local If true, return the local time; otherwise, return UTC time.
     * 
     * @return The current time in ISO 8601 format.
     */
    static std::string time_iso8601(Time::ISO8601_FORMAT format, int local);

    /**
     * @brief Returns the current time since epoch in milliseconds.
     * @return The current time since epoch in milliseconds.
     */
    static uint64_t gettime();

    /**
     * @brief Waits for the specified amount of time in milliseconds.
     */
    static void wait(int ms);

    /**
     * @brief Waits for the specified amount of time in microseconds.
     */
    static void waitmicroseconds(uint64_t us);

    /**
     * @brief Waits for the specified amount of time in nanoseconds.
     */
    static void waitnanoseconds(uint64_t ns);
};
}   // namespace Utility
}   // namespace Nebulite
