#include "Nebulite.hpp"
#include "DomainModule/Common/Debug.hpp"

namespace Nebulite::DomainModule::Common {

//------------------------------------------
// Update
Constants::Event Debug::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Event Debug::print(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(caller.capture);
    }
    if (args.size() == 2) {
        auto const scopedKey = Data::ScopedKey(args[1]);
        auto const memberType = callerScope.memberType(scopedKey);
        if (memberType == Data::KeyType::null) {
            caller.capture.log.println("{}");
            return Constants::Event::Success;
        }
        if (memberType == Data::KeyType::object || memberType == Data::KeyType::array) {
            caller.capture.log.println(callerScope.serialize(scopedKey));
            return Constants::Event::Success;
        }
        if (memberType == Data::KeyType::value) {
            caller.capture.log.println(callerScope.get<std::string>(scopedKey).value_or(""));
            return Constants::Event::Success;
        }
    }
    caller.capture.log.println(callerScope.serialize());
    (void)caller; // Unused parameter
    return Constants::Event::Success;
}

Constants::Event Debug::printId(std::span<std::string const> const& /*args*/, Interaction::Execution::Domain& caller, Data::JsonScope& /*callerScope*/) {
    caller.capture.log.println(caller.getId());
    return Constants::Event::Success;
}

Constants::Event Debug::error(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& /*callerScope*/) {
    auto const& argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    caller.capture.error.println(argStr);
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event Debug::warn(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    domain.capture.warning.println(argStr);
    return Constants::Event::Warning;
}

Constants::Event Debug::critical(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    domain.capture.error.println(argStr);
    return Constants::Event::Error;
}

} // namespace Nebulite::DomainModule::Common
