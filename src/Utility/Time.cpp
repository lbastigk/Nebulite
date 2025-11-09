#include "Utility/Time.hpp"

#include <thread>
#include <array>
#include <chrono>

std::string Nebulite::Utility::Time::TimeIso8601(ISO8601Format const& format, bool const& local) noexcept
{
    // Get current time
    std::time_t const time = std::time(nullptr);

    // Consider local vs UTC time
    std::tm const tm_struct = local ? *std::localtime(&time) : *std::gmtime(&time);

    // Write into buffer
    // Using a switch so that the compiler sees a format literal at compile time,
    // meaning its format type is checked properly.
    std::array<char, 32> buffer{};
    std::size_t constexpr maxLen = 32; // maximum length of the buffer
    std::size_t written = 0;
    switch (format) {
        case ISO8601Format::YYYY:
            written = std::strftime(buffer.data(), sizeof(buffer), "%Y", &tm_struct);
            break;
        case ISO8601Format::YYYY_MM:
            written = std::strftime(buffer.data(), sizeof(buffer), "%Y-%m", &tm_struct);
            break;
        case ISO8601Format::YYYY_MM_DD:
            written = std::strftime(buffer.data(), sizeof(buffer), "%Y-%m-%d", &tm_struct);
            break;
        case ISO8601Format::YYYY_MM_DD_HH_MM_SS:
            written = std::strftime(buffer.data(), sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tm_struct);
            break;
        case ISO8601Format::YYYY_MM_DD_HH_MM_SS_TZ:
            written = std::strftime(buffer.data(), sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm_struct);
            break;
        default:
            return {};
    }

    // Defensive: if strftime fails, return empty string
    if (written == 0) return {};

    // Use the stored maximum length for this format (from isoInfo)
    // and return at most 'written' characters. This avoids using the
    // enum numeric value (which is the choice index, not a length).
    std::size_t const retLen = written > maxLen ? maxLen : written;
    return {buffer.data(), retLen};
}

uint64_t Nebulite::Utility::Time::getTime() noexcept {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
}

void Nebulite::Utility::Time::wait(uint64_t const& milliseconds){
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Nebulite::Utility::Time::waitMicroseconds(uint64_t const& microseconds){
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void Nebulite::Utility::Time::waitNanoseconds(uint64_t const& nanoseconds){
    std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
}
