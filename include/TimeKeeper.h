

#include "Time.h"

class TimeKeeper{
public:

    TimeKeeper(){
        t_ms = Time::gettime() - t_start;
        dt_ms = 0;
        loop_t_ms = t_ms;
        loop_last_t_ms = t_ms;
    }

    void update(uint64_t fixed_dt_ms = 0){
        // Calculate dt from last update call
        // Regardless of whether the timer is running or not
        loop_last_t_ms = loop_t_ms;
        loop_t_ms = Time::gettime() - t_start;
        loop_dt = loop_t_ms - loop_last_t_ms;

        if(fixed_dt_ms > 0){
            // If fixed dt is set, use that
            loop_dt = fixed_dt_ms;
        }

        // If running, add dt to values
        if(running){
            dt_ms = loop_dt;
        }
        else{
            dt_ms = 0;
        }
        t_ms += dt_ms;
    }

    void start(){
        running = true;
    }

    void stop(){
        running = false;
    }

    // Calculates the projected dt if update() was to be called
    uint64_t projected_dt(){
        if(running){
            uint64_t sim_last_t_ms = loop_t_ms;
            uint64_t sim_t_ms = Time::gettime() - t_start;
            uint64_t sim_dt = sim_t_ms - sim_last_t_ms;
            return sim_dt;
        }
        else{
            return 0;
        }
    }

    uint64_t get_t_ms() const {
        return t_ms;
    }

    uint64_t get_dt_ms() const {
        return dt_ms;
    }


private:
    // Proper, accessible values
    uint64_t t_ms;
    uint64_t dt_ms;

    // Needed values for timekeeping
    bool running = false; // On construction, timer is off
    uint64_t loop_last_t_ms;
    uint64_t loop_t_ms;
    uint64_t loop_dt;

    const uint64_t t_start = Time::gettime();
};