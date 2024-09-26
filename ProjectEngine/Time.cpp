#pragma once

#include <chrono>
#include <thread>
#include <iostream>
#include <time.h>
#include <ctime>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include <Windows.h>
#include <iomanip>
#include <format>
#include <sys/stat.h>
#include <direct.h>
#include <utility>

#pragma warning( disable : 4996 )

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

class Time {
public:
    //Returns a string with current iso time
    static std::string time_iso8601(int length, int local) {
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
    static uint64_t gettime() {
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    //Returns the difference in ms between starttime and now as string
    static std::string getruntime(uint64_t starttime) {
        uint64_t time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - starttime;

        //time now contains the seconds since starttime has passed

        return std::to_string(time) + "ms";
    }

    static void wait(int ms) {
        Sleep(ms);
    }

    static void waitmicroseconds(uint64_t us) {
        sleep_for(microseconds(us));
    }

    static void waitnanoseconds(uint64_t ns) {
        sleep_for(nanoseconds(ns));
    }
};