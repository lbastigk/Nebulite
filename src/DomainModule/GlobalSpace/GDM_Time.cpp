#include "DomainModule/GlobalSpace/GDM_Time.hpp"

#include "Core/GlobalSpace.hpp"


void Nebulite::DomainModule::GlobalSpace::Time::update() {
    //------------------------------------------
    // Full time (runtime)

    // Update
    RealTime.update();
    uint64_t dt_ms = RealTime.get_dt_ms();
    uint64_t t_ms  = RealTime.get_t_ms();

    // Set in doc
    domain->getDoc()->set<double>( "runtime.dt", dt_ms / 1000.0);
    domain->getDoc()->set<double>( "runtime.t",   t_ms / 1000.0);
    domain->getDoc()->set<Uint64>( "runtime.dt_ms", dt_ms);
    domain->getDoc()->set<Uint64>( "runtime.t_ms", t_ms);

    // See if simulation time can progress
    bool canProgress = !haltThisFrame && timeLocks.empty();
    if(domain->RendererExists()){
        canProgress = canProgress && !domain->getRenderer()->isSkippingUpdate();
    }
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
        uint64_t dt_ms = SimulationTime.get_dt_ms();
        uint64_t t_ms = SimulationTime.get_t_ms();

        // Set in doc
        domain->getDoc()->set<double>( "time.dt", dt_ms / 1000.0);
        domain->getDoc()->set<double>( "time.t",   t_ms / 1000.0);
        domain->getDoc()->set<Uint64>( "time.dt_ms", dt_ms);
        domain->getDoc()->set<Uint64>( "time.t_ms", t_ms);

        //------------------------------------------
        // Increase Frame count
        domain->getDoc()->set<uint64_t>("frameCount",frameCount); // Starts at 0
        frameCount++;
    } else{
        //------------------------------------------
        // TODO: Tell renderer to skip logic this frame
        domain->getRenderer()->skipUpdateNextFrame();
    }
    haltThisFrame = false;
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Time::haltOnce(int argc, char** argv){
    haltThisFrame = true;
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Time::lock(int argc, char** argv){
    if(argc < 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    std::string lockName = argv[1];
    timeLocks.insert(lockName);
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Time::unlock(int argc, char** argv){
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

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Time::masterUnlock(int argc, char** argv){
    timeLocks.clear();
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::GlobalSpace::Time::setFixedDeltaTime(int argc, char** argv){
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