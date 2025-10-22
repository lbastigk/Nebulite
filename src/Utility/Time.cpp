#include "Utility/Time.hpp"

std::string Nebulite::Utility::Time::TimeIso8601(Time::ISO8601FORMATTER format, bool local) {
    int length = static_cast<int>(format);

    std::time_t time = std::time(0); // Get current time

    // Construct local time
    char loc[sizeof("2021-03-01T10:44:10Z")];
    std::strftime(loc, sizeof(loc), "%FT%TZ", localtime(&time));

    // Construct UTC time 
    char utc[sizeof("2021-03-01T10:44:10Z")];
    std::strftime(utc, sizeof(utc), "%FT%TZ", gmtime(&time));
    int i;
    std::stringstream returnval;

    if (local) {
        for (i = 0; i < length; i++) {
            returnval << loc[i];
        }
        return returnval.str();
    }
    else {
        for (i = 0; i < length; i++) {
            returnval << utc[i];
        }
        return returnval.str();
    }
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
