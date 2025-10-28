#include "DomainModule/GlobalSpace/GSDM_Time.hpp"
#include "Core/GlobalSpace.hpp"

namespace Nebulite::DomainModule::GlobalSpace {

//------------------------------------------
// Key definitions

std::string const Time::time_name = "time";
std::string const Time::time_desc = R"(Commands for time management)";

std::string const Time::key_runtime_t = "runtime.t";
std::string const Time::key_runtime_t_ms = "runtime.t_ms";
std::string const Time::key_runtime_dt = "runtime.dt";
std::string const Time::key_runtime_dt_ms = "runtime.dt_ms";

std::string const Time::key_time_t = "time.t";
std::string const Time::key_time_t_ms = "time.t_ms";
std::string const Time::key_time_dt = "time.dt";
std::string const Time::key_time_dt_ms = "time.dt_ms";

std::string const Time::key_framecount = "frameCount";

//------------------------------------------
// Update

Nebulite::Constants::Error Time::update(){
    //------------------------------------------
    // Full time (runtime)

    // Update
    RealTime.update();
    uint64_t dt_ms = RealTime.get_dt_ms();
    uint64_t t_ms  = RealTime.get_t_ms();

    // Set in doc
    domain->getDoc()->set<double>(key_runtime_dt,    static_cast<double>(dt_ms) / 1000.0);
    domain->getDoc()->set<double>(key_runtime_t,     static_cast<double>(t_ms)  / 1000.0);
    domain->getDoc()->set<Uint64>(key_runtime_dt_ms, dt_ms);
    domain->getDoc()->set<Uint64>(key_runtime_t_ms,  t_ms);

    // See if simulation time can progress
    bool canProgress = !haltThisFrame && timeLocks.empty() && !domain->getRenderer()->isSkippingUpdate();
    if(canProgress){
        //------------------------------------------
        // Simulation time (can be paused)

        // Update
        if(fixedDeltaTime > 0){
            // Use fixed delta time
            SimulationTime.update(fixedDeltaTime);
        } else{
            // Use real delta time
            SimulationTime.update(dt_ms);
        }
        uint64_t sim_dt_ms = SimulationTime.get_dt_ms();
        uint64_t sim_t_ms = SimulationTime.get_t_ms();

        // Set in doc
        domain->getDoc()->set<double>(key_time_dt,    static_cast<double>(sim_dt_ms) / 1000.0);
        domain->getDoc()->set<double>(key_time_t,     static_cast<double>(sim_t_ms)  / 1000.0);
        domain->getDoc()->set<Uint64>(key_time_dt_ms, sim_dt_ms);
        domain->getDoc()->set<Uint64>(key_time_t_ms,  sim_t_ms);

        //------------------------------------------
        // Increase Frame count
        /**
         * @todo Consider a secondary frame count that only increases when simulation time progresses
         * Perhaps even use that as standard frame count, and have a separate "real frame count" for total frames rendered
         * Best idea would be to check if the renderer is skipping the update, and only increase frame count if not
         */
        domain->getDoc()->set<uint64_t>(key_framecount, frameCount); // Starts at 0
        frameCount++;
    } else{
        //------------------------------------------
        // TODO: Tell renderer to skip logic this frame
        domain->getRenderer()->skipUpdateNextFrame();
    }
    haltThisFrame = false;

    // Ignoring results for now, just return NONE
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Nebulite::Constants::Error Time::time_haltOnce(int argc,  char** argv){
    haltThisFrame = true;
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Time::time_haltOnce_name = "time halt-once";
std::string const Time::time_haltOnce_desc = R"(Halts time for one frame
Meaning you can halt time by continuously calling this function.

Usage: time halt-once
)";

Nebulite::Constants::Error Time::time_lock(int argc,  char** argv){
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string lockName = argv[1];
    timeLocks.insert(lockName);
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Time::time_lock_name = "time lock";
std::string const Time::time_lock_desc = R"(Locks time with lock provided, 
Time can only progress if no locks are present.

Usage: time lock <lock_name>

<lock_name> : Name of the lock to add. Any string without whitespace is valid.
)";


Nebulite::Constants::Error Time::time_unlock(int argc,  char** argv){
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string lockName = argv[1];
    auto it = timeLocks.find(lockName);
    if(it != timeLocks.end()){
        timeLocks.erase(it);
    } else{
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Time::time_unlock_name = "time unlock";
std::string const Time::time_unlock_desc = R"(Removes a time lock.
Time can only progress if no locks are present.

Usage: time unlock <lock_name>

<lock_name> : Name of the lock to remove. Must match an existing lock.
)";

Nebulite::Constants::Error Time::time_masterUnlock(int argc,  char** argv){
    timeLocks.clear();
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Time::time_masterUnlock_name = "time master-unlock";
std::string const Time::time_masterUnlock_desc = R"(Removes all time locks.
Time can only progress if no locks are present.

Usage: time master-unlock
)";

Nebulite::Constants::Error Time::time_setFixedDeltaTime(int argc,  char** argv){
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    try{
        uint64_t dt = std::stoull(argv[1]);
        fixedDeltaTime = dt;
    } catch(...){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
std::string const Time::time_setFixedDeltaTime_name = "time set-fixed-dt";
std::string const Time::time_setFixedDeltaTime_desc = R"(Sets a fixed delta time in milliseconds for the simulation time.
Use 0 to disable fixed dt.

Usage: time set-fixed-dt <dt_ms>
)";

} // namespace Nebulite::DomainModule::GlobalSpace