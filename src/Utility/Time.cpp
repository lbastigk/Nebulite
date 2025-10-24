#include "Utility/Time.hpp"

#include <array>

std::string Nebulite::Utility::Time::TimeIso8601(Time::ISO8601FORMATTER format, bool local) {
    const char* fmt = "%FT%TZ";
    std::time_t time = std::time(nullptr);
    std::tm tm_struct;

    if (local) {
        tm_struct = *std::localtime(&time);
    } else {
        tm_struct = *std::gmtime(&time);
    }

    // Use a buffer large enough for any reasonable ISO8601 string
    std::array<char, 32> buffer{};
    size_t written = std::strftime(buffer.data(), buffer.size(), fmt, &tm_struct);

    // Defensive: if strftime fails, return empty string
    if (written == 0) return {};

    // If you want to truncate to 'format' length, do so safely
    int length = static_cast<int>(format);
    if (length > 0 && length < static_cast<int>(written)) {
        return std::string(buffer.data(), length);
    }
    return std::string(buffer.data());
}

uint64_t Nebulite::Utility::Time::gettime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void Nebulite::Utility::Time::wait(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void Nebulite::Utility::Time::waitmicroseconds(uint64_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void Nebulite::Utility::Time::waitnanoseconds(uint64_t ns) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
}
