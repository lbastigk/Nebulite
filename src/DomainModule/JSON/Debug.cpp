#include "DomainModule/JSON/Debug.hpp"
#include "Data/Document/JSON.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::JSON {

//------------------------------------------
// Update
Constants::Error Debug::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Constants::Error Debug::print(int argc, char** argv) {
    if (argc > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    if (argc == 2) {
        auto const memberType = domain.getDoc()->memberType(argv[1]);
        if (memberType == Data::JSON::KeyType::null) {
            Nebulite::cout() << "{}" << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
        if (memberType == Data::JSON::KeyType::object) {
            Nebulite::cout() << domain.getDoc()->serialize(argv[1]) << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
        if (memberType == Data::JSON::KeyType::value) {
            Nebulite::cout() << domain.getDoc()->get<std::string>(argv[1], "") << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
    }
    Nebulite::cout() << domain.getDoc()->serialize() << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::JSON
