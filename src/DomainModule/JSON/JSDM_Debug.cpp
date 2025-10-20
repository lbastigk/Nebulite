#include "DomainModule/JSON/JSDM_Debug.hpp"
#include "Utility/JSON.hpp"       // Global Space for Nebulite

namespace Nebulite::DomainModule::JSON {

//------------------------------------------
// Update
Nebulite::Constants::Error Debug::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error Debug::print(int argc,  char* argv[]){
    if(argc > 2){
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    else if(argc == 2){
        auto memberType = domain->getDoc()->memberCheck(argv[1]);
        if(memberType == Nebulite::Utility::JSON::KeyType::null){
            std::cout << "{}" << std::endl;
            return Nebulite::Constants::ErrorTable::NONE();
        }
        if(memberType == Nebulite::Utility::JSON::KeyType::document){
            std::cout << domain->getDoc()->serialize(argv[1]) << std::endl;
            return Nebulite::Constants::ErrorTable::NONE();
        }
        if(memberType == Nebulite::Utility::JSON::KeyType::value){
            std::cout << domain->getDoc()->get<std::string>(argv[1], "") << std::endl;
            return Nebulite::Constants::ErrorTable::NONE();
        }
    }

    std::cout << domain->getDoc()->serialize() << std::endl;
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::print_name = "print";
const std::string Debug::print_desc = R"(Prints the JSON document to the console for debugging purposes.
If key is empty, prints the entire document.

Usage: print [key]
)";

} // namespace Nebulite::DomainModule::JSON