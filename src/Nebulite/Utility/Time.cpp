//------------------------------------------
// Includes

// Standard library
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <string>
#include <thread>
#include <utility>

// Nebulite
#include "Nebulite/Utility/Time.hpp"

//------------------------------------------
namespace Nebulite::Utility {

std::string Time::timeIso8601(Iso8601Format const format, bool const local) {
    // Get current time
    std::time_t const time = std::time(nullptr);

    // Consider local vs UTC time
    std::tm const timeInfo = local ? *std::localtime(&time) : *std::gmtime(&time); // NOLINT(concurrency-mt-unsafe)

    // Write into buffer
    // Using a switch so that the compiler sees a format literal at compile time,
    // meaning its format type is checked properly.
    auto constexpr maxLen = 32; // maximum length of the buffer
    std::array<char, maxLen> buffer{};
    std::size_t const written = [&] {
        switch (format) {
        case Iso8601Format::YYYY:
            return std::strftime(buffer.data(), sizeof(buffer), "%Y", &timeInfo);
        case Iso8601Format::YYYY_MM:
            return std::strftime(buffer.data(), sizeof(buffer), "%Y-%m", &timeInfo);
        case Iso8601Format::YYYY_MM_DD:
            return std::strftime(buffer.data(), sizeof(buffer), "%Y-%m-%d", &timeInfo);
        case Iso8601Format::YYYY_MM_DD_HH_MM_SS:
            return std::strftime(buffer.data(), sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeInfo);
        case Iso8601Format::YYYY_MM_DD_HH_MM_SS_TZ:
            return std::strftime(buffer.data(), sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeInfo);
        default:
            std::unreachable();
        }
    }();

    // Defensive: if strftime fails, return empty string
    if (written == 0)
        return {};

    // Use the stored maximum length for this format (from isoInfo)
    // and return at most 'written' characters. This avoids using the
    // enum numeric value (which is the choice index, not a length).
    std::size_t const retLen = written > maxLen ? maxLen : written;
    return {buffer.data(), retLen};
}

uint64_t Time::getTime() noexcept {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
}

void Time::wait(std::uint64_t const milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Time::waitMicroseconds(std::uint64_t const microseconds) {
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void Time::waitNanoseconds(std::uint64_t const nanoseconds) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
}

} // namespace Nebulite::Utility
