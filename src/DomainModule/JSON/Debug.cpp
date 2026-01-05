#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
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
        auto const scopedKey = Data::ScopedKey(argv[1]);
        auto const memberType = getDoc().memberType(scopedKey);
        if (memberType == Data::KeyType::null) {
            Nebulite::cout() << "{}" << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
        if (memberType == Data::KeyType::object || memberType == Data::KeyType::array) {
            Nebulite::cout() << getDoc().serialize(scopedKey) << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
        if (memberType == Data::KeyType::value) {
            Nebulite::cout() << getDoc().get<std::string>(scopedKey, "") << Nebulite::endl;
            return Constants::ErrorTable::NONE();
        }
    }
    Nebulite::cout() << getDoc().serialize() << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::JsonScope
