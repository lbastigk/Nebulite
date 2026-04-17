#include "Module/Domain/RenderObject/Logging.hpp"
#include "Core/RenderObject.hpp"
#include "Utility/FileManagement.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Update
Constants::Event Logging::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Event Logging::log_all(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    std::string const serialized = callerScope.serialize();
    if (args.size() > 1) {
        for (auto const& arg : args.subspan(1)) {
            if (!Utility::FileManagement::WriteFile(arg, serialized)) {
                return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
            }
        }
    } else {
        if (!Utility::FileManagement::WriteFile("RenderObject_id" + std::to_string(caller.getId()) + ".log.jsonc", serialized)) {
            return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
        }
    }
    (void)caller;      // Unused
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event Logging::log_key(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(caller.capture);
    }
    auto const key = callerScope.getRootScope() + args[1];
    std::string file = "RenderObject_id" + std::to_string(caller.getId()) + ".log.jsonc";
    if (args.size() > 2) {
        file = args[2];
    }
    auto const value = callerScope.get<std::string>(key.view()).value_or("Key not found");
    if (!Utility::FileManagement::WriteFile(file, value)) {
        return Constants::StandardCapture::Error::File::couldNotWriteFile(domain.capture);
    }
    (void)caller;      // Unused
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::RenderObject
