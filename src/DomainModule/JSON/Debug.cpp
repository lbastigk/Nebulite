#include "Nebulite.hpp"
#include "Data/Document/JsonScope.hpp"
#include "DomainModule/JsonScope/Debug.hpp"

namespace Nebulite::DomainModule::JsonScope {

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
        auto const memberType = getDoc().memberType(argv[1]);
        if (memberType == Data::KeyType::null) {
            Nebulite::cout() << "{}" << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
        if (memberType == Data::KeyType::object) {
            Nebulite::cout() << getDoc().serialize(argv[1]) << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
        if (memberType == Data::KeyType::value) {
            Nebulite::cout() << getDoc().get<std::string>(argv[1], "") << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
    }
    Nebulite::cout() << getDoc().serialize() << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::JsonScope
