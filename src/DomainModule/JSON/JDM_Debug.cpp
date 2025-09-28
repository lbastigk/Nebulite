#include "DomainModule/JSON/JDM_Debug.hpp"
#include "Utility/JSON.hpp"       // Global Space for Nebulite

//------------------------------------------
// Update
void Nebulite::DomainModule::JSON::Debug::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error Nebulite::DomainModule::JSON::Debug::print(int argc, char* argv[]){
    if(argc > 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    else if(argc == 2){
        Nebulite::Utility::JSON subdoc = domain->getDoc()->get_subdoc(argv[1]);
        std::cout << subdoc.serialize() << std::endl;
        return Nebulite::Constants::ErrorTable::NONE();
    }

    std::cout << domain->getDoc()->serialize() << std::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}