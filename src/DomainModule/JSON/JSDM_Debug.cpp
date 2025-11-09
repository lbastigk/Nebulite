#include "DomainModule/JSON/JSDM_Debug.hpp"
#include "Utility/JSON.hpp"       // Global Space for Nebulite

namespace Nebulite::DomainModule::JSON {

//------------------------------------------
// Update
Constants::Error Debug::update(){
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Error Debug::print(int argc,  char** argv){
    if(argc > 2){
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    if(argc == 2){
        auto const memberType = domain->getDoc()->memberCheck(argv[1]);
        if(memberType == Utility::JSON::KeyType::null){
            logln("{}");
            return Constants::ErrorTable::NONE();
        }
        if(memberType == Utility::JSON::KeyType::document){
            logln(domain->getDoc()->serialize(argv[1]));
            return Constants::ErrorTable::NONE();
        }
        if(memberType == Utility::JSON::KeyType::value){
            logln(domain->getDoc()->get<std::string>(argv[1], ""));
            return Constants::ErrorTable::NONE();
        }
    }
    logln(domain->getDoc()->serialize());
    return Constants::ErrorTable::NONE();
}
std::string const Debug::print_name = "print";
std::string const Debug::print_desc = R"(Prints the JSON document to the console for debugging purposes.
If key is empty, prints the entire document.

Usage: print [key]
)";

} // namespace Nebulite::DomainModule::JSON