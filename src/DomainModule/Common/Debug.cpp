#include "Nebulite.hpp"
#include "../../../include/DomainModule/Common/Debug.hpp"

namespace Nebulite::DomainModule::Common {

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

// NOLINTNEXTLINE
Constants::Error Debug::print(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    if (args.size() == 2) {
        auto const scopedKey = Data::ScopedKey(args[1]);
        auto const memberType = callerScope.memberType(scopedKey);
        if (memberType == Data::KeyType::null) {
            caller.capture.log.println("{}");
            return Constants::ErrorTable::NONE();
        }
        if (memberType == Data::KeyType::object || memberType == Data::KeyType::array) {
            caller.capture.log.println(callerScope.serialize(scopedKey));
            return Constants::ErrorTable::NONE();
        }
        if (memberType == Data::KeyType::value) {
            caller.capture.log.println(callerScope.get<std::string>(scopedKey).value_or(""));
            return Constants::ErrorTable::NONE();
        }
    }
    caller.capture.log.println(callerScope.serialize());
    (void)caller; // Unused parameter
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Common
