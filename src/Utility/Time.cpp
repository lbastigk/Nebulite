#include "Utility/Time.hpp"

#include <thread>
#include <array>
#include <chrono>

std::string Nebulite::Utility::Time::TimeIso8601(ISO8601Format format, bool const& local) noexcept
{
    // Convert enum to index and get format info
    static constexpr std::array<IsoFmtInfo, 5> isoInfo{{
        {"%Y", 4},
        {"%Y-%m", 7},
        {"%Y-%m-%d", 10},
        {"%Y-%m-%dT%H:%M:%S", 19},
        {"%Y-%m-%dT%H:%M:%SZ", 20},
    }};

    // Get current time
    std::time_t const time = std::time(nullptr);

    // Consider local vs UTC time
    std::tm const tm_struct = local ? *std::localtime(&time) : *std::gmtime(&time);

    // Use a buffer large enough for any reasonable ISO8601 string
    std::array<char, 32> buffer{};
    const auto& [fmt, maxLen] = isoInfo[static_cast<std::size_t>(format)];
    size_t const written = std::strftime(buffer.data(), buffer.size(), fmt.data(), &tm_struct);

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
