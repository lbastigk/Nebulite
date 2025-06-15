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
#include <iomanip>
#include <format>
#include <sys/stat.h>
#include <utility>

#pragma warning( disable : 4996 )

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

class Time {
public:
    //Returns a string with current iso time
    static std::string time_iso8601(int length, int local);

    //Gives the current time since epoch in ms as integer
    static uint64_t gettime();

    //Returns the difference in ms between starttime and now as string
    static std::string getruntime(uint64_t starttime);

    static void wait(int ms);

    static void waitmicroseconds(uint64_t us);

    static void waitnanoseconds(uint64_t ns);
};

class TimeKeeper{
public:
    TimeKeeper(){
        t_ms = Time::gettime();
        dt_ms = 0;
        last_t_ms = t_ms;
    }

    void update(){
        last_t_ms = t_ms;
        t_ms = Time::gettime();
        dt_ms = t_ms - last_t_ms;
    }

    uint64_t t_ms;
    uint64_t dt_ms;

private:
    uint64_t last_t_ms;
};