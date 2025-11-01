#include "Utility/Time.hpp"

#include <thread>
#include <array>
#include <chrono>

std::string Nebulite::Utility::Time::TimeIso8601(Time::ISO8601Format format, bool local) noexcept {
    // Convert enum to index and get format info
    static constexpr std::array<IsoFmtInfo, 5> isoInfo{{
        {"%Y", 4},
        {"%Y-%m", 7},
        {"%Y-%m-%d", 10},
        {"%Y-%m-%dT%H:%M:%S", 19},
        {"%Y-%m-%dT%H:%M:%SZ", 20},
    }};

    // Get current time
    std::time_t time = std::time(nullptr);
    std::tm tm_struct;

    // Consider local vs UTC time
    if (local){
        tm_struct = *std::localtime(&time);
    } else {
        tm_struct = *std::gmtime(&time);
    }

    // Use a buffer large enough for any reasonable ISO8601 string
    std::array<char, 32> buffer{};
    const auto &info = isoInfo[static_cast<std::size_t>(format)];
    size_t written = std::strftime(buffer.data(), buffer.size(), info.fmt.data(), &tm_struct);

    // Defensive: if strftime fails, return empty string
    if (written == 0) return {};

    // Use the stored maximum length for this format (from isoInfo)
    // and return at most 'written' characters. This avoids using the
    // enum numeric value (which is the choice index, not a length).
    const std::size_t maxLen = info.maxLen; // IsoFmtInfo second member
    const std::size_t retLen = (written > maxLen) ? maxLen : written;
    return std::string(buffer.data(), retLen);
}

uint64_t Nebulite::Utility::Time::gettime() noexcept {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
}

void Nebulite::Utility::Time::wait(uint64_t milliseconds){
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Nebulite::Utility::Time::waitmicroseconds(uint64_t microseconds){
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void Nebulite::Utility::Time::waitnanoseconds(uint64_t nanoseconds){
    std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
}
