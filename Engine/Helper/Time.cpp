#include "Time.h"

//Returns a string with current iso time
std::string Time::time_iso8601(int length, int local) {
    std::time_t time = std::time(0); // Get current time

    // Construct local time
    char loc[sizeof("2021-03-01T10:44:10Z")];
    strftime(loc, sizeof(loc), "%FT%TZ", localtime(&time));

    // Construct UTC time 
    char utc[sizeof("2021-03-01T10:44:10Z")];
    strftime(utc, sizeof(utc), "%FT%TZ", gmtime(&time));
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


//Gives the current time since epoch in ms as integer
uint64_t Time::gettime() {
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

//Returns the difference in ms between starttime and now as string
std::string Time::getruntime(uint64_t starttime) {
    uint64_t time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - starttime;

    //time now contains the seconds since starttime has passed

    return std::to_string(time) + "ms";
}

void Time::wait(int ms) {
    sleep_for(milliseconds(ms));
}

void Time::waitmicroseconds(uint64_t us) {
    sleep_for(microseconds(us));
}

void Time::waitnanoseconds(uint64_t ns) {
    sleep_for(nanoseconds(ns));
}
